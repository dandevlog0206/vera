#include "../../include/vera/typography/font_atlas.h"

#include "../../include/vera/core/exception.h"
#include "../../include/vera/core/device.h"
#include "../../include/vera/core/pipeline.h"
#include "../../include/vera/core/pipeline_layout.h"
#include "../../include/vera/core/command_buffer.h"
#include "../../include/vera/core/descriptor_pool.h"
#include "../../include/vera/core/descriptor_set_layout.h"
#include "../../include/vera/core/descriptor_set.h"
#include "../../include/vera/core/buffer.h"
#include "../../include/vera/math/vector_math.h"
#include "../../include/vera/util/rect_packer.h"
#include "../../include/vera/util/static_vector.h"
#include "font_impl_base.h"

#define FLOAT_INF         0x7f800000
#define FLAG_NONE         0x0u
#define FLAG_ON_CURVE     0x1u
#define FLAG_END_CONTOUR  0x2u
#define FLAG_END_GLYPH    0x4u
#define MAX_CONTOUR_COUNT 128
#define MAX_EDGE_COUNT    1024
#define MAX_POINT_COUNT   1024
#define END_CONTOUR       { FLOAT_INF, 0.0 }
#define END_GLYPH         { FLOAT_INF, FLOAT_INF }

VERA_NAMESPACE_BEGIN
VERA_PRIV_NAMESPACE_BEGIN

class FontAtlasGlobalResource : public ManagedObject
{
public:
	obj<Device>   device;
	obj<Pipeline> hardMaskPipeline;
	obj<Pipeline> softMaskPipeline;
	obj<Pipeline> sdfPipeline;
	obj<Pipeline> psdfPipeline;
	obj<Pipeline> msdfPipeline;
	obj<Pipeline> mtsdfPipeline;
};

class FontAtlasResource
{
public:
	obj<FontAtlasGlobalResource> globalResource;
	obj<Device>                  device;
	obj<DescriptorPool>          descriptorPool;
	obj<DescriptorSet>           descriptorSet;
	obj<Pipeline>                pipeline;
	obj<CommandBuffer>           commandBuffer;
	obj<Buffer>                  vertexBuffer;
	obj<Buffer>                  storageBuffer;
	ShaderStageFlags             pcStageFlags;
	CommandBufferSync            commandBufferSync;
};

struct GlyphPage
{
	std::vector<obj<Texture>>                textures;
	std::unordered_map<GlyphID, PackedGlyph> glyphMap;
	std::unique_ptr<RectPacker>              packer;
	uint32_t                                 textureCount;
	uint32_t                                 fullyPackedCount; // number of fully packed textures
	uint32_t                                 px;
};

VERA_PRIV_NAMESPACE_END

enum EdgeColor : uint8_t
{
	EDGE_COLOR_None    = 0x0,
	EDGE_COLOR_Blue    = 0x1,
	EDGE_COLOR_Green   = 0x2,
	EDGE_COLOR_YELLOW  = 0x3,
	EDGE_COLOR_Red     = 0x4,
	EDGE_COLOR_Magenta = 0x5,
	EDGE_COLOR_CYAN    = 0x6,
	EDGE_COLOR_WHITE   = 0x7
};

static weak_obj<priv::FontAtlasGlobalResource> g_global_resource;

typedef float2 SDFGlyphPoint;

struct BitmapVertex
{
	float2 pos;
	float2 uv;

	VERA_VERTEX_DESCRIPTOR_BEGIN(BitmapVertex)
		VERA_VERTEX_ATTRIBUTE(0, pos),
		VERA_VERTEX_ATTRIBUTE(1, uv),
	VERA_VERTEX_DESCRIPTOR_END
};

struct SDFVertex
{
	uint2    position;
	float2   fontCoordMin;
	float2   fontCoordMax;
	uint32_t storageOffset;
	uint32_t layerIndex;
};

struct MSDFGlyphPoint
{
	float2    position;
	bool      onCurve;
	EdgeColor color;
};

static int32_t seed_extract2(hash_t& seed)
{
	int32_t v = static_cast<int32_t>(seed) & 1;
	seed >>= 1;
	return v;
}

static int32_t seed_extract3(hash_t& seed)
{
	int32_t v = static_cast<int32_t>(seed % 3);
	seed /= 3;
	return v;
}

static EdgeColor init_edge_color(hash_t& seed) {
	static const EdgeColor colors[3] = {
		EDGE_COLOR_CYAN,
		EDGE_COLOR_Magenta,
		EDGE_COLOR_YELLOW
	};

	return colors[seed_extract3(seed)];
}

static void switch_edge_color(EdgeColor& color, hash_t& seed)
{
	int32_t shifted = color << (1 + seed_extract2(seed));
	color = EdgeColor((shifted | shifted >> 3) & EDGE_COLOR_WHITE);
}

static void switch_edge_color(EdgeColor& color, hash_t& seed, EdgeColor banned)
{
	EdgeColor combined = EdgeColor(color & banned);

	if (combined == EDGE_COLOR_Red || combined == EDGE_COLOR_Green || combined == EDGE_COLOR_Blue)
		color = EdgeColor(combined ^ EDGE_COLOR_WHITE);
	else
		switch_edge_color(color, seed);
}

