#version 450
// Intricate test-purpose vertex shader with:
// - optional skinning, morph targets, instancing
// - procedural FBM displacement (simplex noise)
// - TBN construction, view vector, fog, and per-vertex color modulation
// Toggle features with -DUSE_SKINNING, -DUSE_MORPH, -DUSE_INSTANCING at compile time.

#define USE_SKINNING

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent; // xyz = tangent, w = bitangent sign
layout(location = 3) in vec2 inUV;
layout(location = 4) in vec4 inColor;

#ifdef USE_SKINNING
layout(location = 5) in uvec4 inJoints; // joint indices
layout(location = 6) in vec4  inWeights; // up to 4 weights
#endif

#ifdef USE_MORPH
layout(location = 7) in vec3 inPositionMorph;
layout(location = 8) in vec3 inNormalMorph;
layout(location = 9) in float inMorphWeight;
#endif

#ifdef USE_INSTANCING
layout(location = 10) in mat4 inInstanceModel; // per-instance model matrix
#endif

// Uniforms
layout(std140, binding = 0) uniform PerFrame {
    mat4 uView;
    mat4 uProj;
    vec3 uCamPos;
    float uTime;
} PerFrameUBO;

layout(std140, binding = 1) uniform PerObject {
    mat4 uModel;      // base model matrix
    mat3 uNormalMat;  // model-space normal matrix (inverse-transpose of upper-left 3x3)
} PerObjectUBO;

#ifdef USE_SKINNING
const int MAX_BONES = 128; // adjust as needed
layout(std140, binding = 2) uniform Bones {
    mat4 uBones[MAX_BONES];
} BonesUBO;
#endif

// Outputs to fragment shader
layout(location = 0) out vec3 vWorldPos;
layout(location = 1) out vec3 vNormal;
layout(location = 2) out vec2 vUV;
layout(location = 3) out vec3 vViewDir;
layout(location = 4) out vec3 vTangent;
layout(location = 5) out vec4 vColor;
layout(location = 6) out float vFogFactor;

// ----------------------------- Helper noise (Simplex) -----------------------------
// Ashima 3D simplex noise (slightly adapted, compacted for vertex-side use)
vec4 mod289(vec4 x){ return x - floor(x * (1.0/289.0)) * 289.0; }
vec3 mod289(vec3 x){ return x - floor(x * (1.0/289.0)) * 289.0; }
vec4 permute(vec4 x){ return mod289(((x*34.0)+1.0)*x); }
float snoise(vec3 v)
{
    const vec2 C = vec2(1.0/6.0, 1.0/3.0);
    const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);
    // First corner
    vec3 i  = floor(v + dot(v, C.yyy));
    vec3 x0 = v - i + dot(i, C.xxx);
    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min( g.xyz, l.zxy );
    vec3 i2 = max( g.xyz, l.zxy );
    //  x0, x1, x2, x3
    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy;
    vec3 x3 = x0 - D.yyy;
    // Permutations
    i = mod289(i);
    vec4 p = permute( permute( permute(
                i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
              + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
              + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));
    // Gradients
    float n_ = 1.0/7.0;
    vec3 ns = n_ * D.wyz - D.xzx;
    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);
    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_);
    vec4 x = x_ * ns.x + ns.y;
    vec4 y = y_ * ns.x + ns.y;
    vec4 h = 1.0 - abs(x) - abs(y);
    vec4 b0 = vec4( x.xy, y.xy );
    vec4 b1 = vec4( x.zw, y.zw );
    vec4 s0 = floor(b0)*2.0 + 1.0;
    vec4 s1 = floor(b1)*2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));
    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy;
    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww;
    vec3 g0 = vec3(a0.x, a0.y, h.x);
    vec3 g1 = vec3(a0.z, a0.w, h.y);
    vec3 g2 = vec3(a1.x, a1.y, h.z);
    vec3 g3 = vec3(a1.z, a1.w, h.w);
    // Normalise gradients
    vec4 norm = 1.79284291400159 - 0.85373472095314 *
        vec4(dot(g0,g0), dot(g1,g1), dot(g2,g2), dot(g3,g3));
    g0 *= norm.x;
    g1 *= norm.y;
    g2 *= norm.z;
    g3 *= norm.w;
    // Mix contributions
    vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
    m = m * m;
    return 42.0 * dot( m*m, vec4(dot(g0,x0), dot(g1,x1), dot(g2,x2), dot(g3,x3)) );
}

