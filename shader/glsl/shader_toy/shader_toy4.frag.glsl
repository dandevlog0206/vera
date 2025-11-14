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

#define lofi(i,j) (floor((i)/(j))*(j))
#define lofir(i,j) (round((i)/(j))*(j))

const float PI  = acos(-1.0);
const float TAU = PI * 2.0;

mat2 r2d(float t)
{
	float c = cos(t);
	float s = sin(t);
	return mat2(c, s, -s, c);
}

mat3 orthbas(vec3 z)
{
	z = normalize(z);
	
	vec3 up = 0.999 < abs(z.y) ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 1.0, 0.0);
	vec3 x  = normalize(cross(up, z));
	
	return mat3(x, cross(z, x), z);
}

uvec3 pcg3d(uvec3 s)
{
	s    = s * 1145141919u + 1919810u;
	s.x += s.y * s.z;
	s.y += s.z * s.x;
	s.z += s.x * s.y;
	s   ^= s >> 16;
	s.x += s.y * s.z;
	s.y += s.z * s.x;
	s.z += s.x * s.y;

	return s;
}

vec3 pcg3df(vec3 s)
{
	uvec3 r = pcg3d(floatBitsToUint(s));
	return vec3(r) / float(0xffffffffu);
}

struct Grid
{
	vec3  s;
	vec3  c;
	vec3  h;
	int   i;
	float d;
};

Grid dogrid(vec3 ro, vec3 rd)
{
	Grid r;
	r.s = vec3(2.0, 2.0, 100.0);
	for(int i = 0; i < 3; i++) {
		r.c = (floor(ro / r.s) + 0.5) * r.s;
		r.h = pcg3df(r.c);
		r.i = i;

		if(r.h.x < 0.4) {
			break;
		} else if(i == 0) {
			r.s = vec3(2.0, 1.0, 100.0);
		} else if(i == 1) {
			r.s = vec3(1.0, 1.0, 100.0);
		}
	}
	
	vec3  src = -(ro - r.c) / rd;
	vec3  dst = abs(0.501 * r.s / rd);
	vec3  bv  = src + dst;
	float b   = min(min(bv.x, bv.y), bv.z);
	r.d = b;
	
	return r;
}

float sdbox(vec3 p, vec3 s)
{
	vec3 d = abs(p) - s;
	return length(max(d, 0.0)) + min(0.0, max(max(d.x, d.y), d.z));
}

float sdbox(vec2 p, vec2 s)
{
	vec2 d = abs(p) - s;
	return length(max(d, 0.0)) + min(0.0, max(d.x, d.y));
}