static int32_t symmetrical_trichotomy(uint32_t position, uint32_t n)
{
	return static_cast<int32_t>(3.0 + 2.875 * position / (n - 1) - 1.4375 + 0.5) - 3;
}

static void min_distance(std::pair<float, float>& min_dist, const std::pair<float, float>& dist)
{
	float dist_a = std::abs(min_dist.first);
	float dist_b = std::abs(dist.first);

	if (dist_a > dist_b || (dist_a == dist_b && dist.second < min_dist.second))
		min_dist = dist;
}

static std::pair<float, float> line_sdf(const float2& p0, const float2& p1, const float2& p)
{
	float2 aq = p - p0;
	float2 ab = p1 - p0;
	float  t  = dot(aq, ab) / dot(ab, ab);

	float2 eq = 0.5 < t ? p1 - p : p0 - p;
	float  ed = length(eq);

	if (0.0 < t && t < 1.0) {
		float l  = length(ab);
		float od = dot(float2(ab.y / l, -ab.x / l), aq);

		if (std::abs(od) < ed)
			return { od, 0.0 };
	}

	if (cross(aq, ab) < 0.0)
		ed = -ed;

	return { ed, std::abs(dot(normalize(ab), normalize(eq))) };
}

static float glyph_sdf(const Glyph& glyph, const float2& p)
{
	uint32_t state;
	float2   ps;
	float2   p0;
	float2   p1;
	float2   p2;
	float2   p3;

	std::pair<float, float> min_dist = { FLT_MAX, 0.0 };

	for (const auto& contour : glyph.contours) {
		state = 0;

		for (const auto& point : contour) {
			switch (state) {
			case 0:
				ps    = point.position;
				p0    = ps;
				state = 1;
				break;
			case 1:
				if (point.onCurve) {
					p1 = point.position;
					min_distance(min_dist, line_sdf(p0, p1, p));
					p0 = p1;
				} else {
					state = 2;
				}
				break;
			case 2:
				if (point.onCurve) {
					p2    = point.position;
					min_distance(min_dist, line_sdf(p0, p2, p));
					p0    = p2;
					state = 1;
				} else {
					p2 = point.position;
					p3 = (p0 + p2) * 0.5f;
					min_distance(min_dist, line_sdf(p0, p3, p));
					p0 = p3;
				}
				break;
			}
		}

		min_distance(min_dist, line_sdf(p0, ps, p));
	}

	return min_dist.first;
}

static float2 encode_sdf_glyph_point(const GlyphPoint& gp)
{
	uint32_t mask_x = (std::bit_cast<uint32_t>(gp.position.x) & 0xfffffffe);

	mask_x |= static_cast<uint32_t>(gp.onCurve);

	return {
		std::bit_cast<float>(mask_x),
		gp.position.y
	};
}

static float2 encode_msdf_glyph_point(const MSDFGlyphPoint& gp)
{
	uint32_t mask_x = (std::bit_cast<uint32_t>(gp.position.x) & 0xfffffffc);
	uint32_t mask_y = (std::bit_cast<uint32_t>(gp.position.y) & 0xfffffffc);

	mask_x |=
		(gp.onCurve ? 0x1u : 0x0u) |
		(gp.color & EDGE_COLOR_Red ? 0x2u : 0x0u);
	mask_y |=
		(gp.color & EDGE_COLOR_Green ? 0x1u : 0x0u) |
		(gp.color & EDGE_COLOR_Blue ? 0x2u : 0x0u);

	return {
		std::bit_cast<float>(mask_x),
		std::bit_cast<float>(mask_y)
	};
}

static uint32_t get_font_size(const FontAtlasCreateInfo& info, uint32_t px)
{
	switch (info.type) {
	case AtlasType::HardMask:
	case AtlasType::SoftMask:
		return px;
	case AtlasType::SDF:
	case AtlasType::PSDF:
	case AtlasType::MSDF:
	case AtlasType::MTSDF:
		return info.sdfFontSize == 0 ? 48 : info.sdfFontSize;
	}
	
	VERA_ASSERT_MSG(false, "invalid atlas type");
	return 0;
}

static float get_font_scale(const priv::FontImplBase& impl, uint32_t px)
{
	return static_cast<float>(px) / impl.unitsPerEM;
}

static Format get_font_atlas_format(AtlasType type)
{
	switch (type) {
	case AtlasType::HardMask:
		return Format::R8Unorm;
	case AtlasType::SoftMask:
		return Format::R8Unorm;
	case AtlasType::SDF:
		return Format::R8Unorm;
	case AtlasType::PSDF:
		return Format::R8Unorm;
	case AtlasType::MSDF:
		return Format::RGBA32Float;
	case AtlasType::MTSDF:
		return Format::RGBA32Float;
	}
	
	VERA_ASSERT_MSG(false, "invalid atlas type");
	return {};
}

