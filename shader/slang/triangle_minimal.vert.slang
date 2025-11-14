struct Vertex
{
	float2 pos;
	float4 color;
};

static const Vertex vertices[] = 
{
	{ float2(0.0, 0.5), float4(1.0, 0.0, 0.0, 1.0) },
	{ float2(-0.5, -0.5), float4(0.0, 1.0, 0.0, 1.0) },
	{ float2(0.5, -0.5), float4(0.0, 0.0, 1.0, 1.0) }
};

struct VertexOut
{
	float4 position : SV_Position;
	float4 color    : COLOR0;
};

[shader("vertex")]
VertexOut main(uint vertexID : SV_VertexID)
{
	VertexOut output;

	output.position = float4(vertices[vertexID].pos, 0.0, 1.0);
	output.color    = vertices[vertexID].color;
	
	return output;
}