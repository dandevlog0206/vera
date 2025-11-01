#version 450

layout (location=0) out vec2 outUV;

vec2 vertices[] =
{
	vec2(-1.f, +1.f),
	vec2(+1.f, +1.f),
	vec2(+1.f, -1.f),
	vec2(-1.f, +1.f),
	vec2(-1.f, -1.f),
	vec2(+1.f, -1.f),
};

vec2 uvs[] =
{
	vec2(0.f, 1.f),
	vec2(1.f, 1.f),
	vec2(1.f, 0.f),
	vec2(0.f, 1.f),
	vec2(0.f, 0.f),
	vec2(1.f, 0.f),
};

void main()
{
	gl_Position = vec4(vertices[gl_VertexIndex], 0.f, 1.f);
	outUV       = uvs[gl_VertexIndex];
}