static std::unique_ptr<priv::FontAtlasResource> create_font_atlas_resource(
	obj<Device>                device,
	const FontAtlasCreateInfo& info
) {
	auto resource = std::make_unique<priv::FontAtlasResource>();

	if (!g_global_resource) {
		resource->globalResource = make_obj<priv::FontAtlasGlobalResource>();

		g_global_resource = resource->globalResource;
		g_global_resource->device = device;
	} else {
		if (g_global_resource->device != device)
			throw Exception("Font Atlas global resource device mismatch");
		
		resource->globalResource = g_global_resource;
	}

	switch (info.type) {
	case AtlasType::HardMask: {
	} break;
	case AtlasType::SoftMask: {
	} break;
	case AtlasType::SDF: {
		if (!g_global_resource->sdfPipeline) {
			auto mesh = Shader::create(device, "shader/font_atlas/mesh_sdf.mesh.glsl.spv");
			auto frag = Shader::create(device, "shader/font_atlas/mesh_sdf.frag.glsl.spv");
			
			MeshPipelineCreateInfo pipeline_info = {
				.meshShader        = mesh,
				.fragmentShader    = frag
			};

			g_global_resource->sdfPipeline = Pipeline::create(device, pipeline_info);
		}

		resource->pipeline     = g_global_resource->sdfPipeline;
		resource->pcStageFlags = ShaderStageFlagBits::Mesh;
	} break;
	case AtlasType::PSDF: {
		if (!g_global_resource->psdfPipeline) {
			auto mesh = Shader::create(device, "shader/font_atlas/mesh_sdf.mesh.glsl.spv");
			auto frag = Shader::create(device, "shader/font_atlas/mesh_psdf.frag.glsl.spv");
			
			MeshPipelineCreateInfo pipeline_info = {
				.meshShader        = mesh,
				.fragmentShader    = frag
			};

			g_global_resource->psdfPipeline = Pipeline::create(device, pipeline_info);
		}

		resource->pipeline     = g_global_resource->psdfPipeline;
		resource->pcStageFlags = ShaderStageFlagBits::Mesh;
	} break;
	case AtlasType::MSDF: {
		if (!g_global_resource->msdfPipeline) {
			auto mesh = Shader::create(device, "shader/font_atlas/mesh_sdf.mesh.glsl.spv");
			auto frag = Shader::create(device, "shader/font_atlas/mesh_msdf.frag.glsl.spv");
			
			MeshPipelineCreateInfo pipeline_info = {
				.meshShader        = mesh,
				.fragmentShader    = frag
			};

			g_global_resource->msdfPipeline = Pipeline::create(device, pipeline_info);
		}

		resource->pipeline     = g_global_resource->msdfPipeline;
		resource->pcStageFlags = ShaderStageFlagBits::Mesh;
	} break;
	case AtlasType::MTSDF: {
		if (!g_global_resource->mtsdfPipeline) {
			auto mesh = Shader::create(device, "shader/font_atlas/mesh_sdf.mesh.glsl.spv");
			auto frag = Shader::create(device, "shader/font_atlas/mesh_mtsdf.frag.glsl.spv");
			
			MeshPipelineCreateInfo pipeline_info = {
				.meshShader        = mesh,
				.fragmentShader    = frag
			};

			g_global_resource->mtsdfPipeline = Pipeline::create(device, pipeline_info);
		}

		resource->pipeline     = g_global_resource->mtsdfPipeline;
		resource->pcStageFlags = ShaderStageFlagBits::Mesh;
	} break;
	default:
		VERA_ASSERT_MSG(false, "invalid atlas type");
	}
	
	auto pipeline_layout = resource->pipeline->getPipelineLayout().cref();
	auto desc_set_layout = pipeline_layout->getDescriptorSetLayout(0).cref();

	resource->device         = device;
	resource->descriptorPool = DescriptorPool::create(device);
	resource->descriptorSet  = resource->descriptorPool->allocate(desc_set_layout, 64);
	resource->commandBuffer  = CommandBuffer::create(device);

	return resource;
}

static void normalize_contour(
	static_vector<MSDFGlyphPoint*, MAX_EDGE_COUNT>& edges,
	static_vector<MSDFGlyphPoint, MAX_POINT_COUNT>& points,
	const Glyph::ContourType&                       contour
) {
	uint32_t state     = 0;
	uint32_t point_idx = 0;

	edges.clear();
	points.clear();

	for (const auto& point : contour) {
		switch (state) {
		case 0:
			points.emplace_back(point.position, true);
			state = 1;
			point_idx++;
			break;
		case 1:
			if (point.onCurve) {
				points.emplace_back(point.position, true);
				edges.push_back(points.end() - 2);
			} else {
				points.emplace_back(point.position, false);
				state = 2;
			}
			point_idx++;
			break;
		case 2:
			if (point.onCurve) {
				points.emplace_back(point.position, true);
				edges.push_back(points.end() - 3);
				state = 1;
			} else {
				float2 mid_point = (points[point_idx - 1].position + point.position) * 0.5f;

				points.emplace_back(mid_point, true);
				edges.push_back(points.end() - 3);
				point_idx++;
				points.emplace_back(point.position, false);
			}
			point_idx++;
			break;
		}
	}

	points.emplace_back(contour.front().position, true);
	edges.push_back(points.end() - (state == 1 ? 2 : 3));
}

static void get_edge_directions(const MSDFGlyphPoint* p0, float2& dir0, float2& dir1)
{
	const MSDFGlyphPoint* p1 = p0 + 1;

	if (p1->onCurve) { // line segment
		dir0 = dir1 = normalize(p1->position - p0->position);
	} else { // quadratic bezier segment
		const MSDFGlyphPoint* p2 = p0 + 2;
		dir0 = normalize(p1->position - p0->position);
		dir1 = normalize(p2->position - p1->position);
	}
}

