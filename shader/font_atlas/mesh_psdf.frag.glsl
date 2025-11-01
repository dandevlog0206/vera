#version 450

#extension GL_EXT_nonuniform_qualifier : enable

#define MAX_CONTOUR_COUNT          128
#define MAX_EDGE_COUNT             1024
#define DISTANCE_DELTA_FACTOR      1.001
#define PI                         3.14159265358979323846
#define FLT_MAX                    3.402823466e+38
#define FLT_INF                    0x7f800000

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

struct EdgeSegment
{
	vec2 p0;
	vec2 p1; // if Edge is line p1 is same as p0
	vec2 p2;
};

struct PerpendicularSelector
{
	SignedDistance minTrueDistance;
	EdgeSegment    nearEdge;
	float          nearEdgeParam;
	float          minNegativePerpendicularDistance;
	float          minPositivePerpendicularDistance;
};

float cross2D(vec2 a, vec2 b)
{
	return a.x * b.y - a.y * b.x;
}

float shoelace(vec2 a, vec2 b)
{
	return (b.x - a.x) * (a.y + b.y);
}

vec2 normalizeAllowZero(in vec2 v)
{
	float len = length(v);
	return len > 0.0 ? v / len : vec2(0.0);
}

bool decodeGlyphPoint(inout vec2 point)
{
	uint float_bits = floatBitsToUint(point.x);
	point.x = uintBitsToFloat(float_bits & 0xfffffffeu);
	return bool(float_bits & 0x1u);
}

