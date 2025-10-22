#version 450

// Using iq's intersectors: https://iquilezles.org/articles/intersectors
// Using https://www.shadertoy.com/view/ltKBzG
// Using https://www.shadertoy.com/view/tsVXzh
// Using https://www.shadertoy.com/view/WlffDn
// Using https://www.shadertoy.com/view/WslGz4

// This shadertoy use ALPHA, NO_ALPHA set alpha to 1, BG_ALPHA set background as alpha
// iChannel0 used as background if alpha ignored by wallpaper-app
//#define NO_ALPHA
//#define BG_ALPHA
//#define SHADOW_ALPHA
//#define ONLY_BOX

// Save PERFORMANCE by disabling shadow
//#define NO_SHADOW

// Speed of ROTATION
#define ROTATION_SPEED 0.8999

// Static SHAPE form, default 0.5
//#define STATIC_SHAPE 0.15

// ANIMATION shape change
// #define ANIM_SHAPE

// ANIMATION color change
#define ANIM_COLOR

// Custom COLOR, and change those const values
//#define USE_COLOR
const vec3 color_blue = vec3(0.5, 0.65, 0.8);
const vec3 color_red  = vec3(0.99, 0.2, 0.1);

// Use 4xAA for cube only (set 2-4-etc level of AA)
//#define AA_CUBE 4

// Use 4xAA for everything - derivative filtering will not be used, look fcos2
// This is very slow - DO NOT USE
//#define AA_ALL 4

#define tshift 53.

// Reflect back side
//#define backside_refl

// Camera with mouse
#define MOUSE_control

// min(iFrame,0) does not speedup compilation in ANGLE
#define ANGLE_loops 0

// This shader discover Nvidia bug with arrays https://www.shadertoy.com/view/NslGR4
// Use DEBUG with BUG, BUG trigger that bug and one layer will be white on Nvidia in OpenGL
//#define DEBUG
//#define BUG

#define FDIST 0.7
#define PI 3.1415926
#define GROUNDSPACING 0.5
#define GROUNDGRID 0.05
#define BOXDIMS vec3(0.75, 0.75, 1.25)
#define IOR 1.33

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outFragColor;

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

void swapInt(inout int a, inout int b)
{
	int t = a;
	a     = b;
	b     = t;
}

void swapVec3(inout vec3 a, inout vec3 b)
{
	vec3 t = a;
	a      = b;
	b      = t;
}

mat3 rotx(float a)
{
	float s = sin(a);
	float c = cos(a);

	mat3  m;
	m[0] = vec3(1.0, 0.0, 0.0);
	m[1] = vec3(0.0, c, s);
	m[2] = vec3(0.0, -s, c);

	return m;
}

mat3 roty(float a)
{
	float s = sin(a);
	float c = cos(a);

	mat3 m;
	m[0] = vec3(c, 0.0, s);
	m[1] = vec3(0.0, 1.0, 0.0);
	m[2] = vec3(-s, 0.0, c);

	return m;
}

mat3 rotz(float a)
{
	float s = sin(a);
	float c = cos(a);

	mat3 m;
	m[0] = vec3(c, s, 0.0);
	m[1] = vec3(-s, c, 0.0);
	m[2] = vec3(0.0, 0.0, 1.0);

	return m;
}

vec3 fcos1(vec3 x)
{
	vec3  w  = fwidth(x);
	float lw = length(w);

	if ((lw == 0.0) || isnan(lw) || isinf(lw)) {
		vec3 tc = vec3(0.0);

		for (int i = 0; i < 8; i++)
			tc += cos(x + x * float(i - 4) * (0.01 * 400.0 / pc.resolution.y));

		return tc / 8.0;
	}

	return cos(x) * smoothstep(3.14 * 2.0, 0.0, w);
}

vec3 fcos2(vec3 x)
{
	return cos(x);
}

vec3 fcos(vec3 x)
{
#ifdef AA_ALL
	return fcos2(x);
#else
	return fcos1(x);
#endif
}

