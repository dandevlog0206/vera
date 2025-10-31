#version 450

#define ITERATIONS 200          //Increase for less grainy result
#define TEMPORAL_AA

layout (location=0) in vec2 inUV;
layout (location=0) out vec4 outFragColor;

layout(push_constant) uniform PCBlock
{
	vec2  resolution;
	float time;
	float timeDelta;
	float frameRate;
	int   frame;
	float channelTime[4];
	float channelResolution[4];
	vec2  mouse;
	vec3  position;
	vec3  direction;
	vec3  up;
	float scale;
	vec4  colors[3];
} pc;

void main()
{
}