static bool is_corner(const float2& a_dir, const float2& b_dir, double threshold)
{
	return dot(a_dir, b_dir) <= 0 || fabs(cross(a_dir, b_dir)) > threshold;
}

static void split_edge_in_third(
	static_vector<MSDFGlyphPoint*, 6>& new_edges,
	static_vector<MSDFGlyphPoint, 18>&  new_points,
	const MSDFGlyphPoint*               edge
) {
	if (new_edges.empty()) {
		new_points.emplace_back(edge->position, true);
		new_edges.push_back(&new_points.front());
	}

	if ((edge + 1)->onCurve) { // line segment
		float2 p0 = edge->position;
		float2 p3 = (edge + 1)->position;
		float2 p1 = lerp(p0, p3, 1.f / 3.f);
		float2 p2 = lerp(p0, p3, 2.f / 3.f);

		new_points.emplace_back(p1, true);
		new_edges.push_back(&new_points.back());
		new_points.emplace_back(p2, true);
		new_edges.push_back(&new_points.back());
		new_points.emplace_back(p3, true);
	} else { // quadratic bezier segment
		float2 bp0 = edge->position;
		float2 bp1 = (edge + 1)->position;
		float2 bp2 = (edge + 2)->position;
		float2 p0  = lerp(bp0, bp1, 1.f / 3.f);
		float2 p1  = quadratic(bp0, bp1, bp2, 1.f / 3.f);
		float2 p2  = lerp(lerp(bp0, bp1, 5.f / 9.f), lerp(bp1, bp2, 4.f / 9.f), 0.5);
		float2 p3  = quadratic(bp0, bp1, bp2, 2.f / 3.f);
		float2 p4  = lerp(bp1, bp2, 2.f / 3.f);

		new_points.emplace_back(p0, false);
		new_points.emplace_back(p1, true);
		new_edges.push_back(&new_points.back());
		new_points.emplace_back(p2, false);
		new_points.emplace_back(p3, true);
		new_edges.push_back(&new_points.back());
		new_points.emplace_back(p4, false);
		new_points.emplace_back(bp2, true);
	}
}

static void fill_msdf_contour_points(
	std::vector<SDFGlyphPoint>& glyph_points,
	array_view<MSDFGlyphPoint>  points,
	bool                        reverse
) {
	if (reverse) {
		auto first = points.rbegin() + 1;
		auto last  = points.rend() - 1;

		glyph_points.push_back(encode_msdf_glyph_point(points.front()));
		for (auto it = first; it != last; ++it)
			glyph_points.push_back(encode_msdf_glyph_point(*it));
		glyph_points.push_back(encode_msdf_glyph_point(points.back()));
	} else {
		for (const auto& point : points)
			glyph_points.push_back(encode_msdf_glyph_point(point));
	}

	glyph_points.push_back(END_CONTOUR);
}

static void simple_coloring(
	std::vector<SDFGlyphPoint>& glyph_points,
	const Glyph&                glyph,
	bool                        reverse_contours,
	hash_t                      seed
) {
	static_vector<MSDFGlyphPoint*, MAX_EDGE_COUNT> edges;
	static_vector<MSDFGlyphPoint, MAX_POINT_COUNT> points;
	static_vector<uint32_t, MAX_POINT_COUNT>       corners;

	EdgeColor color = init_edge_color(seed);

	for (const auto& contour : glyph.contours) {
		normalize_contour(edges, points, contour);

		float2 dir0;
		float2 dir1;
		float2 prev_dir = normalize(
			(points.end() - 1)->position -
			(points.end() - 2)->position);

		for (size_t i = 0; i < edges.size(); ++i) {
			get_edge_directions(edges[i], dir0, dir1);

			if (is_corner(prev_dir, dir0, sinf(3.0)))
				corners.push_back(static_cast<uint32_t>(i));

			prev_dir = dir1;
		}

		switch_edge_color(color, seed);
		
		if (corners.empty()) {
			for (MSDFGlyphPoint* edge : edges)
				edge->color = color;
		} else if (corners.size() == 1) {
			EdgeColor colors[3];
			colors[0] = color;
			colors[1] = EDGE_COLOR_WHITE;
			switch_edge_color(color, seed);
			colors[2] = color;

			uint32_t corner = corners[0];

			if (edges.size() >= 3) {
				uint32_t edge_count = static_cast<uint32_t>(edges.size());
				for (uint32_t i = 0; i < edge_count; ++i)
					edges[(corner + i) % edge_count]->color = colors[1 + symmetrical_trichotomy(i, edge_count)];
			} else if (edges.size() >= 1) {
				static_vector<MSDFGlyphPoint*, 6> new_edges;
				static_vector<MSDFGlyphPoint, 18>  new_points;

				split_edge_in_third(new_edges, new_points, edges[0]);

				if (edges.size() == 1) {
					new_edges[0]->color = colors[0];
					new_edges[1]->color = colors[1];
					new_edges[2]->color = colors[1];
				} else /* edges.size() == 2 */ {
					split_edge_in_third(new_edges, new_points, edges[1]);
					new_edges[0]->color = colors[0];
					new_edges[1]->color = colors[0];
					new_edges[2]->color = colors[1];
					new_edges[3]->color = colors[1];
					new_edges[4]->color = colors[2];
					new_edges[5]->color = colors[2];
				}

				fill_msdf_contour_points(glyph_points, new_points, reverse_contours);
				continue;
			}
		} else {
			EdgeColor initial_color = color;
			uint32_t  corner_count  = static_cast<uint32_t>(corners.size());
			uint32_t  edge_count    = static_cast<uint32_t>(edges.size());
			uint32_t  start         = corners[0];
			uint32_t  spline        = 0;

			for (uint32_t i = 0; i < edge_count; ++i) {
				uint32_t idx = (start + i) % edge_count;
				
				if (spline + 1 < corner_count && corners[spline + 1] == idx) {
					++spline;

					if (spline == corner_count - 1)
						switch_edge_color(color, seed, initial_color);
					else
						switch_edge_color(color, seed);
				}
				
				edges[idx]->color = color;
			}
		}

		fill_msdf_contour_points(glyph_points, points, reverse_contours);
	}

	glyph_points.back() = END_GLYPH;
}

