#version 450

#extension GL_EXT_mesh_shader : require

precision mediump float;

#define VERTEX_COUNT   4
#define TRIANGLE_COUNT 2
#define LOCAL_SIZE     64

layout(local_size_x=LOCAL_SIZE, local_size_y=1, local_size_z=1) in;
layout(triangles, max_vertices=VERTEX_COUNT * LOCAL_SIZE, max_primitives=TRIANGLE_COUNT * LOCAL_SIZE) out;

layout(location=0) out vec2 outFontCoord[];
layout(location=1) perprimitiveEXT out flat float outScale[];
layout(location=2) perprimitiveEXT out flat uint outStorageOffset[];
layout(location=3) perprimitiveEXT out flat uint outLayerIndex[];

layout(push_constant) uniform PushConstantBlock
{
	uint  vertexOffset;
	uint  vertexCount;
	float scale;
	float sdfPadding;
	vec2  resolution;
} pc;

struct Vertex
{
	uvec2 position;
	vec2  fontCoordMin;
	vec2  fontCoordMax;
	uint  storageOffset;
	uint  layerIndex;
};

layout(set=0, binding=0) readonly buffer VertexBufferBlock
{
	Vertex vertices[];
} vertexBuffer;

void main()
{
	// Each thread is responsible for one glyph.
	// We need to make sure we don't read out of bounds.
	uint vertex_idx = gl_WorkGroupID.x * LOCAL_SIZE + gl_LocalInvocationID.x;

	// The number of vertices and primitives should be set by a single thread in the workgroup.
	// It's often safest to do this with thread 0.
	// The values should reflect the maximum possible output for the workgroup.
	if (gl_LocalInvocationID.x == 0) {
		uint thread_count = min(pc.vertexCount - gl_WorkGroupID.x * LOCAL_SIZE, LOCAL_SIZE);
		SetMeshOutputsEXT(thread_count * VERTEX_COUNT, thread_count * TRIANGLE_COUNT);
	}

	// A barrier is required here to ensure SetMeshOutputsEXT completes
	// before any thread writes to the output arrays.
	barrier();

	uvec2 position       = vertexBuffer.vertices[vertex_idx].position;
	vec2  coord_min      = vertexBuffer.vertices[vertex_idx].fontCoordMin;
	vec2  coord_max      = vertexBuffer.vertices[vertex_idx].fontCoordMax;
	uint  storage_offset = vertexBuffer.vertices[vertex_idx].storageOffset;
	uint  layer_index    = vertexBuffer.vertices[vertex_idx].layerIndex;

	float pad = pc.sdfPadding;
	vec2  f0  = vec2(coord_min.x - pad, coord_max.y + pad);
	vec2  f1  = vec2(coord_max.x + pad, coord_max.y + pad);
	vec2  f2  = vec2(coord_max.x + pad, coord_min.y - pad);
	vec2  f3  = vec2(coord_min.x - pad, coord_min.y - pad);

	float width  = 2.0 * (f1.x - f0.x) * pc.scale / pc.resolution.x;
	float height = 2.0 * (f1.y - f2.y) * pc.scale / pc.resolution.y;
	vec2  pos    = 2.0 * vec2(position) / pc.resolution - vec2(1.0);
	vec2  p0     = vec2(pos.x, -pos.y);
	vec2  p1     = p0 + vec2(width, 0);
	vec2  p2     = p0 + vec2(width, -height);
	vec2  p3     = p0 + vec2(0, -height);

	// Each thread outputs its own vertices and primitives.
	// The offsets are calculated based on the thread's local invocation ID.
	uint vert_off = gl_LocalInvocationID.x * VERTEX_COUNT;
	uint prim_off = gl_LocalInvocationID.x * TRIANGLE_COUNT;

	gl_MeshVerticesEXT[vert_off + 0].gl_Position = vec4(p0, 0.1, 1.0);
	gl_MeshVerticesEXT[vert_off + 1].gl_Position = vec4(p1, 0.1, 1.0);
	gl_MeshVerticesEXT[vert_off + 2].gl_Position = vec4(p2, 0.1, 1.0);
	gl_MeshVerticesEXT[vert_off + 3].gl_Position = vec4(p3, 0.1, 1.0);

	// Assign the per-vertex output data.
	// For a given glyph quad, these values are constant across all 4 vertices.
	outFontCoord[vert_off + 0] = f0;
	outFontCoord[vert_off + 1] = f1;
	outFontCoord[vert_off + 2] = f2;
	outFontCoord[vert_off + 3] = f3;

	outScale[prim_off + 0]         = pc.scale;
	outStorageOffset[prim_off + 0] = storage_offset;
	outLayerIndex[prim_off + 0]    = layer_index;
	outScale[prim_off + 1]         = pc.scale;
	outStorageOffset[prim_off + 1] = storage_offset;
	outLayerIndex[prim_off + 1]    = layer_index;

	gl_PrimitiveTriangleIndicesEXT[prim_off + 0] = uvec3(vert_off + 0, vert_off + 1, vert_off + 2);
	gl_PrimitiveTriangleIndicesEXT[prim_off + 1] = uvec3(vert_off + 0, vert_off + 2, vert_off + 3);
}
