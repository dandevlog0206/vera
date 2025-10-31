#version 450

layout(push_constant) uniform PCBlock
{
	float scale;
	float aspect;
	float attr;
} pc;

layout(location=0) in vec2  inPos;
layout(location=1) in vec4  inColor;
layout(location=2) in float inAttr;

layout(location=0) out vec2  outUV;
layout(location=1) out vec4  outColor;
layout(location=2) out float outAttr;

const vec2 UVs[] = {
	vec2(0.0, 0.0),
	vec2(0.5, 0.0),
	vec2(1.0, 1.0),
};

void main()
{
	gl_Position = vec4(vec2(1.0 / pc.aspect, 1.0) * pc.scale * inPos, 0.0, 1.0);
	outUV       = UVs[gl_VertexIndex % 3];
	outColor    = inColor;
	outAttr     = inAttr;
}