static void fill_msdf_vertices(
	std::vector<SDFVertex>&     vertices,
	std::vector<SDFGlyphPoint>& glyph_points,
	priv::GlyphPage&            page,
	const Glyph&                glyph,
	float                       scale,
	float                       sdf_padding2
) {
	if (page.glyphMap.contains(glyph.glyphID)) return;

	if (page.textureCount == 0)
		page.textureCount = 1;

	const float2   size   = glyph.aabb.size();
	const extent2d extent = {
		static_cast<uint32_t>(round(scale * size.x + sdf_padding2)),
		static_cast<uint32_t>(round(scale * size.y + sdf_padding2))
	};

	urect2d rect;
	if (!page.packer->pack(extent, rect)) {
		page.packer->clear();

		if (!page.packer->pack(extent, rect))
			throw Exception("unable to pack glyph into the atlas texture");

		page.textureCount++;
	}

	auto& packed_glyph = page.glyphMap.emplace(glyph.glyphID, PackedGlyph{}).first->second;
	packed_glyph.glyphID = glyph.glyphID;
	packed_glyph.px      = page.px;
	packed_glyph.layer   = page.textureCount - 1;
	packed_glyph.rect	 = AABB2D(
		static_cast<float>(rect.min_x()),
		static_cast<float>(rect.min_y()),
		static_cast<float>(rect.max_x()),
		static_cast<float>(rect.max_y())
	);

	vertices.push_back(SDFVertex{
		.position      = uint2{ rect.min_x(), rect.min_y() },
		.fontCoordMin  = glyph.aabb.min(),
		.fontCoordMax  = glyph.aabb.max(),
		.storageOffset = static_cast<uint32_t>(glyph_points.size()),
		.layerIndex    = packed_glyph.layer
	});

	float2 outside_point = glyph.aabb.min() - float2(sdf_padding2) / scale;

	hash_t seed = 0;
	hash_combine(seed, glyph.glyphID);

	simple_coloring(
		glyph_points,
		glyph,
		glyph_sdf(glyph, outside_point),
		seed);
}

static void fill_sdf_vertices(
	std::vector<SDFVertex>&     vertices,
	std::vector<SDFGlyphPoint>& glyph_points,
	priv::GlyphPage&            page,
	const Glyph&                glyph,
	float                       scale,
	float                       sdf_padding2
) {
	if (page.glyphMap.contains(glyph.glyphID)) return;

	if (page.textureCount == 0)
		page.textureCount = 1;

	const float2   size   = glyph.aabb.size();
	const extent2d extent = {
		static_cast<uint32_t>(round(scale * size.x + sdf_padding2)),
		static_cast<uint32_t>(round(scale * size.y + sdf_padding2))
	};

	urect2d rect;
	if (!page.packer->pack(extent, rect)) {
		page.packer->clear();

		if (!page.packer->pack(extent, rect))
			throw Exception("unable to pack glyph into the atlas texture");

		page.textureCount++;
	}

	auto& packed_glyph = page.glyphMap.emplace(glyph.glyphID, PackedGlyph{}).first->second;
	packed_glyph.glyphID = glyph.glyphID;
	packed_glyph.px      = page.px;
	packed_glyph.layer   = page.textureCount - 1;
	packed_glyph.rect	 = AABB2D(
		static_cast<float>(rect.min_x()),
		static_cast<float>(rect.min_y()),
		static_cast<float>(rect.max_x()),
		static_cast<float>(rect.max_y())
	);

	vertices.push_back(SDFVertex{
		.position      = uint2{ rect.min_x(), rect.min_y() },
		.fontCoordMin  = glyph.aabb.min(),
		.fontCoordMax  = glyph.aabb.max(),
		.storageOffset = static_cast<uint32_t>(glyph_points.size()),
		.layerIndex    = packed_glyph.layer
	});

	const float2 outside_point = glyph.aabb.min() - float2(sdf_padding2) / scale;

	if (glyph_sdf(glyph, outside_point) > 0.0) {
		for (const auto& contour : glyph.contours) {
			auto first = contour.crbegin();
			auto last  = contour.crend();
			
			if (!first->onCurve) {
				glyph_points.push_back(encode_sdf_glyph_point(contour[0]));
				--last;
			}

			for (auto iter = first; iter != last; ++iter)
				glyph_points.push_back(encode_sdf_glyph_point(*iter));
			glyph_points.push_back(END_CONTOUR);
		}
	} else {
		for (const auto& contour : glyph.contours) {
			for (const auto& point : contour)
				glyph_points.push_back(encode_sdf_glyph_point(point));
			glyph_points.push_back(END_CONTOUR);
		}
	}

	glyph_points.back() = END_GLYPH;
}

