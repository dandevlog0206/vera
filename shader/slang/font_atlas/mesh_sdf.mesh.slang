#define VERTEX_COUNT   4
#define TRIANGLE_COUNT 2
#define LOCAL_SIZE     64

// Vertex structure matching the input buffer
struct Vertex
{
	uint2  position;
	float2 fontCoordMin;
	float2 fontCoordMax;
	uint   storageOffset;
	uint   layerIndex;
};

// Push constant block
struct PushConstantBlock
{
	uint   vertexOffset;
	uint   vertexCount;
	float  scale;
	float  sdfPadding;
	float2 resolution;
};

// Per-vertex output
struct VertexOutput
{
	float4 position : SV_Position;
	float2 fontCoord : TEXCOORD0;
};

// Per-primitive output
struct PrimitiveOutput
{
	float scale : SCALE;
	uint  storageOffset : STORAGE_OFFSET;
	uint  layerIndex : LAYER_INDEX;
};

// Push constants
[[vk::push_constant]]
PushConstantBlock pc;

// Input vertex buffer
[[vk::binding(0, 0)]]
StructuredBuffer<Vertex> vertexBuffer;

[shader("mesh")]
[numthreads(LOCAL_SIZE, 1, 1)]
[outputtopology("triangle")]
void main(
	uint3                                                          groupID : SV_GroupID,
	uint3                                                          localID : SV_GroupThreadID,
	OutputVertices<VertexOutput, VERTEX_COUNT * LOCAL_SIZE>        verts,
	OutputIndices<uint3, TRIANGLE_COUNT * LOCAL_SIZE>              tris,
	OutputPrimitives<PrimitiveOutput, TRIANGLE_COUNT * LOCAL_SIZE> prims
) {
	// Each thread is responsible for one glyph.
	uint vertex_idx = groupID.x * LOCAL_SIZE + localID.x;

	// Bounds check
	if (vertex_idx >= pc.vertexCount) return;

	// The number of vertices and primitives should be set by a single thread in the workgroup.
	// It's often safest to do this with thread 0.
	if (localID.x == 0) {
		uint thread_count = min(pc.vertexCount - groupID.x * LOCAL_SIZE, LOCAL_SIZE);
		SetMeshOutputCounts(thread_count * VERTEX_COUNT, thread_count * TRIANGLE_COUNT);
	}

	// A barrier is required here to ensure SetMeshOutputCounts completes
	// before any thread writes to the output arrays.
	GroupMemoryBarrierWithGroupSync();

	uint2  position       = vertexBuffer[vertex_idx].position;
	float2 coord_min      = vertexBuffer[vertex_idx].fontCoordMin;
	float2 coord_max      = vertexBuffer[vertex_idx].fontCoordMax;
	uint   storage_offset = vertexBuffer[vertex_idx].storageOffset;
	uint   layer_index    = vertexBuffer[vertex_idx].layerIndex;

	float  pad = pc.sdfPadding;
	float2 f0  = float2(coord_min.x - pad, coord_max.y + pad);
	float2 f1  = float2(coord_max.x + pad, coord_max.y + pad);
	float2 f2  = float2(coord_max.x + pad, coord_min.y - pad);
	float2 f3  = float2(coord_min.x - pad, coord_min.y - pad);

	float  width  = 2.0 * (f1.x - f0.x) * pc.scale / pc.resolution.x;
	float  height = 2.0 * (f1.y - f2.y) * pc.scale / pc.resolution.y;
	float2 pos    = 2.0 * float2(position) / pc.resolution - float2(1.0, 1.0);
	float2 p0     = float2(pos.x, -pos.y);
	float2 p1     = p0 + float2(width, 0);
	float2 p2     = p0 + float2(width, -height);
	float2 p3     = p0 + float2(0, -height);

	// Each thread outputs its own vertices and primitives.
	uint vert_off = localID.x * VERTEX_COUNT;
	uint prim_off = localID.x * TRIANGLE_COUNT;

	// Write vertex positions
	verts[vert_off + 0].position = float4(p0, 0.1, 1.0);
	verts[vert_off + 1].position = float4(p1, 0.1, 1.0);
	verts[vert_off + 2].position = float4(p2, 0.1, 1.0);
	verts[vert_off + 3].position = float4(p3, 0.1, 1.0);

	// Write per-vertex font coordinates
	verts[vert_off + 0].fontCoord = f0;
	verts[vert_off + 1].fontCoord = f1;
	verts[vert_off + 2].fontCoord = f2;
	verts[vert_off + 3].fontCoord = f3;

	// Write per-primitive data
	prims[prim_off + 0].scale         = pc.scale;
	prims[prim_off + 0].storageOffset = storage_offset;
	prims[prim_off + 0].layerIndex    = layer_index;
	prims[prim_off + 1].scale         = pc.scale;
	prims[prim_off + 1].storageOffset = storage_offset;
	prims[prim_off + 1].layerIndex    = layer_index;

	// Write triangle indices
	tris[prim_off + 0] = uint3(vert_off + 0, vert_off + 1, vert_off + 2);
	tris[prim_off + 1] = uint3(vert_off + 0, vert_off + 2, vert_off + 3);
}