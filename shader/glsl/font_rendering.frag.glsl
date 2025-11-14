#version 450

precision highp float;

const int   NUM_STEPS = 32;
const float PI        = 3.141592;
const float TWO_PI    = 6.283185;
const float EPSILON   = 1e-4;

const float SEA_HEIGHT      = 7.0;
const vec3  SEA_BASE        = vec3(0.09);
const vec3  SEA_WATER_COLOR = vec3(0.9);

#define EPSILON_NRM (0.1 / pc.resolution.x)

layout (location=0) in vec2 inUV;
layout (location=0) out vec4 outFragColor;

layout(push_constant) uniform PCBlock
{
	vec2  resolution;
	vec3  position;
	vec3  direction;
	vec3  up;
} pc;

layout(set=0, binding=0) uniform sampler2D atlasTexture;

float diffuse(vec3 n, vec3 l, float p)
{
	return pow(dot(n, l) * 0.4 + 0.6, p);
}

float specular(vec3 n, vec3 l, vec3 e, float s)
{ 
	float nrm = (s + 8.0) / (PI * 8.0);
	return pow(max(dot(reflect(e, n), l), 0.0), s) * nrm;
}

float map(vec3 p)
{
	p.x /= 4096.0;
	p.z /= 4096.0;

	float height = texture(atlasTexture, p.xz).r + 50.0;

	return p.y - height;
}

float heightMapTracing(vec3 origin, vec3 dir, out vec3 p)
{
	float tm = 0.0;
	float tx = 1000.0;
	float hx = map(origin + dir * tx);

	if (hx > 0.0) {
		p = origin + dir * tx;
		return tx;
	}

	float hm = map(origin);

	for (int i = 0; i < NUM_STEPS; i++) {
		float tmid = mix(tm, tx, hm / (hm - hx));

		p = origin + dir * tmid;

		float hmid = map(p);
		
		if (hmid < 0.0) {
			tx = tmid;
			hx = hmid;
		} else {
			tm = tmid;
			hm = hmid;
		}

		if (abs(hmid) < EPSILON) break;
	}

	return mix(tm, tx, hm / (hm - hx));
}

vec3 getNormal(vec3 p, float eps)
{
	vec3 n;

	n.y = map(p);
	n.x = map(vec3(p.x + eps, p.y, p.z)) - n.y;
	n.z = map(vec3(p.x, p.y, p.z + eps)) - n.y;
	n.y = eps;

	return normalize(n);
}

vec3 sunWithBloom(vec3 rayDir, vec3 sunDir) {
	const float sunSolidAngle  = 2.0 * PI / 180.0;
	const float minSunCosTheta = cos(sunSolidAngle);
	const float cosTheta       = dot(rayDir, sunDir);

	if (cosTheta >= minSunCosTheta)
		return vec3(1.0);
	
	float offset        = minSunCosTheta - cosTheta;
	float gaussianBloom = 0.5 * exp(-50000.0 * offset);
	float invBloom      = 0.01 / (0.02 + 300.0 * offset);

	return (gaussianBloom + invBloom) * vec3(0.9, 0.8, 0.3);
}

vec3 getSkyColor(vec3 e, vec3 light)
{
	vec3 sunColor = vec3(0.0);

	if (cos(8.0 * PI / 180.0) < dot(e, light))
		sunColor = sunWithBloom(e, light);

	e.y = (max(e.y, 0.0) * 0.8 + 0.2) * 0.8;
	return sunColor + vec3(pow(1.0 - e.y, 2.0), 1.0 - e.y, 0.6 + (1.0 - e.y) * 0.4) * 1.1;
}

vec3 getSeaColor(vec3 p, vec3 n, vec3 l, vec3 eye, vec3 dist)
{  
	float fresnel = clamp(1.0 - dot(n, -eye), 0.0, 1.0);

	fresnel = min(fresnel * fresnel * fresnel, 0.5);
	
	vec3  reflected = getSkyColor(reflect(eye, n), l);
	vec3  refracted = SEA_BASE + diffuse(n, l, 80.0) * SEA_WATER_COLOR * 0.12;
	vec3  color     = mix(refracted, reflected, fresnel);
	float atten     = max(1.0 - dot(dist, dist) * 0.001, 0.0);

	// color += SEA_WATER_COLOR * (p.y - SEA_HEIGHT) * 0.18 * atten;
	color += specular(n, l, eye, 600.0 * inversesqrt(dot(dist, dist)));
	
	return color;
}

void main()
{
	vec2 uv = 2.0 * inUV - 1.0;

	uv.x *= pc.resolution.x / pc.resolution.y;
	
	vec3 p;
	vec3 origin = pc.position;
	vec3 dir    = pc.direction;
	vec3 right  = normalize(cross(vec3(0.0, 1.0, 0.0), dir));
	vec3 up     = normalize(cross(dir, right));

	dir = normalize(uv.x * right + uv.y * up + 1.5 * dir);

	heightMapTracing(origin, dir, p);

	vec3 dist  = p - origin;
	vec3 n     = getNormal(p, dot(dist, dist) * EPSILON_NRM);
	vec3 light = normalize(vec3(0.0, 0.5, 0.8)); 

	vec3 color = mix(
		getSkyColor(dir, light),
		getSeaColor(p, n, light, dir, dist),
		pow(smoothstep(0.0, -0.02, dir.y), 0.2));

	outFragColor = vec4(pow(color, vec3(0.65)), 1.0);
}