static void update_storage_descriptor_set(
	ref<DescriptorSet> storage_descriptor_set,
	ref<Buffer>        storage_buffer,
	uint32_t           dst_binding
) {
	DescriptorBindingInfo binding_info;
	binding_info.descriptorType       = DescriptorType::StorageBuffer;
	binding_info.dstBinding           = dst_binding;
	binding_info.dstArrayElement      = 0;
	binding_info.storageBuffer.buffer = storage_buffer;
	binding_info.storageBuffer.offset = 0;
	binding_info.storageBuffer.range  = storage_buffer->size();
	
	storage_descriptor_set->setDescriptorBindingInfo(binding_info);
}

static void upload_sdf_buffer(
	priv::FontAtlasResource&  resource,
	array_view<SDFVertex>     vertices,
	array_view<SDFGlyphPoint> glyph_points
) {
	size_t vert_bytes = sizeof(SDFVertex) * vertices.size();
	size_t stor_bytes = sizeof(SDFGlyphPoint) * glyph_points.size();

	if (!resource.vertexBuffer) {
		resource.vertexBuffer = Buffer::createStorage(resource.device, vert_bytes);
		update_storage_descriptor_set(resource.descriptorSet, resource.vertexBuffer, 0);
	} else if (resource.vertexBuffer->size() < vert_bytes) {
		resource.vertexBuffer->resize(vert_bytes);
		update_storage_descriptor_set(resource.descriptorSet, resource.vertexBuffer, 0);
	}

	if (!resource.storageBuffer) {
		resource.storageBuffer = Buffer::createStorage(resource.device, stor_bytes);
		update_storage_descriptor_set(resource.descriptorSet, resource.storageBuffer, 1);
	} else if (resource.storageBuffer->size() < stor_bytes) {
		resource.storageBuffer->resize(stor_bytes);
		update_storage_descriptor_set(resource.descriptorSet, resource.storageBuffer, 1);
	}

	auto* vert_map = resource.vertexBuffer->getDeviceMemory()->map();
	auto* stor_map = resource.storageBuffer->getDeviceMemory()->map();

	memcpy(vert_map, vertices.data(), vert_bytes);
	memcpy(stor_map, glyph_points.data(), stor_bytes);

	resource.vertexBuffer->getDeviceMemory()->flush();
	resource.storageBuffer->getDeviceMemory()->flush();
}

static void prepare_page_textures(
	priv::FontAtlasResource&   resource,
	priv::GlyphPage&           page,
	const FontAtlasCreateInfo& info
) {
	uint32_t dst_array_idx        = 0;
	uint32_t curr_texture_count   = static_cast<uint32_t>(page.textures.size());
	uint32_t needed_texture_count = page.textureCount;

	for (uint32_t i = curr_texture_count; i < needed_texture_count; ++i) {
		if (page.fullyPackedCount == page.textures.size()) {
			TextureCreateInfo texture_info = {
				.format = get_font_atlas_format(info.type),
				.usage  = TextureUsageFlagBits::Storage | TextureUsageFlagBits::Sampled,
				.width  = info.atlasWidth,
				.height = info.atlasHeight
			};

			page.textures.push_back(Texture::create(resource.device, texture_info));
			page.packer->clear();
		}

		DescriptorBindingInfo binding_info;
		binding_info.descriptorType             = DescriptorType::StorageImage;
		binding_info.dstBinding                 = 2;
		binding_info.dstArrayElement            = dst_array_idx++;
		binding_info.storageImage.textureView   = page.textures.back()->getTextureView();
		binding_info.storageImage.textureLayout = TextureLayout::General;
		
		resource.descriptorSet->setDescriptorBindingInfo(binding_info);

		page.fullyPackedCount++;
	}
}

