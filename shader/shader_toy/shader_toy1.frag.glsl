#version 450

// #define ANTI_ALIASING
#define USE_CAMERA

const int   NUM_STEPS = 32;
const float PI        = 3.141592;
const float TWO_PI    = 6.283185;
const float EPSILON   = 1e-4;

const int   ITER_GEOMETRY   = 3;
const int   ITER_FRAGMENT   = 5;
const float SEA_HEIGHT      = 0.6;
const float SEA_CHOPPY      = 4.0;
const float SEA_SPEED       = 0.8;
const float SEA_FREQ        = 0.16;
const vec3  SEA_BASE        = vec3(0.0, 0.09, 0.18);
const vec3  SEA_WATER_COLOR = 0.6 * vec3(0.8 ,0.9 ,0.6);
const mat2  octave_m        = mat2(1.6, 1.2, -1.2, 1.6);

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

#define SEA_TIME (1.0 + pc.time * SEA_SPEED)
#define EPSILON_NRM (0.1 / pc.resolution.x)

mat3 fromEuler(vec3 rad)
{
	vec2 a1 = vec2(sin(rad.x),cos(rad.x));
	vec2 a2 = vec2(sin(rad.y),cos(rad.y));
	vec2 a3 = vec2(sin(rad.z),cos(rad.z));
	mat3 m;
	
	m[0] = vec3(
		a1.y * a3.y + a1.x * a2.x * a3.x,
		a1.y * a2.x * a3.x + a3.y * a1.x,
		-a2.y * a3.x);
	
	m[1] = vec3(
		-a2.y * a1.x,
		a1.y * a2.y,
		a2.x);

	m[2] = vec3(
		a3.y * a1.x * a2.x + a1.y * a3.x,
		a1.x * a3.x - a1.y * a3.y * a2.x,
		a2.y * a3.y);

	return m;
}

float hash(vec2 p)
{
	vec3 p3  = fract(vec3(p.xyx) * .1031);
	p3 += dot(p3, p3.yzx + 33.33);
	return fract((p3.x + p3.y) * p3.z);
}

float noise(vec2 p)
{
	vec2 i = floor(p);
	vec2 f = fract(p);

	vec2 u = f * f * (3.0 - 2.0 * f);

	float n0 = mix(hash(i + vec2(0.0, 0.0)), hash(i + vec2(1.0, 0.0)), u.x);
	float n1 = mix(hash(i + vec2(0.0, 1.0)), hash(i + vec2(1.0, 1.0)), u.x);

	return 2.0 * mix(n0, n1, u.y) - 1.0;
}

float diffuse(vec3 n, vec3 l, float p)
{
	return pow(dot(n, l) * 0.4 + 0.6, p);
}

float specular(vec3 n, vec3 l, vec3 e, float s)
{ 
	float nrm = (s + 8.0) / (PI * 8.0);
	return pow(max(dot(reflect(e, n), l), 0.0), s) * nrm;
}

float seaOctave(vec2 uv, float choppy)
{
	uv += noise(uv);
	
	vec2 wv  = 1.0 - abs(sin(uv));
	vec2 swv = abs(cos(uv));    
	
	wv = mix(wv, swv, wv);

	return pow(1.0 - pow(wv.x * wv.y, 0.65), choppy);
}

float map(vec3 p)
{
	float d;
	float h      = 0.0;
	float freq   = SEA_FREQ;
	float amp    = SEA_HEIGHT;
	float choppy = SEA_CHOPPY;
	vec2  uv     = p.xz;
	uv.x *= 0.75;

	for (int i = 0; i < ITER_GEOMETRY; i++) {
		d     = seaOctave(freq * (uv + SEA_TIME), choppy);
		d    += seaOctave(freq * (uv - SEA_TIME), choppy);
		h    += d * amp;
		uv   *= octave_m;
		freq *= 1.9;
		amp  *= 0.22;

		choppy = mix(choppy, 1.0, 0.2);
	}

	return p.y - h;
}

float mapDetailed(vec3 p)
{
	float d;
	float h      = 0.0;
	float freq   = SEA_FREQ;
	float amp    = SEA_HEIGHT;
	float choppy = SEA_CHOPPY;
	vec2  uv     = p.xz;
	uv.x *= 0.75;

	for (int i = 0; i < ITER_FRAGMENT; i++) {
		d     = seaOctave(freq * (uv + SEA_TIME), choppy);
		d    += seaOctave(freq * (uv - SEA_TIME), choppy);
		h    += d * amp;
		uv   *= octave_m;
		freq *= 1.9;
		amp  *= 0.22;

		choppy = mix(choppy, 1.0, 0.2);
	}

	return p.y - h;
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

	color += SEA_WATER_COLOR * (p.y - SEA_HEIGHT) * 0.18 * atten;
	color += specular(n, l, eye, 600.0 * inversesqrt(dot(dist, dist)));
	
	return color;
}

vec3 getNormal(vec3 p, float eps)
{
	vec3 n;

	n.y = mapDetailed(p);
	n.x = mapDetailed(vec3(p.x + eps, p.y, p.z)) - n.y;
	n.z = mapDetailed(vec3(p.x, p.y, p.z + eps)) - n.y;
	n.y = eps;

	return normalize(n);
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

vec3 getPixel(vec2 uv, float time)
{
	uv    = 2.0 * uv - 1.0;
	uv.x *= pc.resolution.x / pc.resolution.y;
	
	vec3 p;
#ifdef USE_CAMERA
	vec3 origin = pc.position;
	vec3 dir    = pc.direction;
	vec3 right  = normalize(cross(vec3(0.0, 1.0, 0.0), dir));
	vec3 up     = normalize(cross(dir, right));

	dir = normalize(uv.x * right + uv.y * up + 1.5 * dir);
#else
	vec3 angle  = vec3(0.1 * sin(3.0 * time), 0.2 * sin(time) + 0.3, time);
	vec3 origin = vec3(0.0, 3.5, 5.0 * time);
	vec3 dir    = normalize(vec3(uv, -1.5));
	
	dir.z += 0.14 * length(uv);
	dir    = normalize(dir) * fromEuler(angle);
#endif

	heightMapTracing(origin, dir, p);

	vec3 dist  = p - origin;
	vec3 n     = getNormal(p, dot(dist, dist) * EPSILON_NRM);
	vec3 light = normalize(vec3(0.0, 0.5, 0.8)); 

	return mix(
		getSkyColor(dir, light),
		getSeaColor(p, n, light, dir, dist),
		pow(smoothstep(0.0, -0.02, dir.y), 0.2));
}

void main()
{
	float time = 0.3 * pc.time + 0.01 * pc.mouse.x;

#ifdef ANTI_ALIASING
	vec3 color = vec3(0.0);

	for (int i = -1; i <= 1; i++)
		for (int j = -1; j <= 1; j++)
			color += getPixel(inUV + vec2(i, j) / pc.resolution / 3.0, time);

	color /= 9.0;
#else
	vec3 color = getPixel(inUV, time);
#endif

	outFragColor = vec4(pow(color, vec3(0.65)), 1.0);
}