float fbm(vec3 p)
{
    float f = 0.0;
    float w = 0.5;
    for(int i=0;i<4;i++){
        f += w * snoise(p);
        p *= 2.0;
        w *= 0.5;
    }
    return f;
}

// ----------------------------- Main -----------------------------
void main()
{
    // Base attributes
    vec3 pos = inPosition;
    vec3 nrm = inNormal;
    vec3 tan = inTangent.xyz;
    float binSign = inTangent.w;
    vec2 uv = inUV;
    vec4 color = inColor;

    // Apply morph target if present
    #ifdef USE_MORPH
    pos = mix(pos, inPositionMorph, inMorphWeight);
    nrm = normalize(mix(nrm, inNormalMorph, inMorphWeight));
    #endif

    // Skinning: compute skin matrix and transform position/normal/tangent
    #ifdef USE_SKINNING
    mat4 skinMat = mat4(0.0);
    skinMat += inWeights.x * BonesUBO.uBones[inJoints.x];
    skinMat += inWeights.y * BonesUBO.uBones[inJoints.y];
    skinMat += inWeights.z * BonesUBO.uBones[inJoints.z];
    skinMat += inWeights.w * BonesUBO.uBones[inJoints.w];
    vec4 skPos = skinMat * vec4(pos, 1.0);
    pos = skPos.xyz;
    // For normals/tangents transform by inverse-transpose of skinMat's upper-left 3x3
    mat3 skinNormalMat = mat3(skinMat);
    nrm = normalize(skinNormalMat * nrm);
    tan = normalize(skinNormalMat * tan);
    #endif

    // Instance model override
    mat4 modelMat = PerObjectUBO.uModel;
    #ifdef USE_INSTANCING
    modelMat = inInstanceModel * PerObjectUBO.uModel;
    #endif

    // Procedural displacement using FBM noise along normal (intricate, time-varying)
    float baseFreq = 1.6;
    float amp = 0.12;
    vec3 noiseSamplePos = (modelMat * vec4(pos,1.0)).xyz * baseFreq + vec3(0.0, PerFrameUBO.uTime * 0.2, 0.0);
    float displacement = fbm(noiseSamplePos) * amp;
    // Use a second layer to add fine detail
    displacement += 0.03 * snoise(noiseSamplePos * 8.0 + PerFrameUBO.uTime * 0.5);
    pos += nrm * displacement;

    // Compute world-space position and normal
    vec3 worldPos = (modelMat * vec4(pos, 1.0)).xyz;
    vec3 worldNormal = normalize(PerObjectUBO.uNormalMat * nrm);
    vec3 worldTangent = normalize(PerObjectUBO.uNormalMat * tan);
    // Recompute bitangent using tangent.w as sign
    vec3 bitangent = cross(worldNormal, worldTangent) * binSign;

    // View direction
    vec3 viewDir = normalize(PerFrameUBO.uCamPos - worldPos);

    // TBN orthonormalization (Gram-Schmidt)
    worldTangent = normalize(worldTangent - worldNormal * dot(worldNormal, worldTangent));
    bitangent = normalize(cross(worldNormal, worldTangent) * binSign);

    // Fog factor: simple exponential based on distance
    float dist = length(PerFrameUBO.uCamPos - worldPos);
    float fogStart = 10.0;
    float fogEnd = 80.0;
    float fog = clamp((dist - fogStart) / (fogEnd - fogStart), 0.0, 1.0);

    // Color modulation: vertex color + subtle albedo variation from noise
    float colorVariation = 0.05 * snoise(worldPos * 2.5 + vec3(PerFrameUBO.uTime * 0.1));
    vec4 outColor = color * vec4(1.0 + colorVariation, 1.0 + colorVariation, 1.0 + colorVariation, 1.0);

    // Output varyings
    vWorldPos = worldPos;
    vNormal = worldNormal;
    vUV = uv;
    vViewDir = viewDir;
    vTangent = worldTangent;
    vColor = outColor;
    vFogFactor = fog;

    // Final clip-space position
    vec4 viewPos = PerFrameUBO.uView * vec4(worldPos, 1.0);
    gl_Position = PerFrameUBO.uProj * viewPos;

    // Optional: tweak gl_Position.z slightly for logarithmic depth or depth bias if required
}