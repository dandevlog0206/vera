#version 450

//#define CHEAP_NORMALS
//#define HAS_HEART
#define USE_POST_PROCESSING

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

layout (set=0,binding=0) uniform sampler2D iChannel0;

#define S(a, b, t) smoothstep(a, b, t)

vec3 N13(float p)
{
	vec3 p3 = fract(vec3(p) * vec3(0.1031, 0.11369, 0.13787));
	p3 += dot(p3, p3.yzx + 19.19);
	return fract(vec3((p3.x + p3.y) * p3.z, (p3.x + p3.z) * p3.y, (p3.y + p3.z) * p3.x));
}

vec4 N14(float t)
{
	return fract(sin(t*vec4(123.0, 1024.0, 1456.0, 264.0)) * vec4(6547.0, 345.0, 8799.0, 1564.0));
}

float N(float t)
{
	return fract(sin(12345.564 * t) * 7658.76);
}

float Saw(float b, float t)
{
	return S(0.0, b, t) * S(1.0, b, t);
}

vec2 DropLayer2(vec2 uv, float t)
{
	vec2 UV = uv;

	uv.y += 0.75 * t;
	
	vec2 a         = vec2(6., 1.);
	vec2 grid      = 2.0 * a;
	vec2 id        = floor(uv * grid);
	float colShift = N(id.x); 

	uv.y += colShift;
	id    = floor(uv * grid);

	vec3  n      = N13(35.2 * id.x + 2376.1 * id.y);
	vec2  st     = fract(uv * grid) - vec2(0.5, 0.0);
	float x      = n.x - 0.5;
	float y      = 20.0 * UV.y;
	float wiggle = sin(y + sin(y));
	float ti     = fract(t + n.z);

	x += wiggle * (0.5 - abs(x)) * (n.z - 0.5);
	x *= .7;
	y  = (Saw(.85, ti)-.5)*.9+.5;

	vec2  p          = vec2(x, y);
	float d          = length((st - p) * a.yx);
	float mainDrop   = S(0.4, 0.0, d);
	float r          = sqrt(S(1.0, y, st.y));
	float cd         = abs(st.x - x);
	float trail      = S(0.23 * r, 0.15 * r * r, cd);
	float trailFront = S(-0.02, 0.02, st.y - y);

	trail *= trailFront * r * r;
	y      = UV.y;

	float trail2   = S(0.2 * r, 0.0, cd);
	float droplets = max(0.0, (sin(y * (1.0 - y) * 120.0) - st.y)) * trail2 * trailFront * n.z;

	y = fract(y * 10.0) + (st.y - 0.5);

	float dd = length(st - vec2(x, y));

	droplets = S(.3, 0., dd);

	return vec2(mainDrop + droplets * r * trailFront, trail);
}

float StaticDrops(vec2 uv, float t)
{
	uv *= 40.0;

	vec2 id = floor(uv);

	uv = fract(uv) - 0.5;

	vec3  n    = N13(107.45 * id.x + 3543.654 * id.y);
	vec2  p    = (n.xy - 0.5) * 0.7;
	float d    = length(uv - p);
	float fade = Saw(0.025, fract(t + n.z));

	return S(0.3, 0.0, d) * fract(n.z * 10.) * fade;
}

vec2 Drops(vec2 uv, float t, float l0, float l1, float l2)
{
	float s  = StaticDrops(uv, t) * l0; 
	vec2  m1 = DropLayer2(uv, t) * l1;
	vec2  m2 = DropLayer2(1.85 * uv, t) * l2;
	float c  = s + m1.x + m2.x;

	return vec2(S(0.3, 1.0, c), max(m1.y * l0, m2.y * l1));
}

