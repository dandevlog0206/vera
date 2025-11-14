#version 450

#extension GL_ARB_separate_shader_objects : enable

precision mediump float;

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

layout(location=0) in VS_OUT {
	vec2  posMin;
	vec2  posMax;
	vec2  fragMin;
	vec2  fragMax;
	float scale;
	int   offset;
} inGS[];

layout(location=0) out vec2 outFragCoord;
layout(location=1) flat out float outScale;
layout(location=2) flat out int outOffset;

void main()
{
	vec2  p0     = inGS[0].posMin;
	vec2  p1     = vec2(inGS[0].posMax.x, inGS[0].posMin.y);
	vec2  p2     = vec2(inGS[0].posMin.x, inGS[0].posMax.y);
	vec2  p3     = inGS[0].posMax;
	vec2  f0     = inGS[0].fragMin;
	vec2  f1     = vec2(inGS[0].fragMax.x, inGS[0].fragMin.y);
	vec2  f2     = vec2(inGS[0].fragMin.x, inGS[0].fragMax.y);
	vec2  f3     = inGS[0].fragMax;
	float scale  = inGS[0].scale;
	int   offset = inGS[0].offset;

	outScale     = scale;
	outOffset    = offset;
	outFragCoord = f0;
	gl_Position  = vec4(p0, 1.0, 1.0);
	EmitVertex();
	outScale     = scale;
	outOffset    = offset;
	outFragCoord = f1;
	gl_Position  = vec4(p1, 1.0, 1.0);
	EmitVertex();
	outScale     = scale;
	outOffset    = offset;
	outFragCoord = f2;
	gl_Position  = vec4(p2, 1.0, 1.0);
	EmitVertex();
	outScale     = scale;
	outOffset    = offset;
	outFragCoord = f3;
	gl_Position  = vec4(p3, 1.0, 1.0);
	EmitVertex();
	EndPrimitive();
}