vec3 getColor(vec3 p)
{
	p  = 1.25 * abs(p);
	p  = 0.5 * p / dot(p, p);
#ifdef ANIM_COLOR
	p += 0.072 * pc.time;
#endif

	float t   = (0.13) * length(p);
	vec3  col = vec3(0.3, 0.4, 0.5);

	col += 0.12 * fcos(6.28318 * t * 1.0 + vec3(0.0, 0.8, 1.1));
	col += 0.11 * fcos(6.28318 * t * 3.1 + vec3(0.3, 0.4, 0.1));
	col += 0.10 * fcos(6.28318 * t * 5.1 + vec3(0.1, 0.7, 1.1));
	col += 0.10 * fcos(6.28318 * t * 17.1 + vec3(0.2, 0.6, 0.7));
	col += 0.10 * fcos(6.28318 * t * 31.1 + vec3(0.1, 0.6, 0.7));
	col += 0.10 * fcos(6.28318 * t * 65.1 + vec3(0.0, 0.5, 0.8));
	col += 0.10 * fcos(6.28318 * t * 115.1 + vec3(0.1, 0.4, 0.7));
	col += 0.10 * fcos(6.28318 * t * 265.1 + vec3(1.1, 1.4, 2.7));
	col  = clamp(col, 0.0, 1.0);

	return col;
}

void calcColor(vec3 ro, vec3 rd, vec3 nor, float d, float len, int idx, bool si, float td, out vec4 colx,
			   out vec4 colsi)
{
	vec3  pos = (ro + rd * d);
#ifdef DEBUG
	float a   = 1.0 - smoothstep(len - 0.15, len + 0.00001, length(pos));

	if (idx == 0)
		colx = vec4(1.0, 0.0, 0.0, a);
	if (idx == 1)
		colx = vec4(0.0, 1.0, 0.0, a);
	if (idx == 2)
		colx = vec4(0.0, 0.0, 1.0, a);

	if (si) {
		pos = (ro + rd * td);

		float ta = 1.0 - smoothstep(len - 0.15, len + 0.00001, length(pos));

		if (idx == 0)
			colsi = vec4(1.0, 0.0, 0.0, ta);
		if (idx == 1)
			colsi = vec4(0.0, 1.0, 0.0, ta);
		if (idx == 2)
			colsi = vec4(0.0, 0.0, 1.0, ta);
	}
#else
	float a   = 1.0 - smoothstep(len - 0.15 * 0.5, len + 0.00001, length(pos));
	vec3  col = getColor(pos);
	
	colx = vec4(col, a);
	
	if (si) {
		pos = (ro + rd * td);

		float ta = 1.0 - smoothstep(len - 0.15 * 0.5, len + 0.00001, length(pos));
		
		col   = getColor(pos);
		colsi = vec4(col, ta);
	}
#endif
}