void main()
{
	vec2  fragCoord = inUV * pc.resolution.xy;
	vec2  uv        = (fragCoord - 0.5 * pc.resolution.xy) / pc.resolution.y;
	vec2  UV        = fragCoord / pc.resolution;
	float aspect    = pc.resolution.x / pc.resolution.y;
	vec3  M         = vec3(pc.mouse, pc.mouse.x) / vec3(pc.resolution, aspect);
	float T         = pc.time + 2.0 * M.x;

	#ifdef HAS_HEART
	T = mod(pc.time, 102.0);
	T = mix(T, 102.0 * M.x, 0 < M.z ? 1.0 : 0.0);
	#endif

	float t          = 0.5 * T;
	float rainAmount = 0.0 < pc.mouse.x ? M.y : 0.3 * sin(0.05 * T) + 0.7;
	float maxBlur    = mix(3.0, 6.0, rainAmount);
	float minBlur    = 2.0;
	float story      = 0.0;
	float heart      = 0.0;

#ifdef HAS_HEART
	story = S(0.0, 70.0, T);

	t = min(1.0, T / 70.0);                              // remap drop time so it goes slower when it freezes
	t = 1.0 - t;
	t = (1.0 - t * t) * 70.0;
	
	float zoom = mix(0.3, 1.2, story);                   // slowly zoom out

	uv     *=zoom;
	minBlur = 4.0 + 3.0 * S(0.5, 1.0, story);            // more opaque glass towards the end
	maxBlur = 6.0 + 1.5 * S(0.5, 1.0, story);

	vec2 hv = vec2(0.5 * uv.x, uv.y) - vec2(0.0, - 0.1); // build heart

	float s = S(110.0, 70.0, T);                         // heart gets smaller and fades towards the end

	hv.y      -= 0.5 * sqrt(abs(hv.x)) * s;
	heart      = length(hv);
	heart      = S(0.4 * s, 0.2 * s, heart) * s;
	rainAmount = heart;                                  // the rain is where the heart is
	maxBlur   -=heart;                                   // inside the heart slighly less foggy
	uv        *= 1.5;                                    // zoom out a bit more
	t         *= 0.25;
#else
	float zoom = -cos(T * 0.2);

	uv *= 0.7 + 0.3 * zoom;
#endif

	UV = (UV - 0.5) * (0.9 + 0.1 * zoom) + 0.5;
	
	float staticDrops = 2.0 * S(-0.5, 1.0, rainAmount);
	float layer1      = S(0.25, 0.75, rainAmount);
	float layer2      = S(0.0, 0.5, rainAmount);
	vec2  c           = Drops(uv, t, staticDrops, layer1, layer2);

#ifdef CHEAP_NORMALS
	vec2  n  = vec2(dFdx(c.x), dFdy(c.x));               // cheap normals (3x cheaper, but 2 times shittier ;))
#else
	vec2  e  = vec2(0.001, 0.0);
	float cx = Drops(uv + e, t, staticDrops, layer1, layer2).x;
	float cy = Drops(uv + e.yx, t, staticDrops, layer1, layer2).x;
	vec2  n  = vec2(cx - c.x, cy - c.x);                 // expensive normals
#endif

#ifdef HAS_HEART
	n   *= 1. - S(60.0, 85.0, T);
	c.y *= 1. - S(80.0, 100.0, T) * 0.8;
#endif

	float focus = mix(maxBlur - c.y, minBlur, S(0.1, 0.2, c.x));
	vec3  col   = textureLod(iChannel0, UV + n, focus).rgb;

#ifdef USE_POST_PROCESSING
	t = 0.5 * (T + 3.0);                                 // make time sync with first lightnoing

	float colFade = sin(t*.2)*.5+.5+story;

	col *= mix(vec3(1.), vec3(.8, .9, 1.3), colFade);    // subtle color shift
	
	float fade      = S(0.0, 5.0, T);                    // fade in at the start
	float lightning = sin(t * sin(10.0 * t));            // lighting flicker
	
	lightning      *= pow(max(0.0, sin(t + sin(t))), 10.0);
	col            *= 1. + lightning * fade*mix(1.0, 0.1, story * story);
	col            *= 1. - dot(UV -= 0.5, UV);

#ifdef HAS_HEART
	col   = mix(pow(col, vec3(1.2)), col, heart);
	fade *= S(102.0, 97.0, T);
#endif
	col *= fade;
#endif

	outFragColor = vec4(col, 1.0);
}
