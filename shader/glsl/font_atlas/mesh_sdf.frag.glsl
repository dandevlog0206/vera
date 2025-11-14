#version 450

#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_CONTOUR_COUNT 128
#define PI                3.14159265358979323846
#define FLT_MAX           3.402823466e+38
#define FLT_INF           0x7f800000

precision highp float;

layout(location=0) in vec2       inFontCoord;
layout(location=1) flat in float inScale;
layout(location=2) flat in uint  inStorageOffset;
layout(location=3) flat in uint  inLayerIndex;

layout(set=0, binding=1) readonly buffer StorageBufferBlock
{
	vec2 position[];
} storageBuffer;

layout(set=0, binding=2, r32f) uniform image2D glyphTextures[];

struct SignedDistance
{
	float distance;
	float dot;
};

float cross2D(vec2 a, vec2 b)
{
	return a.x * b.y - a.y * b.x;
}

float shoelace(vec2 a, vec2 b)
{
	return (b.x - a.x) * (a.y + b.y);
}

bool decodeGlyphPoint(inout vec2 point)
{
	uint float_bits = floatBitsToUint(point.x);
	point.x = uintBitsToFloat(float_bits & 0xfffffffeu);
	return bool(float_bits & 0x1u);
}

void mergeSignedDistance(inout SignedDistance dst, SignedDistance src)
{
	float da = abs(dst.distance);
	float db = abs(src.distance);

	if (da > db || (da == db && dst.dot > src.dot))
		dst = src;
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
		x[0] = -b / (2.0 * a);
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
		float u = (r < 0.0 ? 1.0 : -1.0) * pow(abs(r) + sqrt(r2 - q3), 1.0 / 3.0);
		float v = (u == 0.0 ? 0.0 : q / u);

		x[0] = (u + v) - a;

		if (u == v || abs(u - v) < 1e-5 * abs(u + v)) {
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
		if (abs(bn) < 1e3)
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
	vec2 qb = p2 - p;
	vec2 ab = p1 - p0;
	vec2 br = p2 - p1 - ab;

	float a = dot(br, br);
	float b = 3.0 * dot(ab, br);
	float c = 2.0 * dot(ab, ab) + dot(qa, br);
	float d = dot(qa, ab);

	vec2  ep_dir;
	float min_dist;
	float dist_a = length(qa);
	float dist_b = length(qb);

	if (dist_a < dist_b) {
		ep_dir   = p1 - p0;
		min_dist = cross2D(ep_dir, qa) < 0.0 ? -dist_a : dist_a;
		param    = -dot(qa, ep_dir) / dot(ep_dir, ep_dir);
	} else {
		ep_dir   = p2 - p1;
		min_dist = cross2D(ep_dir, qb) < 0.0 ? -dist_b : dist_b;
		param    = dot(p - p1, ep_dir) / dot(ep_dir, ep_dir);
	}

	float x[3];
	int   solutions = solveCubic(a, b, c, d, x);

	for (int i = 0; i < solutions; i++) {
		float t = x[i];

		if (0.0 < t && t < 1.0) {
			vec2  qe   = qa + 2.0 * t * ab + t * t * br;
			float dist = length(qe);

			if (dist <= abs(min_dist)) {
				min_dist = cross2D(ab + t * br, qe) < 0.0 ? -dist : dist;
				param    = t;
			}
		}
	}

	if (0.0 < param && param <= 1.0)
		return SignedDistance(min_dist, 0.0);
	else if (param < 0.5)
		return SignedDistance(min_dist, abs(dot(normalize(p1 - p0), normalize(qa))));
	else
		return SignedDistance(min_dist, abs(dot(normalize(p2 - p1), normalize(qb))));
}

void addLine(inout SignedDistance min_dist, vec2 p0, vec2 p1, vec2 p)
{
	float          dummy;
	SignedDistance dist = lineSDF(p0, p1, p, dummy);

	mergeSignedDistance(min_dist, dist);
}

void addQuadratic(inout SignedDistance min_dist, vec2 p0, vec2 p1, vec2 p2, vec2 p)
{
	float          dummy;
	SignedDistance dist = quadraticSDF(p0, p1, p2, p, dummy);

	mergeSignedDistance(min_dist, dist);
}

void writeColor(float dist)
{
	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec4  color = vec4(dist, 0.0, 0.0, 0.0);
	imageStore(glyphTextures[nonuniformEXT(inLayerIndex)], coord, color);
}

void main()
{
	SignedDistance min_distances[MAX_CONTOUR_COUNT];
	int            windings[MAX_CONTOUR_COUNT];
	vec2           gp;

	vec2 font_coord = inScale * inFontCoord;
	uint point_idx  = inStorageOffset;
	uint cont_count = 0;
	uint iter_count = 0;

	do {
		vec2 ps;
		vec2 p0;
		vec2 p1;
		vec2 p2;
		vec2 p3;
		bool on_curve;

		SignedDistance min_dist = SignedDistance(-FLT_MAX, 0.0);
		float          winding_score = 0.0;
		uint           state = 0;

		while (true) {
			gp = storageBuffer.position[point_idx++];

			if (gp.x == FLT_INF) {
				if (state == 1)
					addLine(min_dist, p0, ps, font_coord);
				else
					addQuadratic(min_dist, p0, p1, ps, font_coord);

				winding_score += shoelace(p0, ps);
				break;
			}

			on_curve = decodeGlyphPoint(gp);
			gp      *= inScale;

			switch (state) {
			case 0: // starting point
				p0    = gp;
				ps    = p0;
				state = 1;
				break;
			case 1: // previous point is on-curve
				if (on_curve) {
					p1 = gp;
					addLine(min_dist, p0, p1, font_coord);

					winding_score += shoelace(p0, p1);
					p0             = p1;
				} else {
					p1    = gp;
					state = 2;
				}
				break;
			case 2: // previous point is off-curve
				if (on_curve) {
					p2 = gp;
					addQuadratic(min_dist, p0, p1, p2, font_coord);

					winding_score += shoelace(p0, p2);
					p0             = p2;
					state          = 1;
				} else {
					p3 = gp;
					p2 = 0.5 * (p1 + p3);
					addQuadratic(min_dist, p0, p1, p2, font_coord);

					winding_score += shoelace(p0, p2);
					p0             = p2;
					p1             = p3;
				}
				break;
			}

			// Safety break to avoid infinite loops in malformed glyphs.
			if (1000 < iter_count++) return;
		}

		windings[cont_count]      = winding_score > 0.0 ? 1 : (winding_score < 0.0 ? -1 : 0);
		min_distances[cont_count] = min_dist;
		cont_count++;
	} while (gp.y != FLT_INF);

	SignedDistance shape_signed_dist = SignedDistance(-FLT_MAX, 0.0);
	SignedDistance inner_signed_dist = SignedDistance(-FLT_MAX, 0.0);
	SignedDistance outer_signed_dist = SignedDistance(-FLT_MAX, 0.0);

	for (int i = 0; i < cont_count; ++i) {
		SignedDistance edge_dist = min_distances[i];

		mergeSignedDistance(shape_signed_dist, edge_dist);

		if (windings[i] > 0 && edge_dist.distance >= 0)
			mergeSignedDistance(inner_signed_dist, edge_dist);
		if (windings[i] < 0 && edge_dist.distance <= 0)
			mergeSignedDistance(outer_signed_dist, edge_dist);
	}

	float result_dist = -FLT_MAX;
	float shape_dist  = shape_signed_dist.distance;
	float inner_dist  = inner_signed_dist.distance;
	float outer_dist  = outer_signed_dist.distance;
	int   winding     = 0;

	if (inner_dist >= 0.0 && abs(inner_dist) <= abs(outer_dist)) {
		result_dist = inner_dist;
		winding     = 1;

		for (int i = 0; i < cont_count; ++i) {
			if (windings[i] > 0) {
				float cont_dist = min_distances[i].distance;
				if (abs(cont_dist) < abs(outer_dist) && cont_dist > result_dist)
					result_dist = cont_dist;
			}
		}
	} else if (outer_dist <= 0.0 && abs(outer_dist) < abs(inner_dist)) {
		result_dist = outer_dist;
		winding     = -1;

		for (int i = 0; i < cont_count; ++i) {
			if (windings[i] < 0) {
				float cont_dist = min_distances[i].distance;
				if (abs(cont_dist) < abs(inner_dist) && cont_dist < result_dist)
					result_dist = cont_dist;
			}
		}
	} else {
		writeColor(shape_dist);
		return;
	}

	for (int i = 0; i < cont_count; ++i) {
		if (windings[i] != winding) {
			float cont_dist = min_distances[i].distance;
			if (cont_dist * result_dist >= 0.0 && abs(cont_dist) < abs(result_dist))
				result_dist = cont_dist;
		}
	}

	writeColor(result_dist);
}
