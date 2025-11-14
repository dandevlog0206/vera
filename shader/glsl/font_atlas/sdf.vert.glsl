#version 450

precision mediump float;

layout(push_constant) uniform PCBlock
{
	vec2  resolution;
	float scale;
} pc;

layout(location=0) in uvec2 inPosMin;
layout(location=1) in uvec2 inPosMax;
layout(location=2) in vec2  inFragMin;
layout(location=3) in vec2  inFragMax;
layout(location=4) in int   inOffset;

layout(location=0) out VS_OUT {
	vec2  posMin;
	vec2  posMax;
	vec2  fragMin;
	vec2  fragMax;
	float scale;
	int   offset;
} outVS;

void main()
{
	gl_Position = vec4(0.0, 0.0, 0.0, 1.0);

	vec2 pos_min = 2.0 * inPosMin / pc.resolution - vec2(1.f);
	vec2 pos_max = 2.0 * inPosMax / pc.resolution - vec2(1.f);

	outVS.posMin  = vec2(pos_min.x, -pos_max.y);
	outVS.posMax  = vec2(pos_max.x, -pos_min.y);
	outVS.fragMin = vec2(inFragMin);
	outVS.fragMax = vec2(inFragMax);
	outVS.scale   = pc.scale;
	outVS.offset  = inOffset;
}
