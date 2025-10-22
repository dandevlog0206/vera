//#version 450
//
//layout(location=0) out vec4 outColor;
//
//struct Vertex
//{
//	vec2 pos;
//	vec4 color;
//};
//
//Vertex vertices[] = {
//	Vertex(vec2(0, 0.5), vec4(1.0, 0.0, 0.0, 1.0)),
//	Vertex(vec2(-0.5, -0.5), vec4(0.0 , 1.0, 0.0, 1.0)),
//	Vertex(vec2(0.5, -0.5), vec4(0.0, 0.0, 1.0, 1.0))
//};
//
//void main()
//{
//	gl_Position = vec4(vertices[gl_VertexIndex].pos,0.0, 1.0);
//	outColor    = vertices[gl_VertexIndex].color;
//}
//

#version 450

layout(location=0) out vec2 outUV;
layout(location=1) out vec4 outColor;

struct Vertex
{
	vec2 pos;
	vec2 uv;
	vec4 color;
};

Vertex vertices[] = {
	Vertex(vec2(0, 0.5), vec2(0.5, 0.0), vec4(1.0, 0.0, 0.0, 1.0)),
	Vertex(vec2(-0.5, -0.5), vec2(0.0, 0.0), vec4(0.0 , 1.0, 0.0, 1.0)),
	Vertex(vec2(0.5, -0.5), vec2(1.0, 1.0), vec4(0.0, 0.0, 1.0, 1.0))
};

void main()
{
	gl_Position = vec4(vertices[gl_VertexIndex].pos,0.0, 1.0);
	outUV       = vertices[gl_VertexIndex].uv;
	outColor    = vertices[gl_VertexIndex].color;
}