// renders SDF, PSDF font glyphs into the atlas textures
static CommandBufferSync render_sdf_glyph(
	priv::FontAtlasResource&          resource,
	priv::GlyphPage&                  page,
	const FontAtlasCreateInfo&        info,
	const std::vector<SDFVertex>&     vertices,
	const std::vector<SDFGlyphPoint>& glyph_points,
	float                             scale
) {
	struct SDFPushConstantData {
		uint32_t vertexOffset;
		uint32_t vertexCount;
		float    scale;
		float    sdfPadding;
		float2   resolution;
	};

	Viewport viewport = {
		.posX     = 0.f,
		.posY     = 0.f,
		.width    = static_cast<float>(info.atlasWidth),
		.height   = static_cast<float>(info.atlasHeight)
	};

	Scissor scissor = {
		.minX = 0,
		.minY = 0,
		.maxX = info.atlasWidth,
		.maxY = info.atlasHeight
	};

	RenderingInfo rendering_info = {
		.renderArea = irect2d{ 0, 0, info.atlasWidth, info.atlasHeight },
		.layerCount = 1
	};

	SDFPushConstantData pc_data = {
		.vertexOffset = 0,
		.vertexCount  = static_cast<uint32_t>(vertices.size()),
		.scale        = scale,
		.sdfPadding   = info.sdfPadding / pc_data.scale,
		.resolution   = float2{ viewport.width, viewport.height }
	};

	auto pipeline_layout = resource.pipeline->getPipelineLayout().cref();
	auto cmd_buffer      = resource.commandBuffer.ref();

	if (!resource.commandBufferSync.empty())
		resource.commandBufferSync.waitForComplete();

	uint32_t texture_offset = page.textures.size();

	upload_sdf_buffer(resource, vertices, glyph_points);
	prepare_page_textures(resource, page, info);

	cmd_buffer->reset();
	cmd_buffer->begin();
	cmd_buffer->setViewport(viewport);
	cmd_buffer->setScissor(scissor);
	cmd_buffer->bindDescriptorSet(pipeline_layout, 0, resource.descriptorSet);
	cmd_buffer->pushConstant(pipeline_layout, ShaderStageFlagBits::Mesh, 0, &pc_data, sizeof(pc_data));
	cmd_buffer->bindPipeline(resource.pipeline);
	cmd_buffer->beginRendering(rendering_info);
	cmd_buffer->drawMeshTask((pc_data.vertexCount - 1) / 64 + 1, 1, 1);
	cmd_buffer->endRendering();

	for (uint32_t i = texture_offset; i < page.textures.size(); ++i) {
		cmd_buffer->transitionImageLayout(
			page.textures[i],
			PipelineStageFlagBits::FragmentShader,
			PipelineStageFlagBits::FragmentShader,
			AccessFlagBits::ShaderWrite,
			AccessFlagBits::ShaderRead,
			TextureLayout::Undefined,
			TextureLayout::ShaderReadOnlyOptimal
		);
	}

	cmd_buffer->end();

	return resource.commandBufferSync = cmd_buffer->submit();
}

static priv::GlyphPage* get_glyph_page(
	std::unordered_map<uint32_t, priv::GlyphPage>& pages,
	const FontAtlasCreateInfo&                     info,
	uint32_t                                       px
) {
	auto it = pages.find(px);

	if (it != pages.end())
		return &it->second;
	
	auto [iter, success] = pages.emplace(px, priv::GlyphPage{});
	auto& new_page       = iter->second;
	new_page.px               = px;
	new_page.fullyPackedCount = 0;
	new_page.textureCount     = 0;
	new_page.packer           = RectPacker::createUnique(
		info.packingMethod,
		info.atlasWidth,
		info.atlasHeight,
		info.padding
	);

	return &new_page;
}

static CommandBufferSync load_msdf_glyph(
	priv::FontAtlasResource&    resource,
	const FontAtlasCreateInfo&  info,
	const priv::FontImplBase&   impl,
	priv::GlyphPage&            page,
	const basic_range<GlyphID>& range
) {
	std::vector<SDFVertex>     vertices;
	std::vector<SDFGlyphPoint> glyph_points;

	float scale        = get_font_scale(impl, page.px);
	float sdf_padding2 = 2.f * info.sdfPadding;
	
	for (GlyphID glyph_id : range) {
		fill_msdf_vertices(
			vertices,
			glyph_points,
			page,
			impl.findGlyph(glyph_id),
			scale,
			sdf_padding2
		);
	}

	if (vertices.empty()) return {};

	return render_sdf_glyph(
		resource,
		page,
		info,
		vertices,
		glyph_points,
		scale
	);
}

static CommandBufferSync load_msdf_glyph(
	priv::FontAtlasResource&   resource,
	const FontAtlasCreateInfo& info,
	const priv::FontImplBase&  impl,
	priv::GlyphPage&           page,
	const CodeRange&           range
) {
	return {};
}

static CommandBufferSync load_sdf_glyph(
	priv::FontAtlasResource&    resource,
	const FontAtlasCreateInfo&  info,
	const priv::FontImplBase&   impl,
	priv::GlyphPage&            page,
	const basic_range<GlyphID>& range
) {
	std::vector<SDFVertex>     vertices;
	std::vector<SDFGlyphPoint> glyph_points;

	float scale        = get_font_scale(impl, page.px);
	float sdf_padding2 = 2.f * info.sdfPadding;
	
	for (GlyphID glyph_id : range) {
		fill_sdf_vertices(
			vertices,
			glyph_points,
			page,
			impl.findGlyph(glyph_id),
			scale,
			sdf_padding2
		);
	}

	if (vertices.empty()) return {};

	return render_sdf_glyph(
		resource,
		page,
		info,
		vertices,
		glyph_points,
		scale
	);
}