bool iBilinearPatch(in vec3 ro, in vec3 rd, in vec4 ps, in vec4 ph, in float sz, out float t, out vec3 norm,
					out bool si, out float tsi, out vec3 normsi, out float fade, out float fadesi)
{
	vec3 va = vec3(0.0, 0.0, ph.x + ph.w - ph.y - ph.z);
	vec3 vb = vec3(0.0, ps.w - ps.y, ph.z - ph.x);
	vec3 vc = vec3(ps.z - ps.x, 0.0, ph.y - ph.x);
	vec3 vd = vec3(ps.xy, ph.x);

	t      = -1.0;
	tsi    = -1.0;
	si     = false;
	fade   = 1.0;
	fadesi = 1.0;
	norm   = vec3(0.0, 1.0, 0.0);
	normsi = vec3(0.0, 1.0, 0.0);

	float tmp = 1.0 / (vb.y * vc.x);
	float a   = 0.0;
	float b   = 0.0;
	float c   = 0.0;
	float d   = va.z * tmp;
	float e   = 0.0;
	float f   = 0.0;
	float g   = (vc.z * vb.y - vd.y * va.z) * tmp;
	float h   = (vb.z * vc.x - va.z * vd.x) * tmp;
	float i   = -1.0;
	float j   = (vd.x * vd.y * va.z + vd.z * vb.y * vc.x) * tmp - (vd.y * vb.z * vc.x + vd.x * vc.z * vb.y) * tmp;
	float p   = dot(vec3(a, b, c), rd.xzy * rd.xzy) + dot(vec3(d, e, f), rd.xzy * rd.zyx);
	float q   = dot(vec3(2.0, 2.0, 2.0) * ro.xzy * rd.xyz, vec3(a, b, c)) + dot(ro.xzz * rd.zxy, vec3(d, d, e)) +
			  dot(ro.yyx * rd.zxy, vec3(e, f, f)) + dot(vec3(g, h, i), rd.xzy);
	float r =
		dot(vec3(a, b, c), ro.xzy * ro.xzy) + dot(vec3(d, e, f), ro.xzy * ro.zyx) + dot(vec3(g, h, i), ro.xzy) + j;

	if (abs(p) < 0.000001) {
		float tt = -r / q;

		if (tt <= 0.0)
			return false;
		
		t = tt;

		vec3 pos = ro + t * rd;

		if (length(pos) > sz)
			return false;

		vec3 grad =
			vec3(2.0) * pos.xzy * vec3(a, b, c) + pos.zxz * vec3(d, d, e) + pos.yyx * vec3(f, e, f) + vec3(g, h, i);

		norm = -normalize(grad);

		return true;
	} else {
		float sq = q * q - 4.0 * p * r;
		if (sq < 0.0) {
			return false;
		} else {
			float s   = sqrt(sq);
			float t0  = (-q + s) / (2.0 * p);
			float t1  = (-q - s) / (2.0 * p);
			float tt1 = min(t0 < 0.0 ? t1 : t0, t1 < 0.0 ? t0 : t1);
			float tt2 = max(t0 > 0.0 ? t1 : t0, t1 > 0.0 ? t0 : t1);
			float tt0 = tt1;

			if (tt0 <= 0.0)
				return false;

			vec3 pos = ro + tt0 * rd;
			bool ru  = 0.5 < step(sz, length(pos));

			if (ru) {
				tt0 = tt2;
				pos = ro + tt0 * rd;
			}

			if (tt0 <= 0.0)
				return false;
			
			if (0.5 < step(sz, length(pos)))
				return false;

			if (0.0 < tt2 && !ru && step(sz, length(ro + tt2 * rd)) <= 0.5) {
				si     = true;
				fadesi = s;
				tsi    = tt2;

				vec3 tpos  = ro + tsi * rd;
				vec3 tgrad = vec3(2.0) * tpos.xzy * vec3(a, b, c) + tpos.zxz * vec3(d, d, e) +
							 tpos.yyx * vec3(f, e, f) + vec3(g, h, i);

				normsi = -normalize(tgrad);
			}

			fade = s;
			t    = tt0;

			vec3 grad =
				2.0 * pos.xzy * vec3(a, b, c) + pos.zxz * vec3(d, d, e) + pos.yyx * vec3(f, e, f) + vec3(g, h, i);

			norm = -normalize(grad);

			return true;
		}
	}
}

float dot2(in vec3 v)
{
	return dot(v, v);
}

float segShadow(in vec3 ro, in vec3 rd, in vec3 pa, float sh)
{
	float dm = dot(rd.yz, rd.yz);
	float k1 = (ro.x - pa.x) * dm;
	float k2 = (ro.x + pa.x) * dm;
	vec2  k5 = (ro.yz + pa.yz) * dm;
	float k3 = dot(ro.yz + pa.yz, rd.yz);
	vec2  k4 = (pa.yz + pa.yz) * rd.yz;
	vec2  k6 = (pa.yz + pa.yz) * dm;

	for (int i = 0; i < 4 + ANGLE_loops; i++) {
		vec2 s = vec2(i & 1, i >> 1);
		float t = dot(s, k4) - k3;

		if (0.0 < t)
			sh = min(sh, dot2(vec3(clamp(-rd.x * t, k1, k2), k5 - k6 * s) + rd * t) / (t * t));
	}

	return sh;
}

float boxSoftShadow(in vec3 ro, in vec3 rd, in vec3 rad, in float sk)
{
	rd += 0.0001 * (1.0 - abs(sign(rd)));

	vec3  rdd = rd;
	vec3  roo = ro;
	vec3  m   = 1.0 / rdd;
	vec3  n   = m * roo;
	vec3  k   = abs(m) * rad;
	vec3  t1  = -n - k;
	vec3  t2  = -n + k;
	float tN  = max(max(t1.x, t1.y), t1.z);
	float tF  = min(min(t2.x, t2.y), t2.z);

	if (tN < tF && tF > 0.0)
		return 0.0;

	float sh = 1.0;

	sh = segShadow(roo.xyz, rdd.xyz, rad.xyz, sh);
	sh = segShadow(roo.yzx, rdd.yzx, rad.yzx, sh);
	sh = segShadow(roo.zxy, rdd.zxy, rad.zxy, sh);
	sh = clamp(sk * sqrt(sh), 0.0, 1.0);

	return sh * sh * (3.0 - 2.0 * sh);
}

