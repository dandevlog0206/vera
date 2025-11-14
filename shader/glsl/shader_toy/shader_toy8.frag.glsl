#version 450

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
	float a  = pc.resolution.x / pc.resolution.y;
	float pz = 0.8 * cos(0.7 * pc.time) - 1.7;
	float z  = 0.0;
	float d  = 0.0;
	float i  = 0.0;
	vec3  O  = vec3(0.0);

	for(; i++ < 20.0;) {
		//Sample point (from ray direction)
		vec3 p = z * normalize(vec3(2.0 * vec2(a * inUV.x, inUV.y) - vec2(a, 1.0), pz));

		//Polar coordinates and additional transformations
		p = vec3(2.0 * atan(p.y / 0.2, p.x), p.z / 3.0, length(p.xy) - 5.0 - 0.2 * z);

		//Apply turbulence and refraction effect
		for(d = 0.0; d++ < 7.0;)
			p += sin(p.yzx * d + pc.time + 0.3 * i) / d;

		//Distance to cylinder and waves with refraction
		z += d = length(vec4(0.4 * cos(p) - 0.4, p.z));

		//Coloring and brightness
		O += (1.0 + cos(p.x + 0.4 * i + z + vec3(6.0, 1.0, 2.0))) / d;
	}

	//Tanh tonemap
	outFragColor = vec4(tanh(O * O / 4e2), 1.0);
}
