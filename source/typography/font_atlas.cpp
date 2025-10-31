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
#include "../../include/vera/util/rect_packer.h"
#include "../../include/vera/util/renderdoc.h"
#include "font_impl.h"

#define FLOAT_INF        0x7f800000
#define FLAG_NONE        0x0u
#define FLAG_ON_CURVE    0x1u
#define FLAG_END_CONTOUR 0x2u
#define FLAG_END_GLYPH   0x4u
#define MAX_CONTOUR_SIZE 128

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
	uint32_t                                 px;
	std::vector<obj<Texture>>                textures;
	std::unordered_map<GlyphID, PackedGlyph> glyphMap;
	std::unique_ptr<RectPacker>              packer;
};

VERA_PRIV_NAMESPACE_END

static weak_obj<priv::FontAtlasGlobalResource> g_global_resource;

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

struct SDFGlyphPoint
{
	float2 position;
};

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

static float2 encode_glyph_point(const GlyphPoint& gp)
{
	float2   result = gp.position;
	uint32_t mask_x = (std::bit_cast<uint32_t>(result.x) & 0xfffffffe) | static_cast<uint32_t>(gp.onCurve);

	result.x = std::bit_cast<float>(mask_x);

	return result;
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

static float get_font_scale(const priv::FontImpl& impl, uint32_t px)
{
	return static_cast<float>(px) / static_cast<float>(impl.font.head.unitsPerEm);
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
				.fragmentShader    = frag,
				.rasterizationInfo = RasterizationInfo{},
				.depthStencilInfo  = DepthStencilInfo{},
				.colorBlendInfo    = ColorBlendInfo{
					.attachments = { 
						ColorBlendAttachmentState{}
					}
				},
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
				.fragmentShader    = frag,
				.rasterizationInfo = RasterizationInfo{},
				.depthStencilInfo  = DepthStencilInfo{},
				.colorBlendInfo    = ColorBlendInfo{
					.attachments = { 
						ColorBlendAttachmentState{}
					}
				},
			};

			g_global_resource->psdfPipeline = Pipeline::create(device, pipeline_info);
		}

		resource->pipeline     = g_global_resource->psdfPipeline;
		resource->pcStageFlags = ShaderStageFlagBits::Mesh;
	} break;
	case AtlasType::MSDF: {
	} break;
	case AtlasType::MTSDF: {
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

static void fill_vertices_by_glyph_id(
	std::vector<SDFVertex>&     vertices,
	std::vector<extent2d>&      glyph_sizes,
	std::vector<SDFGlyphPoint>& glyph_points,
	std::vector<PackedGlyph*>&  packed_glyphs,
	priv::GlyphPage&            page,
	GlyphID                     glyph_id,
	const priv::FontImpl&       impl,
	float                       scale,
	float                       sdf_padding2
) {
	static const SDFGlyphPoint end_contour{ .position = { FLOAT_INF, 0.0f } };

	auto glyph_it = impl.font.glyphs.find(glyph_id);

	VERA_ASSERT(glyph_it != impl.font.glyphs.cend());

	auto& packed_glyph = page.glyphMap.emplace(glyph_id, PackedGlyph{}).first->second;
	packed_glyph.glyphID = glyph_id;
	packed_glyph.px      = page.px;

	packed_glyphs.push_back(&packed_glyph);

	const auto& glyph = glyph_it->second;
	const auto& size  = glyph.aabb.size();

	glyph_sizes.push_back(extent2d{
		static_cast<uint32_t>(round(scale * size.x + sdf_padding2)),
		static_cast<uint32_t>(round(scale * size.y + sdf_padding2))
	});

	vertices.push_back(SDFVertex{
		.fontCoordMin  = glyph.aabb.min(),
		.fontCoordMax  = glyph.aabb.max(),
		.storageOffset = static_cast<uint32_t>(glyph_points.size()),
	});

	float2 outside_point = glyph.aabb.min() - float2(sdf_padding2) / scale;

	if (glyph_sdf(glyph, outside_point) > 0.0) {
		for (const auto& contour : glyph.contours) {
			auto first = contour.crbegin();
			auto last  = contour.crend();
			
			if (!first->onCurve) {
				glyph_points.push_back(SDFGlyphPoint{
					.position = encode_glyph_point(contour[0]),
				});
				--last;
			}

			for (auto iter = first; iter != last; ++iter) {
				glyph_points.push_back(SDFGlyphPoint{
					.position = encode_glyph_point(*iter),
				});
			}

			glyph_points.push_back(end_contour);
		}
	} else {
		for (const auto& contour : glyph.contours) {
			for (const auto& point : contour) {
				glyph_points.push_back(SDFGlyphPoint{
					.position = encode_glyph_point(point),
				});
			}

			glyph_points.push_back(end_contour);
		}
	}

	glyph_points.back().position.y = FLOAT_INF;
}

static void pack_font_glyph(
	std::vector<SDFVertex>&      vertices,
	std::vector<PackedGlyph*>&   packed_glyphs,
	std::vector<uint32_t>&       packed_counts,
	RectPacker&                  packer,
	const std::vector<extent2d>& glyph_sizes,
	uint32_t                     layer_offset
) {
	std::vector<urect2d> glyph_rects;

	bool failed_packing = false;

	for (uint32_t packed_count = 0, vert_idx = 0; packed_count < glyph_sizes.size();) {
		auto glyph_size_view = array_view<extent2d>{ glyph_sizes }.subview(packed_count);
		
		uint32_t packed = packer.pack(glyph_size_view, glyph_rects);
		packed_counts.push_back(packed);
		packed_count += packed;

		if (packed == 0 && failed_packing)
			throw Exception("unable to pack glyphs into the atlas texture");

		if (failed_packing = (packed == 0)) {
			packer.clear();
			continue;
		}

		for (const auto& rect : glyph_rects) {
			auto& vertex = vertices[vert_idx];
			vertex.position   = rect.upper_left();
			vertex.layerIndex = layer_offset;

			auto& packed_glyph = *packed_glyphs[vert_idx++];
			packed_glyph.layer = layer_offset;
			packed_glyph.rect  = AABB2D(
				static_cast<float>(rect.min_x()),
				static_cast<float>(rect.min_y()),
				static_cast<float>(rect.max_x()),
				static_cast<float>(rect.max_y())
			);
		}

		++layer_offset;
	}
}

static bool fill_font_vertices(
	std::vector<SDFVertex>&     vertices,
	std::vector<SDFGlyphPoint>& glyph_points,
	std::vector<uint32_t>&      packed_counts,
	priv::GlyphPage&            page,
	const priv::FontImpl&       impl,
	const CodeRange&            range,
	float                       sdf_padding
) {
	std::vector<PackedGlyph*> packed_glyphs;
	std::vector<extent2d>     glyph_sizes;
	
	const auto& char_map     = impl.font.characterMap.charToGlyphMap;
	float       scale        = get_font_scale(impl, page.px);
	float       sdf_padding2 = 2.f * sdf_padding;

	if (range.getUnicodeRange() == UnicodeRange::ALL) {
		uint32_t glyph_count = impl.font.maxProfile.numGlyphs;

		for (GlyphID glyph_id = 0; glyph_id < glyph_count; ++glyph_id) {
			fill_vertices_by_glyph_id(
				vertices,
				glyph_sizes,
				glyph_points,
				packed_glyphs,
				page,
				glyph_id,
				impl,
				scale,
				sdf_padding2
			);
		}
	} else {
		for (auto codepoint = range.start(); codepoint <= range.end(); ++codepoint) {
			auto id_it = char_map.find(codepoint);
			if (id_it == char_map.cend()) continue;

			GlyphID glyph_id = id_it->second;
			if (page.glyphMap.contains(glyph_id)) continue;

			fill_vertices_by_glyph_id(
				vertices,
				glyph_sizes,
				glyph_points,
				packed_glyphs,
				page,
				glyph_id,
				impl,
				scale,
				sdf_padding2
			);
		}
	}

	if (vertices.empty()) return false;

	pack_font_glyph(
		vertices,
		packed_glyphs,
		packed_counts,
		*page.packer,
		glyph_sizes,
		static_cast<uint32_t>(page.textures.size())
	);

	return true;
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
	const FontAtlasCreateInfo& info,
	array_view<uint32_t>       packed_counts
) {
	uint32_t dst_array_idx = 0;

	for (uint32_t packed : packed_counts) {
		if (packed == 0 || page.textures.empty()) {
			TextureCreateInfo texture_info = {
				.format = Format::R32Float,
				.usage  = TextureUsageFlagBits::Storage,
				.width  = info.atlasWidth,
				.height = info.atlasHeight
			};

			page.textures.push_back(Texture::create(resource.device, texture_info));
			page.packer->clear();
			
			if (packed == 0) continue;
		}

		DescriptorBindingInfo binding_info;
		binding_info.descriptorType             = DescriptorType::StorageImage;
		binding_info.dstBinding                 = 2;
		binding_info.dstArrayElement            = dst_array_idx++;
		binding_info.storageImage.textureView   = page.textures.back()->getTextureView();
		binding_info.storageImage.textureLayout = TextureLayout::General;
		
		resource.descriptorSet->setDescriptorBindingInfo(binding_info);
	}
}

// renders SDF, PSDF, MSDF, MTSDF font glyphs into the atlas textures
static CommandBufferSync render_sdf_font(
	priv::FontAtlasResource&          resource,
	const priv::FontImpl&             impl,
	priv::GlyphPage&                  page,
	const FontAtlasCreateInfo&        info,
	const std::vector<SDFVertex>&     vertices,
	const std::vector<SDFGlyphPoint>& glyph_points,
	const std::vector<uint32_t>&      packed_counts
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
		.scale        = get_font_scale(impl, page.px),
		.sdfPadding   = info.sdfPadding / pc_data.scale,
		.resolution   = float2{ viewport.width, viewport.height }
	};

	auto pipeline_layout = resource.pipeline->getPipelineLayout().cref();
	auto cmd_buffer      = resource.commandBuffer.ref();

	if (!resource.commandBufferSync.empty())
		resource.commandBufferSync.waitForComplete();

	upload_sdf_buffer(resource, vertices, glyph_points);
	prepare_page_textures(resource, page, info, packed_counts);

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
	cmd_buffer->end();

	return cmd_buffer->submit();
}