float box(in vec3 ro, in vec3 rd, in vec3 r, out vec3 nn, bool entering)
{
	rd += 0.0001 * (1.0 - abs(sign(rd)));

	vec3  dr   = 1.0 / rd;
	vec3  n    = ro * dr;
	vec3  k    = r * abs(dr);
	vec3  pin  = -k - n;
	vec3  pout = k - n;
	float tin  = max(pin.x, max(pin.y, pin.z));
	float tout = min(pout.x, min(pout.y, pout.z));

	if (tin > tout)
		return -1.0;

	if (entering) {
		nn = -sign(rd) * step(pin.zxy, pin.xyz) * step(pin.yzx, pin.xyz);
		return tin;
	} else {
		nn = sign(rd) * step(pout.xyz, pout.zxy) * step(pout.xyz, pout.yzx);
		return tout;
	}
}

vec3 bgcol(in vec3 rd)
{
	return mix(vec3(0.01), vec3(0.336, 0.458, .668), 1.0 - pow(abs(rd.z + 0.25), 1.3));
}

vec3 background(in vec3 ro, in vec3 rd, vec3 l_dir, out float alpha)
{
	alpha   = 0.0;
#ifdef ONLY_BOX
	return vec3(0.01);
#endif
	float t = (-BOXDIMS.z - ro.z) / rd.z;
	vec3 bgc = bgcol(rd);

	if (t < 0.0)
		return bgc;

	vec2  uv   = ro.xy + t * rd.xy;
#ifdef NO_SHADOW
	float shad = 1.0;
#else
	float shad = boxSoftShadow((ro + t * rd), normalize(l_dir + vec3(0.0, 0.0, 1.0)) * rotz(PI * 0.65), BOXDIMS, 1.5);
#endif
	float aofac = smoothstep(-0.95, 0.75, length(abs(uv) - min(abs(uv), vec2(0.45))));
	float lght  = max(dot(normalize(ro + t * rd + vec3(0.0, -0.0, -5.0)),
						  normalize(l_dir - vec3(0.0, 0.0, 1.0)) * rotz(PI * 0.65)),
					  0.0);

	aofac = min(aofac, smoothstep(-0.65, 1.0, shad));

	vec3 col = mix(vec3(0.4), vec3(.71, .772, 0.895), lght * lght * aofac + 0.05) * aofac;

	alpha    = 1.0 - smoothstep(7.0, 10.0, length(uv));

#ifdef SHADOW_ALPHA
	// alpha=clamp(alpha*max(lght*lght*0.95,(1.-aofac)*1.25),0.0,1.0);
	alpha = clamp(alpha * (1.0 - aofac) * 1.25, 0.0, 1.0);
#endif

	return mix(col * length(col) * 0.8, bgc, smoothstep(7.0, 10.0, length(uv)));
}