vec4 map(vec3 p, Grid grid)
{
	p -= grid.c;
	p.z += 0.4 * sin(2.0 * pc.time + 1.0 * fract(grid.h.z * 28.0) + 0.3 * (grid.c.x + grid.c.y));
	
	vec3  psize = grid.s / 2.0;
	psize.z = 1.0;
	psize  -= 0.02;
	float d = sdbox(p + vec3(0.0, 0.0, 1.0), psize) - 0.02;
	
	float pcol = 1.0;

	vec3 pt = p;
	
	if(grid.i == 0) { //2x2
		if(grid.h.y < 0.3) { //speaker
			vec3 c = vec3(0.0);
			pt.xy *= r2d(PI / 4.0);
			c.xy   = lofir(pt.xy, 0.1);
			pt    -= c;
			pt.xy *= r2d(-PI / 4.0);
			
			float r    = 0.02 * smoothstep(0.9, 0.7, abs(p.x)) * smoothstep(0.9, 0.7, abs(p.y));
			float hole = length(pt.xy) - r;
			d = max(d, -hole);
		} else if(grid.h.y < 0.5) { //eq
			vec3 c = vec3(0.0);
			c.x  = clamp(lofir(pt.x, 0.2), -0.6, 0.6);
			pt  -= c;
			float hole = sdbox(pt.xy, vec2(0.0, 0.7)) - 0.03;
			d = max(d, -hole);
			
			pt.y -= 0.5 - smoothstep(-0.5, 0.5, sin(pc.time + c.x + grid.h.z * 100.0));
			float d2 = sdbox(pt, vec3(0.02, 0.07, 0.07)) - 0.03;
			
			if(d2 < d) {
				float l = step(abs(pt.y), 0.02);
				return vec4(d2, 2.0 * l, l, 0.0);
			}
			
			pt  = p;
			c.y = clamp(lofir(pt.y, 0.2), -0.6, 0.6);
			pt -= c;
			pcol *= smoothstep(0.0, 0.01, sdbox(pt.xy, vec2(0.07, 0.0)) - 0.005);

			pt  = p;
			c.y = clamp(lofir(pt.y, 0.6), -0.6, 0.6);
			pt -= c;
			pcol *= smoothstep(0.0, 0.01, sdbox(pt.xy, vec2(0.1, 0.0)) - 0.01);
			
			pcol = mix(1.0, pcol, smoothstep(0.0, 0.01, sdbox(pt.xy, vec2(0.03, 1.0)) - 0.01));

		} else if(grid.h.y < 0.6) { //kaosspad
			float hole = sdbox(p.xy, vec2(0.9, 0.9) + 0.02);
			d = max(d, -hole);

			float d2 = sdbox(p, vec3(0.9, 0.9, 0.05));

			if(d2 < d) {
				float l = step(abs(p.x), 0.7) * step(abs(p.y), 0.7);
				return vec4(d2, 4.0 * l, 0.0, 0.0);
			}
		} else if(grid.h.y < 1.0) { //bigass knob
			float ani = smoothstep(-0.5, 0.5, sin(pc.time + grid.h.z * 100.0));
			pt.xy *= r2d(PI / 6.0 * 5.0 * mix(-1.0, 1.0, ani));

			float metal = step(length(pt.xy), 0.45);
			float wave  = metal * sin(length(pt.xy) * 500.0) / 1000.0;
			float d2    = length(pt.xy) - 0.63 + 0.05 * pt.z - 0.02 * cos(8.0 * atan(pt.y, pt.x));
			d2 = max(d2, abs(pt.z) - 0.4 - wave);

			float d2b = length(pt.xy) - 0.67 + 0.05 * pt.z;
			d2b = max(d2b, abs(pt.z) - 0.04);
			d2  = min(d2, d2b);
			
			if(d2 < d) {
				float l = smoothstep(0.01, 0.0, length(pt.xy - vec2(0.0, 0.53)) - 0.03);
				return vec4(d2, 3.0 * metal, l, 0.0);
			}
			
			pt = p;
			float a = clamp(lofir(atan(-pt.x, pt.y), PI / 12.0), -PI / 6.0 * 5.0, PI / 6.0 * 5.0);
			pt.xy *= r2d(a);
			pcol  *= smoothstep(0.0, 0.01, length(pt.xy - vec2(0.0, 0.74)) - 0.015);

			pt = p;
			a  = clamp(lofir(atan(-pt.x, pt.y), PI / 6.0 * 5.0), -PI / 6.0 * 5.0, PI / 6.0 * 5.0);
			pt.xy *= r2d(a);
			pcol  *= smoothstep(0.0, 0.01, length(pt.xy - vec2(0.0, 0.74)) - 0.03);
			
			float d3 = length(p - vec3(0.7, -0.7, 0.0)) - 0.05;
			
			if(d3 < d) {
				float led = 1.0 - ani;
				led *= 0.5 + 0.5 * sin(pc.time * exp2(3.0 + 3.0 * grid.h.z));
				return vec4(d3, 2.0, led, 0.0);
			}
		}
	} else if(grid.i == 1) { //2x1
		if(grid.h.y < 0.4) { //fader
			float hole = sdbox(p.xy, vec2(0.9, 0.05));
			d = max(d, -hole);
			
			float ani = smoothstep(-0.2, 0.2, sin(pc.time + grid.h.z * 100.0));
			pt.x -= mix(-0.8, 0.8, ani);
			
			float d2 = sdbox(pt, vec3(0.07, 0.25, 0.4)) + 0.05 * p.z;
			d2 = max(d2, -p.z);

			if(d2 < d) {
				float l = smoothstep(0.01, 0.0, abs(p.y) - 0.02);
				return vec4(d2, 0.0, l, 0.0);
			}
			
			pt = p;
			vec3 c = vec3(0.0);
			c.x  = clamp(lofir(pt.x, 0.2), -0.8, 0.8);
			pt  -= c;
			pcol *= smoothstep(0.0, 0.01, sdbox(pt.xy, vec2(0.0, 0.15)) - 0.005);

			pt = p;
			c  = vec3(0.0);
			c.x = clamp(lofir(pt.x, 0.8), -0.8, 0.8);
			pt -= c;
			pcol *= smoothstep(0.0, 0.01, sdbox(pt.xy, vec2(0.0, 0.18)) - 0.01);
			
			pcol = mix(1.0, pcol, smoothstep(0.0, 0.01, sdbox(p.xy, vec2(1.0, 0.08))));
		} else if(grid.h.y < 0.5) { //button
			vec3 c = vec3(0.0);
			c.x  = clamp(lofi(pt.x, 0.44) + 0.44 / 2.0, -0.44 * 1.5, 0.44 * 1.5);
			pt  -= c;

			float hole = sdbox(pt.xy, vec2(0.19, 0.33)) - 0.01;
			d = max(d, -hole);
			
			float ani = smoothstep(0.8, 0.9, sin(10.0 * pc.time - c.x * 2.2 + grid.h.z * 100.0));

			vec4  fuck = vec4(d, 0.0, 0.0, 0.0);
			float d3   = length(pt - vec3(0.0, 0.22, 0.04)) - 0.05;
			
			if(d3 < fuck.x) {
				float led = ani;
				fuck = vec4(d3, 2.0, led, 0.0);
			}

			float d2 = sdbox(pt, vec3(0.17, 0.3, 0.05)) - 0.01;
			d2 = min(d2, sdbox(pt - vec3(0.0, -0.1, 0.0), vec3(0.17, 0.2, 0.08)) - 0.01) + 0.5 * pt.z;

			if(d2 < fuck.x) {
				fuck = vec4(d2, 5.0, fract(grid.h.z * 8.89), 0.0);
			}
			
			if(fuck.x < d) {
				return fuck;
			}
			
		} else if(grid.h.y < 1.0) { //meter
			float hole = sdbox(p.xy, vec2(0.9, 0.3) + 0.02);
			d = max(d, -hole);

			float d2 = sdbox(p, vec3(0.9, 0.3, 0.1));

			if(d2 < d) {
				float l = step(abs(p.x), 0.8) * step(abs(p.y), 0.2);
				return vec4(d2, l, 0.0, 0.0);
			}
		}
	} else { //1x1
		if(grid.h.y < 0.5) { //knob
			float hole = length(p.xy) - 0.25;
			d = max(d, -hole);
			
			float ani = smoothstep(-0.5, 0.5, sin(2.0 * pc.time + grid.h.z * 100.0));
			pt.xy *= r2d(PI / 6.0 * 5.0 * mix(-1.0, 1.0, ani));
			
			float d2 = length(pt.xy) - 0.23 + 0.05 * pt.z;
			d2 = max(d2, abs(pt.z) - 0.4);
			
			if(d2 < d) {
				float l = smoothstep(0.01, 0.0, abs(pt.x) - 0.015);
				l *= smoothstep(0.01, 0.0, -pt.y + 0.05);
				return vec4(d2, 0.0, l, 0.0);
			}
			
			pt = p;
			float a = clamp(lofir(atan(-pt.x, pt.y), PI / 6.0), -PI / 6.0 * 5.0, PI / 6.0 * 5.0);
			pt.xy *= r2d(a);
			pcol  *= smoothstep(0.0, 0.01, sdbox(pt.xy - vec2(0.0, 0.34), vec2(0.0, 0.02)) - 0.005);

			pt = p;
			a  = clamp(lofir(atan(-pt.x, pt.y), PI / 6.0 * 5.0), -PI / 6.0 * 5.0, PI / 6.0 * 5.0);
			pt.xy *= r2d(a);
			pcol  *= smoothstep(0.0, 0.01, sdbox(pt.xy - vec2(0.0, 0.34), vec2(0.0, 0.03)) - 0.01);
		} else if(grid.h.y < 0.8) { //jack
			float hole = length(p.xy) - 0.1;
			d = max(d, -hole);
			
			float d2 = length(p.xy) - 0.15;
			d2 = max(d2, abs(p.z) - 0.12);
			
			pt.xy *= r2d(100.0 * grid.h.z);
			float d3 = abs(pt.y) - 0.2;
			pt.xy *= r2d(PI / 3.0 * 2.0);
			d3 = max(d3, abs(pt.y) - 0.2);
			pt.xy *= r2d(PI / 3.0 * 2.0);
			d3 = max(d3, abs(pt.y) - 0.2);
			d3 = max(d3, abs(p.z) - 0.03);

			d2 = min(d2, d3);
			d2 = max(d2, -hole);
			
			if(d2 < d) {
				return vec4(d2, 3.0, 0.0, 0.0);
			}
		} else if(grid.h.y < 0.99) { //button
			pt.y += 0.08;
			
			float hole = sdbox(pt.xy, vec2(0.22)) - 0.05;
			d = max(d, -hole);
			
			float ani  = sin(2.0 * pc.time + grid.h.z * 100.0);
			float push = smoothstep(0.3, 0.0, abs(ani));
			ani = smoothstep(-0.1, 0.1, ani);
			pt.z += 0.06 * push;

			float d2 = sdbox(pt, vec3(0.2, 0.2, 0.05)) - 0.05;

			if(d2 < d) {
				return vec4(d2, 0.0, 0.0, 0.0);
			}
			
			float d3 = length(p - vec3(0.0, 0.3, 0.0)) - 0.05;
			
			if(d3 < d) {
				float led = ani;
				return vec4(d3, 2.0, led, 0.0);
			}
		} else if(grid.h.y < 1.0) { //0b5vr
			pt    = abs(pt);
			pt.xy = pt.x < pt.y ? pt.yx : pt.xy;
			pcol *= smoothstep(0.0, 0.01, sdbox(pt.xy, vec2(0.05)));
			pcol *= smoothstep(0.0, 0.01, sdbox(pt.xy - vec2(0.2, 0.0), vec2(0.05, 0.15)));
			pcol  = 1.0 - pcol;
		}
	}
	
	return vec4(d, 0.0, pcol, 0.0);
}

