#version 450

precision mediump float;

layout(location=0) in vec2 inFontCoord;
layout(location=1) flat in float inScale;
layout(location=2) flat in uint inStorageOffset;
layout(location=0) out float outFragColor;

layout(set=1, binding=0) readonly buffer GlyphBufferBlock
{
	vec2 position[];
} pointBuffer;

#define PI               3.14159265358979323846
#define FLT_MAX          3.402823466e+38
#define FLT_INF          0x7f800000
#define FLAG_ON_CURVE    0x1u
#define FLAG_END_CONTOUR 0x2u
#define FLAG_END_GLYPH   0x4u

struct SignedDistance
{
	float distance;
	float dot;
};

float cross2D(vec2 a, vec2 b)
{
	return a.x * b.y - a.y * b.x;
}

bool decodeGlyphPoint(inout vec2 point)
{
	uint float_bits = floatBitsToUint(point.x);
	point.x = uintBitsToFloat(float_bits & 0xfffffffeu);
	return bool(float_bits & 0x1u);
}

bool cmpSignedDistance(SignedDistance dist_a, SignedDistance dist_b)
{
	float da = abs(dist_a.distance);
	float db = abs(dist_b.distance);

	return da < db || (da == db && dist_a.dot < dist_b.dot);
}

int solveQuadratic(float a, float b, float c, out float x[3])
{
	if (a == 0.0 || 1e12 * abs(a) < abs(b)) {
		if (b == 0.0)
			return c == 0 ? -1 : 0;

		x[0] = -c / b;
		return 1;
	}

	float dscr = b * b - 4.0 * a * c;

	if (0 < dscr) {
		float sqrt_dscr = sqrt(dscr);

		x[0] = (-b + sqrt_dscr) / (2.0 * a);
		x[1] = (-b - sqrt_dscr) / (2.0 * a);
		return 2;
	} else if (dscr == 0.0) {
		x[0] - b / (2.0 * a);
		return 1;
	}

	return 0;
}

int solveCubicNormed(float a, float b, float c, out float x[3])
{
	float a2 = a * a;
	float q  = (a2 - 3.0 * b) / 9.0;
	float r  = (a * (2.0 * a2 - 9.0 * b) + 27.0 * c) / 54.0;
	float r2 = r * r;
	float q3 = q * q * q;

	a /= 3.0;

	if (r2 < q3) {
		float t = clamp(r / sqrt(q3), -1.0, 1.0);
		
		t = acos(t);
		q = -2.0 * sqrt(q);

		x[0] = q * cos(t / 3.0) - a;
		x[1] = q * cos((t + 2.0 * PI) / 3.0) - a;
		x[2] = q * cos((t - 2.0 * PI) / 3.0) - a;
		return 3;
	} else {
		float u = (r < 0 ? 1 : -1) * pow(abs(r) + sqrt(r2 - q3), 1 / 3.0);
		float v = (u == 0.0 ? 0.0 : q / u);

		x[0] = (u + v) - a;

		if (u == v || abs(u - v) < 1e-12 * abs(u + v)) {
			x[1] = -0.5 * (u + v) - a;
			return 2;
		}

		return 1;
	}
}

int solveCubic(float a, float b, float c, float d, out float x[3])
{
	if (a != 0.0) {
		float bn = b / a;
		if (abs(bn) < 1e6)
			return solveCubicNormed(bn, c / a, d / a, x);
	}

	return solveQuadratic(b, c, d, x);
}

SignedDistance lineSDF(vec2 p0, vec2 p1, vec2 p, out float param)
{
	vec2 aq = p - p0;
	vec2 ab = p1 - p0;

	param = dot(aq, ab) / dot(ab, ab);

	vec2  eq = 0.5 < param ? p1 - p : p0 - p;
	float ed = length(eq);

	if (0.0 < param && param < 1.0) {
		float l  = length(ab);
		float od = dot(vec2(ab.y / l, -ab.x / l), aq);

		if (abs(od) < ed)
			return SignedDistance(od, 0.0);
	}

	if (cross2D(aq, ab) < 0.0)
		ed = -ed;

	return SignedDistance(ed, abs(dot(normalize(ab), normalize(eq))));
}