vec4 insides(vec3 ro, vec3 rd, vec3 nor_c, vec3 l_dir, out float tout)
{
	tout = -1.0;

	vec3  trd = rd;
	vec3  col = vec3(0.0);

	if (0.5 < abs(nor_c.x)) {
		rd = rd.xzy * nor_c.x;
		ro = ro.xzy * nor_c.x;
	} else if (0.5 < abs(nor_c.z)) {
		l_dir *= roty(PI);
		rd     = rd.yxz * nor_c.z;
		ro     = ro.yxz * nor_c.z;
	} else if (0.5 < abs(nor_c.y)) {
		l_dir *= rotz(-PI * 0.5);
		rd     = rd * nor_c.y;
		ro     = ro * nor_c.y;
	}

#ifdef ANIM_SHAPE
	float curvature =
		(0.001 + 1.5 - 1.5 * smoothstep(0.0, 8.5, mod((pc.time + tshift) * 0.44, 20.0)) *
						   (1.0 - smoothstep(10.0, 18.5, mod((pc.time + tshift) * 0.44, 20.0))));
	// curvature(to not const above) make compilation on Angle 15+ sec
#else
#ifdef STATIC_SHAPE
	const float curvature = STATIC_SHAPE;
#else
	const float curvature = .5;
#endif
#endif
	float bil_size  = 1.0;
	vec4  ps        = vec4(-bil_size, -bil_size, bil_size, bil_size) * curvature;
	vec4  ph        = vec4(-bil_size, bil_size, bil_size, -bil_size) * curvature;
	vec4  colx[3]   = { vec4(0.0), vec4(0.0), vec4(0.0) };
	vec3  dx[3]     = { vec3(-1.0), vec3(-1.0), vec3(-1.0) };
	vec4  colxsi[3] = { vec4(0.0), vec4(0.0), vec4(0.0) };
	int   order[3]  = { 0, 1, 2 };

	for (int i = 0; i < 3 + ANGLE_loops; i++) {
		if (0.5 < abs(nor_c.x)) {
			ro *= rotz(-PI * (1.0 / float(3)));
			rd *= rotz(-PI * (1.0 / float(3)));
		} else if (0.5 < abs(nor_c.z)) {
			ro *= rotz(PI * (1.0 / float(3)));
			rd *= rotz(PI * (1.0 / float(3)));
		} else if (0.5 < abs(nor_c.y)) {
			ro *= rotx(PI * (1.0 / float(3)));
			rd *= rotx(PI * (1.0 / float(3)));
		}

		vec3  normnew;
		float tnew;
		bool  si;
		float tsi;
		vec3  normsi;
		float fade;
		float fadesi;

		if (iBilinearPatch(ro, rd, ps, ph, bil_size, tnew, normnew, si, tsi, normsi, fade, fadesi)) {
			if (0.0 < tnew) {
				vec4 tcol;
				vec4 tcolsi;

				calcColor(ro, rd, normnew, tnew, bil_size, i, si, tsi, tcol, tcolsi);

				if (0.0 < tcol.a) {
					dx[i]   = vec3(tnew, float(si), tsi);
#ifdef DEBUG
					colx[i] = tcol;

					if (si)
						colxsi[i] = tcolsi;
#else

					float dif = clamp(dot(normnew, l_dir), 0.0, 1.0);
					float amb = clamp(0.5 + 0.5 * dot(normnew, l_dir), 0.0, 1.0);

					{
#ifdef USE_COLOR
						vec3 shad = 0.57 * color_blue * amb + 1.5 * color_blue.bgr * dif;
						const vec3 tcr = color_red;
#else
						vec3 shad = vec3(0.32, 0.43, 0.54) * amb + vec3(1.0, 0.9, 0.7) * dif;
						const vec3 tcr = vec3(1.0, 0.21, 0.11);
#endif
						float ta = clamp(length(tcol.rgb), 0.0, 1.0);
						tcol     = clamp(tcol * tcol * 2.0, 0.0, 1.0);
						vec4 tvalx =
							vec4((tcol.rgb * shad * 1.4 + 3.0 * (tcr * tcol.rgb) * clamp(1.0 - (amb + dif), 0.0, 1.0)),
								 min(tcol.a, ta));
						tvalx.rgb = clamp(2.0 * tvalx.rgb * tvalx.rgb, 0.0, 1.0);
						tvalx *= (min(fade * 5.0, 1.0));
						colx[i] = tvalx;
					}

					if (si) {
						dif = clamp(dot(normsi, l_dir), 0.0, 1.0);
						amb = clamp(0.5 + 0.5 * dot(normsi, l_dir), 0.0, 1.0);

						{
#ifdef USE_COLOR
							vec3       shad = 0.57 * color_blue * amb + 1.5 * color_blue.bgr * dif;
							const vec3 tcr  = color_red;
#else
							vec3       shad = vec3(0.32, 0.43, 0.54) * amb + vec3(1.0, 0.9, 0.7) * dif;
							const vec3 tcr  = vec3(1.0, 0.21, 0.11);
#endif
							float ta = clamp(length(tcolsi.rgb), 0.0, 1.0);
							
							tcolsi   = clamp(tcolsi * tcolsi * 2.0, 0.0, 1.0);
							
							vec4 tvalx =
								vec4(tcolsi.rgb * shad + 3.0 * (tcr * tcolsi.rgb) * clamp(1.0 - (amb + dif), 0.0, 1.0),
									 min(tcolsi.a, ta));

							tvalx.rgb  = clamp(2.0 * tvalx.rgb * tvalx.rgb, 0.0, 1.0);
							tvalx.rgb *= (min(fadesi * 5.0, 1.0));
							colxsi[i]  = tvalx;
						}
					}
#endif
				}
			}
		}
	}

	// transparency logic and layers sorting
	float a = 1.0;
	if (dx[0].x < dx[1].x) {
		swapVec3(dx[0], dx[1]);
		swapInt(order[0], order[1]);
	} if (dx[1].x < dx[2].x) {
		swapVec3(dx[1], dx[2]);
		swapInt(order[1], order[2]);
	} if (dx[0].x < dx[1].x) {
		swapVec3(dx[0], dx[1]);
		swapInt(order[0], order[1]);
	}

	tout = max(max(dx[0].x, dx[1].x), dx[2].x);

	if (dx[0].y < 0.5)
		a = colx[order[0]].a;

#if !(defined(DEBUG) && defined(BUG))

	// self intersection
	bool[3] rul = bool[3](((dx[0].y > 0.5) && (dx[1].x <= 0.0)), ((dx[1].y > 0.5) && (dx[0].x > dx[1].z)),
						  ((dx[2].y > 0.5) && (dx[1].x > dx[2].z)));

	for (int k = 0; k < 3; k++) {
		if (rul[k]) {
			vec4 tcolxsi = colxsi[order[k]];
			vec4 tcolx   = colx[order[k]];
			vec4 tvalx   = mix(tcolxsi, tcolx, tcolx.a);
			vec4 tvalx2  = mix(vec4(0.0), tvalx, max(tcolx.a, tcolxsi.a));

			colx[order[k]] = tvalx;
			colx[order[k]] = tvalx2;
		}
	}

#endif

	float a1 = (dx[1].y < 0.5) ? colx[order[1]].a : ((dx[1].z > dx[0].x) ? colx[order[1]].a : 1.0);
	float a2 = (dx[2].y < 0.5) ? colx[order[2]].a : ((dx[2].z > dx[1].x) ? colx[order[2]].a : 1.0);

	col = mix(mix(colx[order[0]].rgb, colx[order[1]].rgb, a1), colx[order[2]].rgb, a2);
	a   = max(max(a, a1), a2);

	return vec4(col, a);
}