bool compareSignedDistance(in SignedDistance dist_a, in SignedDistance dist_b)
{
	float da = abs(dist_a.distance);
	float db = abs(dist_b.distance);

	return da < db || (da == db && dist_a.dot < dist_b.dot);
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

SignedDistance edgeSDF(in EdgeSegment edge, in vec2 p, out float param)
{
	if (edge.p0 == edge.p1)
		return lineSDF(edge.p0, edge.p2, p, param);
	else
		return quadraticSDF(edge.p0, edge.p1, edge.p2, p, param);
}

vec2 edgeDirection0(in EdgeSegment edge)
{
	if (edge.p0 == edge.p1)
		return normalizeAllowZero(edge.p2 - edge.p0);

	return normalizeAllowZero(edge.p1 - edge.p0);
}

vec2 edgeDirection1(in EdgeSegment edge)
{
	if (edge.p0 == edge.p1)
		return normalizeAllowZero(edge.p2 - edge.p0);

	return normalizeAllowZero(edge.p2 - edge.p1);
}

bool getPerpendicularDistance(
	in    vec2                  ep,
	in    vec2                  edge_dir,
	inout float                 dist
) {
	float ts = dot(ep, edge_dir);

	if (ts > 0) {
		float perp_dist= cross2D(ep, edge_dir);
		
		if (abs(perp_dist) < abs(dist)) {
			dist = perp_dist;
			return true;
		}
	}

	return false;
}

void resetPerpendicularSelector(out PerpendicularSelector selector)
{
	selector.minTrueDistance                  = SignedDistance(-FLT_MAX, 0.0);
	selector.nearEdge                         = EdgeSegment(vec2(FLT_INF), vec2(FLT_INF), vec2(FLT_INF));
	selector.nearEdgeParam                    = 0.0;
	selector.minNegativePerpendicularDistance = -FLT_MAX;
	selector.minPositivePerpendicularDistance = FLT_MAX;
}

void addEdgeTrueDistance(
	inout PerpendicularSelector selector,
	in    EdgeSegment           edge,
	in    SignedDistance        dist,
	in    float                 param 
) {
	if (compareSignedDistance(dist, selector.minTrueDistance)) {
		selector.minTrueDistance = dist;
		selector.nearEdge        = edge;
		selector.nearEdgeParam   = param;
	}
}

void addEdgePerpendicularDistance(
	inout PerpendicularSelector selector,
	in    float                 dist
) {
	if (dist <= 0 && dist > selector.minNegativePerpendicularDistance)
		selector.minNegativePerpendicularDistance = dist;
	if (dist >= 0 && dist < selector.minPositivePerpendicularDistance)
		selector.minPositivePerpendicularDistance = dist;
}

void addEdge(
	inout PerpendicularSelector selector,
	in    EdgeSegment           prev_edge,
	in    EdgeSegment           curr_edge,
	in    EdgeSegment           next_edge,
	in    vec2                  p
) {
	float param;
	SignedDistance dist = edgeSDF(curr_edge, p, param);

	addEdgeTrueDistance(selector, curr_edge, dist, param);

	vec2  ap       = p - curr_edge.p0;
	vec2  bp       = p - curr_edge.p1;
	vec2  a_dir    = edgeDirection0(curr_edge);
	vec2  b_dir    = edgeDirection1(curr_edge);
	vec2  prev_dir = edgeDirection1(prev_edge);
	vec2  next_dir = edgeDirection0(next_edge);
	float add      = dot(ap, normalizeAllowZero(prev_dir + a_dir));
	float bdd      = -dot(bp, normalizeAllowZero(b_dir + next_dir));

	if (add > 0) {
		float perp_dist = dist.distance;

		if (getPerpendicularDistance(ap, -a_dir, perp_dist))
			addEdgePerpendicularDistance(selector, perp_dist = -perp_dist);
	}

	if (bdd > 0) {
		float perp_dist = dist.distance;

		if (getPerpendicularDistance(bp, b_dir, perp_dist))
			addEdgePerpendicularDistance(selector, perp_dist);
	}
}

void distanceToPerpendicularDistance(
	in EdgeSegment       edge,
	in vec2              p,
	in float             param,
	inout SignedDistance dist
) {
	if (param < 0) {
		vec2  dir = edgeDirection0(edge);
		vec2  aq  = p - edge.p0;
		float ts  = dot(aq, dir);

		if (ts < 0) {
			float perp_dist = cross2D(aq, dir);

			if (abs(perp_dist) <= abs(dist.distance)) {
				dist.distance = perp_dist;
				dist.dot      = 0;
			}
		}
	} else if (param > 1) {
		vec2  dir = edgeDirection1(edge);
		vec2  bq  = p - edge.p1;
		float ts  = dot(bq, dir);
		
		if (ts > 0) {
			float perp_dist = cross2D(bq, dir);

			if (abs(perp_dist) <= abs(dist.distance)) {
				dist.distance = perp_dist;
				dist.dot      = 0;
			}
		}
	}
}

float computeDistance(in PerpendicularSelector selector, in vec2 p)
{
	float min_dist =
		selector.minTrueDistance.distance < 0.0 ?
		selector.minNegativePerpendicularDistance :
		selector.minPositivePerpendicularDistance;

	if (selector.nearEdge.p0 != vec2(FLT_INF)) {
		SignedDistance dist = selector.minTrueDistance;

		distanceToPerpendicularDistance(selector.nearEdge, p, selector.nearEdgeParam, dist);

		if (abs(dist.distance) < abs(min_dist))
			min_dist = dist.distance;
	}

	return min_dist;
}

void mergePerpendicularSelector(inout PerpendicularSelector dst, in PerpendicularSelector src)
{
	if (compareSignedDistance(src.minTrueDistance, dst.minTrueDistance)) {
		dst.minTrueDistance = src.minTrueDistance;
		dst.nearEdge        = src.nearEdge;
		dst.nearEdgeParam   = src.nearEdgeParam;
	}

	if (src.minNegativePerpendicularDistance > dst.minNegativePerpendicularDistance)
		dst.minNegativePerpendicularDistance = src.minNegativePerpendicularDistance;
	if (src.minPositivePerpendicularDistance < dst.minPositivePerpendicularDistance)
		dst.minPositivePerpendicularDistance = src.minPositivePerpendicularDistance;
}

void writeColor(float dist)
{
	ivec2 coord = ivec2(gl_FragCoord.xy);
	vec4  color = vec4(dist, 0.0, 0.0, 0.0);
	imageStore(glyphTextures[nonuniformEXT(inLayerIndex)], coord, color);
}

void main()
{
	PerpendicularSelector selectors[MAX_CONTOUR_COUNT];
	EdgeSegment           edges[MAX_EDGE_COUNT];
	float                 distances[MAX_CONTOUR_COUNT];
	int                   windings[MAX_CONTOUR_COUNT];
	vec2                  gp;

	vec2 font_coord = inScale * inFontCoord;
	uint point_idx  = inStorageOffset;
	uint cont_count = 0;
	uint iter_count = 0;

	do {
		bool  on_curve;
		float winding_score = 0.0;
		uint  state         = 0;
		uint  edge_count    = 0;

		while (true) {
			gp = storageBuffer.position[point_idx++];

			if (gp.x == FLT_INF) {
				vec2 p0 = edges[edge_count].p0;
				vec2 p2 = edges[0].p0;

				if (state == 1)
					edges[edge_count].p1 = p0;

				edges[edge_count++].p2 = p2;
				winding_score         += shoelace(p0, p2);
				break;
			}

			on_curve = decodeGlyphPoint(gp);
			gp      *= inScale;

			switch (state) {
			case 0: // starting point
				edges[0].p0 = gp;
				state       = 1;
				break;
			case 1: // previous point is on-curve
				if (on_curve) {
					edges[edge_count].p1 = edges[edge_count].p0;
					edges[edge_count].p2 = gp;
					winding_score       += shoelace(edges[edge_count].p0, gp);
					edge_count++;
					edges[edge_count].p0 = gp;
				} else {
					edges[edge_count].p1 = gp;
					state                = 2;
				}
				break;
			case 2: // previous point is off-curve
				if (on_curve) {
					edges[edge_count].p2 = gp;
					winding_score       += shoelace(edges[edge_count].p0, gp);
					edge_count++;
					edges[edge_count].p0 = gp;
					state                = 1;
				} else {
					vec2 p2 = 0.5 * (edges[edge_count].p1 + gp);

					edges[edge_count].p2 = p2;
					winding_score       += shoelace(edges[edge_count].p0, p2);
					edge_count++;
					edges[edge_count].p0 = p2;
					edges[edge_count].p1 = gp;
				}
				break;
			}

			// Safety break to avoid infinite loops in malformed glyphs.
			if (MAX_EDGE_COUNT < iter_count++) return;
		}

		windings[cont_count] = winding_score > 0.0 ? 1 : (winding_score < 0.0 ? -1 : 0);

		uint prev_idx = edge_count >= 2 ? edge_count - 2 : 0;
		uint curr_idx = edge_count - 1;
		uint next_idx = 0;

		resetPerpendicularSelector(selectors[cont_count]);

		for (; next_idx < edge_count; ++next_idx) {
			addEdge(
				selectors[cont_count],
				edges[prev_idx],
				edges[curr_idx],
				edges[next_idx],
				font_coord
			);

			prev_idx = curr_idx;
			curr_idx = next_idx;
		}

		cont_count++;
	} while (gp.y != FLT_INF);

	PerpendicularSelector shape_selector;
	PerpendicularSelector inner_selector;
	PerpendicularSelector outer_selector;

	resetPerpendicularSelector(shape_selector);
	resetPerpendicularSelector(inner_selector);
	resetPerpendicularSelector(outer_selector);

	for (int i = 0; i < cont_count; ++i) {
		distances[i] = computeDistance(selectors[i], font_coord);

		mergePerpendicularSelector(shape_selector, selectors[i]);

		if (windings[i] > 0 && distances[i] >= 0)
			mergePerpendicularSelector(inner_selector, selectors[i]);
		if (windings[i] < 0 && distances[i] <= 0)
			mergePerpendicularSelector(outer_selector, selectors[i]);
	}

	float result_dist = -FLT_MAX;
	float shape_dist  = computeDistance(shape_selector, font_coord);
	float inner_dist  = computeDistance(inner_selector, font_coord);
	float outer_dist  = computeDistance(outer_selector, font_coord);
	int   winding     = 0;

	if (inner_dist >= 0.0 && abs(inner_dist) <= abs(outer_dist)) {
		result_dist = inner_dist;
		winding     = 1;

		for (int i = 0; i < cont_count; ++i) {
			if (windings[i] > 0) {
				float cont_dist = distances[i];
				if (abs(cont_dist) < abs(outer_dist) && cont_dist > result_dist)
					result_dist = cont_dist;
			}
		}
	} else if (outer_dist <= 0.0 && abs(outer_dist) < abs(inner_dist)) {
		result_dist = outer_dist;
		winding     = -1;

		for (int i = 0; i < cont_count; ++i) {
			if (windings[i] < 0) {
				float cont_dist = distances[i];
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
			float cont_dist = distances[i];
			if (cont_dist * result_dist >= 0.0 && abs(cont_dist) < abs(result_dist))
				result_dist = cont_dist;
		}
	}

	writeColor(result_dist);
}