static CommandBufferSync load_sdf_glyph(
	priv::FontAtlasResource&   resource,
	const FontAtlasCreateInfo& info,
	const priv::FontImplBase&  impl,
	priv::GlyphPage&           page,
	const CodeRange&           range
) {
	if (range.getUnicodeRange() == UnicodeRange::ALL) {
		return load_sdf_glyph(
			resource,
			info,
			impl,
			page,
			basic_range<GlyphID>{ 0, info.font->getGlyphCount() + 1 }
		);
	} 

	std::vector<SDFVertex>     vertices;
	std::vector<SDFGlyphPoint> glyph_points;

	float scale        = get_font_scale(impl, page.px);
	float sdf_padding2 = 2.f * info.sdfPadding;

	for (const char32_t codepoint : range) {
		GlyphID glyph_id = info.font->getGlyphID(codepoint);
		if (page.glyphMap.contains(glyph_id)) continue;

		fill_sdf_vertices(
			vertices,
			glyph_points,
			page,
			impl.findGlyph(glyph_id),
			scale,
			sdf_padding2
		);
	}

	if (vertices.empty()) return {};

	return render_sdf_glyph(
		resource,
		page,
		info,
		vertices,
		glyph_points,
		scale
	);
}

obj<FontAtlas> FontAtlas::create(obj<Device> device, const FontAtlasCreateInfo& info)
{
	auto new_obj = obj<FontAtlas>(new FontAtlas());

	new_obj->m_device = std::move(device);
	new_obj->m_info   = info;

	if (info.sdfFontSize == 0)
		new_obj->m_info.sdfFontSize = 48; // default size

	return new_obj;
}

FontAtlas::~FontAtlas() VERA_NOEXCEPT
{
}

obj<Font> FontAtlas::getFont() const VERA_NOEXCEPT
{
	return m_info.font;
}

obj<TextureView> FontAtlas::getTextureView(uint32_t px, uint32_t layer) VERA_NOEXCEPT
{
	px = get_font_size(m_info, px);

	if (auto it = m_pages.find(px); it != m_pages.cend())
		if (layer < it->second.textures.size())
			return unsafe_obj_cast<TextureView>(it->second.textures[layer]->getTextureView());

	return nullptr;
}

uint32_t FontAtlas::getTextureCount(uint32_t px) const VERA_NOEXCEPT
{
	px = get_font_size(m_info, px);

	if (auto it = m_pages.find(px); it != m_pages.cend())
		return static_cast<uint32_t>(it->second.textures.size());

	return 0;
}

extent2d FontAtlas::getTextureSize() const VERA_NOEXCEPT
{
	return extent2d{ m_info.atlasWidth, m_info.atlasHeight };
}

CommandBufferSync FontAtlas::loadGlyphRange(const basic_range<GlyphID>& range, uint32_t px)
{
	m_info.font->loadGlyphRange(range);

	uint32_t         font_px = get_font_size(m_info, px);
	priv::GlyphPage* page = get_glyph_page(m_pages, m_info, font_px);

	if (!m_resource)
		m_resource = create_font_atlas_resource(m_device, m_info);

	switch (m_info.type) {
	case AtlasType::HardMask:
	case AtlasType::SoftMask:
		break;
	case AtlasType::SDF:
	case AtlasType::PSDF:
		return load_sdf_glyph(
			*m_resource,
			m_info,
			*m_info.font->m_impl,
			*page,
			range
		);
	case AtlasType::MSDF:
	case AtlasType::MTSDF:
		return load_msdf_glyph(
			*m_resource,
			m_info,
			*m_info.font->m_impl,
			*page,
			range
		);
		break;
	}

	VERA_ASSERT_MSG(false, "invalid atlas type");
	return {};
}

CommandBufferSync FontAtlas::loadCodeRange(const CodeRange& range, uint32_t px)
{
	m_info.font->loadCodeRange(range);
	
	uint32_t         font_px = get_font_size(m_info, px);
	priv::GlyphPage* page = get_glyph_page(m_pages, m_info, font_px);

	if (!m_resource)
		m_resource = create_font_atlas_resource(m_device, m_info);

	switch (m_info.type) {
	case AtlasType::HardMask:
	case AtlasType::SoftMask:
		break;
	case AtlasType::SDF:
	case AtlasType::PSDF:
		return load_sdf_glyph(
			*m_resource,
			m_info,
			*m_info.font->m_impl,
			*page,
			range
		);
		break;
	case AtlasType::MSDF:
	case AtlasType::MTSDF:
		return load_msdf_glyph(
			*m_resource,
			m_info,
			*m_info.font->m_impl,
			*page,
			range
		);
		break;
	}

	VERA_ASSERT_MSG(false, "invalid atlas type");
	return {};
}

const PackedGlyph& FontAtlas::getGlyph(char32_t codepoint, uint32_t px)
{
	uint32_t font_px  = get_font_size(m_info, px);
	auto     page_it  = m_pages.find(font_px);
	
	if (page_it == m_pages.end())
		loadCodeRange(codepoint, font_px).waitForComplete();

	GlyphID glyph_id = m_info.font->getGlyphID(codepoint);

	auto it = page_it->second.glyphMap.find(glyph_id);
	if (it == page_it->second.glyphMap.end())
		throw Exception("glyph not found in font atlas");

	return it->second;
}

AtlasType FontAtlas::getAtlasType() const VERA_NOEXCEPT
{
	return m_info.type;
}

VERA_NAMESPACE_END