static CommandBufferSync load_sdf_font(
	priv::FontAtlasResource&    resource,
	const priv::FontImpl&       impl,
	const FontAtlasCreateInfo&  info,
	priv::GlyphPage&            page,
	const basic_range<GlyphID>& range
) {
	std::vector<SDFVertex>     vertices;
	std::vector<SDFGlyphPoint> glyph_points;
	std::vector<extent2d>      glyph_sizes;
	std::vector<PackedGlyph*>  packed_glyphs;
	
	float scale        = get_font_scale(impl, page.px);
	float sdf_padding2 = 2.f * info.sdfPadding;
	
	for (GlyphID glyph_id : range) {
		fill_vertices_by_glyph_id(
			vertices,
			glyph_sizes,
			glyph_points,
			packed_glyphs,
			page,
			glyph_id,
			impl,
			scale,
			sdf_padding2
		);
	}

	if (vertices.empty())
		throw Exception("no glyphs to load in the specified range");

	std::vector<uint32_t> packed_counts;

	pack_font_glyph(
		vertices,
		packed_glyphs,
		packed_counts,
		*page.packer,
		glyph_sizes,
		static_cast<uint32_t>(page.textures.size())
	);

	return render_sdf_font(
		resource,
		impl,
		page,
		info,
		vertices,
		glyph_points,
		packed_counts
	);
}