vec3 nmap(vec3 p, Grid grid, float dd)
{
	vec2 d = vec2(0.0, dd);
	return normalize(vec3(
		map(p + d.yxx, grid).x - map(p - d.yxx, grid).x,
		map(p + d.xyx, grid).x - map(p - d.xyx, grid).x,
		map(p + d.xxy, grid).x - map(p - d.xxy, grid).x
	));
}

struct March
{
	vec4  isect;
	vec3  rp;
	float rl;
	Grid  grid;
};

March domarch(vec3 ro, vec3 rd, int iter)
{
	float rl      = 1E-2;
	vec3  rp      = ro + rd * rl;
	vec4  isect;
	Grid  grid;
	float gridlen = rl;
	
	for(int i = 0; i < iter; i++) {
		if(gridlen <= rl) {
			grid     = dogrid(rp, rd);
			gridlen += grid.d;
		}
		
		isect = map(rp, grid);
		rl    = min(rl + isect.x * 0.8, gridlen);
		rp    = ro + rd * rl;
		
		if(abs(isect.x) < 1E-4) { break; }
		if(rl > 50.0) { break; }
	}
	
	March r;
	r.isect = isect;
	r.rp    = rp;
	r.rl    = rl;
	r.grid  = grid;
	
	return r;
}

void main()
{
	vec2  fragCoord = inUV * pc.resolution;
	vec2  uv        = vec2(fragCoord.x / pc.resolution.x, fragCoord.y / pc.resolution.y);
	vec2  p         = uv * 2.0 - 1.0;
	p.x *= pc.resolution.x / pc.resolution.y;

	vec3 col = vec3(0.0);

	float canim = smoothstep(-0.2, 0.2, sin(pc.time));
	vec3  co    = mix(vec3(-6.0, -8.0, -40.0), vec3(0.0, -2.0, -40.0), canim);
	vec3  ct    = vec3(0.0, 0.0, -50.0);
	float cr    = mix(0.5, 0.0, canim);
	co.xy += pc.time;
	ct.xy += pc.time;
	mat3 cb = orthbas(co - ct);
	vec3 ro = co + cb * vec3(4.0 * p * r2d(cr), 0.0);
	vec3 rd = cb * normalize(vec3(0.0, 0.0, -2.0));
	
	March march = domarch(ro, rd, 100);

	if(march.isect.x < 1E-2) {
		vec3  basecol = vec3(0.5);
		vec3  speccol = vec3(0.2);
		float specpow = 30.0;
		float ndelta  = 1E-4;
		
		float mtl  = march.isect.y;
		float mtlp = march.isect.z;
		if(mtl == 0.0) {
			mtlp    = mix(mtlp, 1.0 - mtlp, step(fract(march.grid.h.z * 66.0), 0.1));
			vec3 c  = 0.9 + 0.0 * sin(0.1 * (march.grid.c.x + march.grid.c.y) + march.grid.h.z + vec3(0.0, 2.0, 3.0));
			basecol = mix(vec3(0.04), c, mtlp);
		} else if(mtl == 1.0) {
			basecol = vec3(0.0);
			speccol = vec3(0.5);
			specpow = 60.0;
			
			vec2 size = vec2(0.05, 0.2);
			vec2 pp   = (march.rp - march.grid.c).xy;
			vec2 c    = lofi(pp.xy, size) + size / 2.0;
			vec2 cc   = pp - c;
			vec3 led  = vec3(1.0);
			led *= exp(-60.0 * sdbox(cc, vec2(0.0, 0.08)));
			led *= c.x > 0.5 ? vec3(5.0, 1.0, 2.0) : vec3(1.0, 5.0, 2.0);
			float lv = abs(sin(mod(pc.time, PI)));
			col     += led * step(c.x, -0.8 + 1.6 * lv);
			basecol  = 0.04 * led;
		} else if(mtl == 2.0) { //led
			basecol = vec3(0.0);
			speccol = vec3(1.0);
			specpow = 100.0;
			
			col += mtlp * vec3(2.0, 0.5, 0.5);
		} else if(mtl == 3.0) { //metal
			basecol = vec3(0.2);
			speccol = vec3(1.8);
			specpow = 100.0;
			ndelta  = 3E-2;
		} else if(mtl == 4.0) { //kaoss
			basecol = vec3(0.0);
			speccol = vec3(0.5);
			specpow = 60.0;
			
			vec2 size = vec2(0.1);
			vec2 pp   = (march.rp - march.grid.c).xy;
			vec2 c    = lofi(pp.xy, size) + size / 2.0;
			vec2 cc   = pp - c;
			vec3 led  = vec3(1.0);
			led *= exp(-60.0 * sdbox(cc, vec2(0.0, 0.0)));
			led *= vec3(2.0, 1.0, 2.0);
			float plasma = sin(length(c) * 10.0 - 10.0 * pc.time + march.grid.h.z * 0.7);
			plasma += sin(c.y * 10.0 - 7.0 * pc.time);
			led    *= 0.5 + 0.5 * sin(plasma);
			col    += 2.0 * led;
			basecol = 0.04 * led;
		} else if(mtl == 5.0) { //808
			basecol = vec3(0.9, mtlp, 0.02);
		}
		
		vec3 n = nmap(march.rp, march.grid, ndelta);
		vec3 v = -rd;
		
		{
			vec3  l      = normalize(vec3(1.0, 3.0, 5.0));
			vec3  h      = normalize(l + v);
			float dotnl  = max(0.0, dot(n, l));
			float dotnh  = max(0.0, dot(n, h));
			float shadow = step(1E-1, domarch(march.rp, l, 30).isect.x);
			vec3  diff   = basecol / PI;
			vec3  spec   = speccol * pow(dotnh, specpow);
			col += vec3(0.5, 0.6, 0.7) * shadow * dotnl * (diff + spec);
		}
		{
			vec3  l      = normalize(vec3(-1.0, -1.0, 5.0));
			vec3  h      = normalize(l + v);
			float dotnl  = max(0.0, dot(n, l));
			float dotnh  = max(0.0, dot(n, h));
			float shadow = step(1E-1, domarch(march.rp, l, 30).isect.x);
			vec3  diff   = basecol / PI;
			vec3  spec   = speccol * pow(dotnh, specpow);
			col += shadow * dotnl * (diff + spec);
		}
	}

	col = pow(col, vec3(0.4545));
	col = smoothstep(vec3(0.0, -0.1, -0.2), vec3(1.0, 1.1, 1.2), col);

	outFragColor = vec4(col, 1.0);
}