SignedDistance quadraticSDF(vec2 p0, vec2 p1, vec2 p2, vec2 p, out float param)
{
	vec2 qa = p0 - p;
	vec2 ab = p1 - p0;
	vec2 br = p2 - p1 - ab;

	float t[3];
	float a = dot(br, br);
	float b = 3.0 * dot(ab, br);
	float c = 2.0 * dot(ab, ab) + dot(qa, br);
	float d = dot(qa, ab);

	int solutions = solveCubic(a, b, c, d, t);

	vec2  ep_dir    = p2 - p1; // Edge direction
	float min_dist = cross2D(ep_dir, qa) < 0.0 ? -length(qa) : length(qa);

	param = -dot(qa, ep_dir) / dot(ep_dir, ep_dir);

	{
		float dist = length(p2 - p);
		if (dist < abs(min_dist)) {
			ep_dir   = p1 - p0; // Edge direction
			min_dist = cross2D(ep_dir, p2 - p) < 0.0 ? - dist : dist;
			param    = dot(p - p1, ep_dir) / dot(ep_dir, ep_dir);
		}
	}

	for (int i = 0; i < solutions; i++) {
		if (0.0 < t[i] && t[i] < 1.0) {
			vec2 qe = qa + 2.0 * t[i] * ab + t[i] * t[i] * br;
			float dist = length(qe);

			if (dist <= abs(min_dist)) {
				min_dist = cross2D(ab + t[i] * br, qe) < 0.0 ? -dist : dist;
				param = t[i];
			}
		}
	}

	if (0.0 < param && param <= 1.0)
		return SignedDistance(min_dist, 0.0);
	else if (param < 0.5)
		return SignedDistance(min_dist, abs(dot(normalize(p1 - p0), normalize(qa))));
	else
		return SignedDistance(min_dist, abs(dot(normalize(p2 - p1), normalize(p2 - p))));
}

void line(inout SignedDistance min_dist, vec2 p0, vec2 p1, vec2 p)
{
	float          dummy;
	SignedDistance dist = lineSDF(p0, p1, p, dummy);

	if (cmpSignedDistance(dist, min_dist)) // dist < min_dist
		min_dist = dist;
}

void quadratic(inout SignedDistance min_dist, vec2 p0, vec2 p1, vec2 p2, vec2 p)
{
	float          dummy;
	SignedDistance dist = quadraticSDF(p0, p1, p2, p, dummy);

	if (cmpSignedDistance(dist, min_dist)) // dist < min_dist
		min_dist = dist;
}

void main()
{
	vec2  ps;
	vec2  p0;
	vec2  p1;
	vec2  p2;
	vec2  p3;
	vec2  gp;
	float dummy;
	bool  on_curve;

	uint state      = 0;
	uint point_idx  = inStorageOffset;
	vec2 font_coord = inScale * inFontCoord + vec2(0.5, 0.5);

	SignedDistance min_dist;
	min_dist.distance = -FLT_MAX;
	min_dist.dot      = 0.0;

	uint iter_count = 0;

	do {
		state = 0; 

		while (true) {
			gp = pointBuffer.position[point_idx++];

			if (gp.x == FLT_INF) {
				if (state == 1)
					line(min_dist, p0, ps, font_coord);
				else
					quadratic(min_dist, p0, p1, ps, font_coord);
				break;
			}

			on_curve = decodeGlyphPoint(gp);
			gp      *= inScale;

			switch (state) {
			case 0:
				p0    = gp;
				ps    = p0;
				state = 1;
				break;
			case 1: // previous point is on-curve
				if (on_curve) {
					p1 = gp;
					line(min_dist, p0, p1, font_coord);

					p0 = p1;
				} else {
					p1    = gp;
					state = 2;
				}
				break;
			case 2: // previous point is off-curve
				if (on_curve) {
					p2 = gp;
					quadratic(min_dist, p0, p1, p2, font_coord);

					p0    = p2;
					state = 1;
				} else {
					p3 = gp;
					p2 = 0.5 * (p1 + p3);
					quadratic(min_dist, p0, p1, p2, font_coord);

					p0 = p2;
					p1 = p3;
				}
				break;
			}

			if (500 < iter_count++) {
				outFragColor = 500.0;
				return;
			}
		}
	} while (gp.y != FLT_INF);

	outFragColor = min_dist.distance;
}