static CommandBufferSync load_sdf_font(
	priv::FontAtlasResource&   resource,
	const priv::FontImpl&      impl,
	const FontAtlasCreateInfo& info,
	priv::GlyphPage&           page,
	const CodeRange&           range
) {
	std::vector<SDFVertex>     vertices;
	std::vector<SDFGlyphPoint> glyph_points;
	std::vector<extent2d>      glyph_sizes;
	std::vector<PackedGlyph*>  packed_glyphs;
	
	const auto& char_map     = impl.font.characterMap.charToGlyphMap;
	float       scale        = get_font_scale(impl, page.px);
	float       sdf_padding2 = 2.f * info.sdfPadding;

	if (range.getUnicodeRange() == UnicodeRange::ALL) {
		uint32_t glyph_count = impl.font.maxProfile.numGlyphs;

		for (GlyphID glyph_id = 0; glyph_id < glyph_count; ++glyph_id) {
			fill_vertices_by_glyph_id(
				vertices,
				glyph_sizes,
				glyph_points,
				packed_glyphs,
				page,
				glyph_id,
				impl,
				scale,
				sdf_padding2
			);
		}
	} else {
		for (auto codepoint = range.start(); codepoint <= range.end(); ++codepoint) {
			auto id_it = char_map.find(codepoint);
			if (id_it == char_map.cend()) continue;

			GlyphID glyph_id = id_it->second;
			if (page.glyphMap.contains(glyph_id)) continue;

			fill_vertices_by_glyph_id(
				vertices,
				glyph_sizes,
				glyph_points,
				packed_glyphs,
				page,
				glyph_id,
				impl,
				scale,
				sdf_padding2
			);
		}
	}

	if (vertices.empty())
		throw Exception("no glyphs to load in the specified range");

	std::vector<uint32_t> packed_counts;

	pack_font_glyph(
		vertices,
		packed_glyphs,
		packed_counts,
		*page.packer,
		glyph_sizes,
		static_cast<uint32_t>(page.textures.size())
	);

	return render_sdf_font(
		resource,
		impl,
		page,
		info,
		vertices,
		glyph_points,
		packed_counts
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

ref<Font> FontAtlas::getFont() const VERA_NOEXCEPT
{
	return m_info.font;
}

const_ref<Texture> FontAtlas::getTexture(uint32_t px, uint32_t layer) const VERA_NOEXCEPT
{
	if (auto it = m_pages.find(px); it != m_pages.cend())
		if (layer < it->second.textures.size())
			return const_ref<Texture>(it->second.textures[layer].get());

	return nullptr;
}

uint32_t FontAtlas::getTextureCount(uint32_t px) const VERA_NOEXCEPT
{
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
	FontResult       result  = m_info.font->loadGlyphRange(range);
	uint32_t         font_px = get_font_size(m_info, px);
	priv::GlyphPage* page = nullptr;

	if (result != FontResultType::Success)
		throw Exception("failed to load glyph range from font");

	if (auto page_it = m_pages.find(font_px); page_it == m_pages.end()) {
		auto [iter, success] = m_pages.emplace(font_px, priv::GlyphPage{});
		iter->second.px     = font_px;
		iter->second.packer = RectPacker::createUnique(
			m_info.packingMethod,
			m_info.atlasWidth,
			m_info.atlasHeight,
			m_info.padding
		);

		page = &iter->second;
	} else {
		page = &page_it->second;
	}

	switch (m_info.type) {
	case AtlasType::HardMask:
	case AtlasType::SoftMask:
		break;
	case AtlasType::SDF:
	case AtlasType::PSDF:
	case AtlasType::MSDF:
	case AtlasType::MTSDF:
		if (!m_resource)
			m_resource = create_font_atlas_resource(m_device, m_info);

		return load_sdf_font(
			*m_resource,
			*m_info.font->m_impl,
			m_info,
			*page,
			range
		);
	}

	VERA_ASSERT_MSG(false, "invalid atlas type");
	return {};
}

CommandBufferSync FontAtlas::loadCodeRange(const CodeRange& range, uint32_t px)
{
	FontResult       result  = m_info.font->loadCodeRange(range);
	uint32_t         font_px = get_font_size(m_info, px);
	priv::GlyphPage* page = nullptr;

	if (result != FontResultType::Success)
		throw Exception("failed to load glyph range from font");

	if (auto page_it = m_pages.find(font_px); page_it == m_pages.end()) {
		auto [iter, success] = m_pages.emplace(font_px, priv::GlyphPage{});
		iter->second.px     = font_px;
		iter->second.packer = RectPacker::createUnique(
			m_info.packingMethod,
			m_info.atlasWidth,
			m_info.atlasHeight,
			m_info.padding
		);

		page = &iter->second;
	} else {
		page = &page_it->second;
	}

	switch (m_info.type) {
	case AtlasType::HardMask:
	case AtlasType::SoftMask:
		break;
	case AtlasType::SDF:
	case AtlasType::PSDF:
	case AtlasType::MSDF:
	case AtlasType::MTSDF:
		if (!m_resource)
			m_resource = create_font_atlas_resource(m_device, m_info);

		return load_sdf_font(
			*m_resource,
			*m_info.font->m_impl,
			m_info,
			*page,
			range
		);
	}

	VERA_ASSERT_MSG(false, "invalid atlas type");
	return {};
}

const PackedGlyph& FontAtlas::getGlyph(char32_t codepoint, uint32_t px)
{
	uint32_t font_px = get_font_size(m_info, px);
	auto     page_it = m_pages.find(font_px);

	if (page_it == m_pages.end())
		loadCodeRange(codepoint, font_px);

	const auto& char_map = m_info.font->m_impl->font.characterMap;
	auto        glyph_it = char_map.charToGlyphMap.find(codepoint);
	if (glyph_it == char_map.charToGlyphMap.end())
		throw Exception("glyph not found in font");

	auto it = page_it->second.glyphMap.find(glyph_it->second);
	if (it == page_it->second.glyphMap.end())
		throw Exception("glyph not found in font atlas");

	return it->second;
}

AtlasType FontAtlas::getAtlasType() const VERA_NOEXCEPT
{
	return m_info.type;
}

VERA_NAMESPACE_END
