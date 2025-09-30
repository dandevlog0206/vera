#version 450

layout(push_constant) uniform PCBlock
{
	float scale;
	vec4  colors[3];
} pc;

layout(location=0) out vec4 outColor;

vec2 vertices[] = {
	vec2(0, 0.5),
	vec2(-0.5, -0.5),
	vec2(0.5, -0.5)
};

void main()
{
	gl_Position = vec4(pc.scale * vertices[gl_VertexIndex],0.0, 1.0);
	outColor    = pc.colors[gl_VertexIndex];
}
