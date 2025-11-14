#version 450

layout(push_constant) uniform PCBlock
{
	vec2   viewport;
	float  time;
	mat3x3 transform;
	vec4   colors[3];
} pc;

layout(location=0) in vec2 inPos;
layout(location=1) in vec4 inColor;
layout(location=2) in vec2 inUV;

layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outUV;

void main()
{
	vec3 p = pc.transform * vec3(inPos, 1.0);
	p.x /= p.z;
	p.y /= p.z;

	gl_Position = vec4(2.0 * p.xy / pc.viewport - vec2(1.0), 0.0, 1.0);

	outColor = inColor;
	outUV    = inUV;
}