void main()
{
	vec2  fragCoord = inUV * pc.resolution.xy;
	float osc       = 0.5;
	vec3  l_dir     = normalize(vec3(0.0, 1.0, 0.0)) *  rotz(0.5);
	vec3 eye        = pc.position.xzy;
	vec3 w          = pc.direction.xzy;
	vec3 up         = vec3(0.0, 0.0, 1.0);
	vec3 u          = normalize(cross(w, up));
	vec3 v          = cross(u, w);
	vec4 tot        = vec4(0.0);

#if defined(AA_CUBE) || defined(AA_ALL)
#ifdef AA_CUBE
	const int AA = AA_CUBE;
#else
	const int AA = AA_ALL;
#endif
	vec3 incol_once    = vec3(0.0);
	vec4 incolbg_once  = vec4(0.0);
	vec4 outcolbg_once = vec4(0.0);
	bool in_once       = false;
	bool bg_in_once    = false;
	bool bg_out_once   = false;

	for (int mx = 0; mx < AA; mx++)
	for (int nx = 0; nx < AA; nx++) {
	vec2 o  = vec2(mod(float(mx + AA / 2), float(AA)), mod(float(nx + AA / 2), float(AA))) / float(AA) - 0.5;
	vec2 uv = (fragCoord + o - 0.5 * pc.resolution.xy) / pc.resolution.x;
#else
	vec2 uv = (fragCoord - 0.5 * pc.resolution.xy) / pc.resolution.x;
#endif
	vec3 rd = normalize(w * FDIST + uv.x * u + uv.y * v);

	vec3  ni;
	float t           = box(eye, rd, BOXDIMS, ni, true);
	vec3  ro          = eye + t * rd;
	vec2  coords      = ro.xy * ni.z / BOXDIMS.xy + ro.yz * ni.x / BOXDIMS.yz + ro.zx * ni.y / BOXDIMS.zx;
	float fadeborders = (1.0 - smoothstep(0.915, 1.05, abs(coords.x))) * (1.0 - smoothstep(0.915, 1.05, abs(coords.y)));

	if (0.0 < t) {
		float ang = -pc.time * 0.33;
		vec3  col = vec3(0.0);
#ifdef AA_CUBE
		if(in_once)
		col = incol_once;
		else{
		in_once=true;
#endif
		float talpha;
		float R0      = pow((IOR - 1.0) / (IOR + 1.0), 2.0);
		vec2  theta   = vec2(0.0);
		vec3  n       = vec3(cos(theta.x) * sin(theta.y), sin(theta.x) * sin(theta.y), cos(theta.y));
		vec3  nr      = n.zxy * ni.x + n.yzx * ni.y + n.xyz * ni.z;
		vec3  rdr     = reflect(rd, nr);
		vec3  reflcol = background(ro, rdr, l_dir, talpha);
		vec3  rd2     = refract(rd, nr, 1.0 / IOR);
		float accum   = 1.0;
		vec3  no2     = ni;
		vec3  ro_refr = ro;
		vec4  colo[2] = { vec4(0.0), vec4(0.0) };

		for (int j = 0; j < 2 + ANGLE_loops; j++) {
			float tb;
			vec2  coords2  = ro_refr.xy * no2.z + ro_refr.yz * no2.x + ro_refr.zx * no2.y;
			vec3  eye2     = vec3(coords2, -1.0);
			vec3  rd2trans = rd2.yzx * no2.x + rd2.zxy * no2.y + rd2.xyz * no2.z;

			rd2trans.z = -rd2trans.z;

			vec4 internalcol = insides(eye2, rd2trans, no2, l_dir, tb);
			
			if (0.0 < tb) {
				internalcol.rgb *= accum;
				colo[j]          = internalcol;
			}

			if ((tb <= 0.0) || (internalcol.a < 1.0)) {
				float tout = box(ro_refr, rd2, BOXDIMS, no2, false);

				no2 = n.zyx * no2.x + n.xzy * no2.y + n.yxz * no2.z;
				
				vec3  rout     = ro_refr + tout * rd2;
				vec3  rdout    = refract(rd2, -no2, IOR);
				float fresnel2 = R0 + (1.0 - R0) * pow(1.0 - dot(rdout, no2), 1.3);

				rd2 = reflect(rd2, -no2);

#ifdef backside_refl
				if(0.5 < dot(rdout, no2))
					fresnel2 = 1.0;
#endif
				ro_refr   = rout;
				ro_refr.z = max(ro_refr.z, -0.999);

				accum *= fresnel2;
			}
		}

		float fresnel = R0 + (1.0 - R0) * pow(1.0 - dot(-rd, nr), 5.0);
		
		col = mix(mix(colo[1].rgb * colo[1].a, colo[0].rgb, colo[0].a) * fadeborders, reflcol, pow(fresnel, 1.5));
		col = clamp(col, 0.0, 1.0);
#ifdef AA_CUBE
		}

	incol_once = col;
		
	if(!bg_in_once){
		float alpha;
		bg_in_once=true;
		incolbg_once = vec4(background(eye, rd, l_dir, alpha), 0.15);
#if defined(BG_ALPHA)||defined(ONLY_BOX)||defined(SHADOW_ALPHA)
		incolbg_once.w = alpha;
#endif
	}
#endif
		
	float cineshader_alpha = 0.0;
	cineshader_alpha = clamp(dot(eye,ro),0.0,1.0);
	vec4 tcolx = vec4(col, cineshader_alpha);
#if defined(BG_ALPHA)||defined(ONLY_BOX)||defined(SHADOW_ALPHA)
	tcolx.w = 1.0;
#endif
	tot += tcolx;
	} else {
		vec4 tcolx = vec4(0.0);
#ifdef AA_CUBE
		if(!bg_out_once){
		bg_out_once=true;
#endif
		float alpha;
		tcolx = vec4(background(eye, rd, l_dir, alpha), 1.0);
#if defined(BG_ALPHA)||defined(ONLY_BOX)||defined(SHADOW_ALPHA)
		tcolx.w = alpha;
#endif
#ifdef AA_CUBE
		outcolbg_once=tcolx;
		} else tcolx=max(outcolbg_once,incolbg_once);
#endif
		tot += tcolx;
	}
#if defined(AA_CUBE) || defined(AA_ALL)
	}
	tot /= float(AA*AA);
#endif
	outFragColor = tot;
#ifdef NO_ALPHA
	outFragColor.w = 1.0;
#endif
	outFragColor.rgb = clamp(outFragColor.rgb, 0.0, 1.0);
#if defined(BG_ALPHA)||defined(ONLY_BOX)||defined(SHADOW_ALPHA)

	outFragColor.rgb = outFragColor.rgb * outFragColor.w + texture(iChannel0, fragCoord / pc.resolution).rgb * (1.0 - outFragColor.w);
#endif
	//fragColor=vec4(fragColor.w);
}
