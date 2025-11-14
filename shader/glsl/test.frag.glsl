#version 450
// Intricate test-purpose fragment shader:
// - Normal mapping (TBN)
// - Simple Cook-Torrance PBR (GGX + Smith + Schlick)
// - Albedo/ORM texture support (occlusion, roughness, metallic)
// - Wireframe overlay via barycentric coordinates
// - Fog, tone-mapping and gamma correction
// - Debug output (normal visualization) in second render target

// Inputs (locations mirror vertex/geometry outputs)
layout(location = 0) in vec3 vWorldPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUV;
layout(location = 3) in vec3 vViewDir;
layout(location = 4) in vec3 vTangent;
layout(location = 5) in vec4 vColor;
layout(location = 6) in float vFogFactor;
layout(location = 7) in vec3 gBarycentric; // for wireframe overlay

// Outputs
layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outDebug; // normals / debug

// Uniforms: reuse PerFrame and PerObject UBOs (must match vertex layout)
layout(std140, binding = 0) uniform PerFrame {
    mat4 uView;
    mat4 uProj;
    vec3 uCamPos;
    float uTime;
} PerFrameUBO;

layout(std140, binding = 1) uniform PerObject {
    mat4 uModel;
    mat3 uNormalMat;
} PerObjectUBO;

// Texture bindings (optional in test scenes)
layout(binding = 3) uniform sampler2D uAlbedoMap;   // RGB albedo
layout(binding = 4) uniform sampler2D uNormalMap;   // RGB normal (tangent-space)
layout(binding = 5) uniform sampler2D uORMMap;      // R=occlusion, G=roughness, B=metallic

// Lighting UBO (simple single directional light + ambient)
layout(std140, binding = 6) uniform LightUBO {
    vec4 lightDir;    // xyz = direction (world), w unused
    vec4 lightColor;  // rgb = color, a = intensity
    vec4 ambient;     // rgb ambient, a unused
} Light;

// ---------- PBR helpers ----------
const float PI = 3.14159265359;

float saturate(float x) { return clamp(x, 0.0, 1.0); }
vec3  saturate(vec3 x)  { return clamp(x, vec3(0.0), vec3(1.0)); }

// GGX / Trowbridge-Reitz Normal Distribution
float D_GGX(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom + 1e-6);
}

// Smith geometry (Schlick-GGX)
float G_Schlick_GGX(float NdotV, float k)
{
    return NdotV / (NdotV * (1.0 - k) + k + 1e-6);
}
float G_Smith(float NdotV, float NdotL, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return G_Schlick_GGX(NdotV, k) * G_Schlick_GGX(NdotL, k);
}

// Fresnel Schlick
vec3 F_Schlick(vec3 F0, float cosTheta)
{
    return F0 + (vec3(1.0) - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

// ACES filmic tone mapping (compact)
vec3 ACESFilm(vec3 x)
{
    const vec3 a = vec3(2.51, 2.51, 2.51);
    const vec3 b = vec3(0.03, 0.03, 0.03);
    const vec3 c = vec3(2.43, 2.43, 2.43);
    const vec3 d = vec3(0.59, 0.59, 0.59);
    const vec3 e = vec3(0.14, 0.14, 0.14);
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// Reconstruct TBN matrix from interpolated tangent + normal
mat3 computeTBN(vec3 N, vec3 T)
{
    vec3 tangent = normalize(T - N * dot(N, T));
    vec3 bitangent = cross(N, tangent);
    return mat3(tangent, bitangent, N);
}

// Wireframe edge factor using barycentric coordinates
float edgeFactor(vec3 bary)
{
    vec3 d = fwidth(bary);
    vec3 a3 = smoothstep(vec3(0.0), d * 1.5, bary);
    return min(min(a3.x, a3.y), a3.z);
}

// Sample textures with safe defaults
vec3 sampleAlbedo(vec2 uv)
{
    #ifdef GL_ES
    return texture(uAlbedoMap, uv).rgb;
    #else
    // If textures are unbound this still samples; engine should bind defaults.
    return texture(uAlbedoMap, uv).rgb;
    #endif
}
vec3 sampleNormalMap(vec2 uv)
{
    vec3 n = texture(uNormalMap, uv).rgb;
    // normal map in [0,1] -> [-1,1]
    return n * 2.0 - 1.0;
}
vec3 sampleORM(vec2 uv)
{
    vec3 v = texture(uORMMap, uv).rgb;
    return v;
}

// ---------- Main ----------
void main()
{
    // Base material values
    vec3 baseColor = vColor.rgb;
    float alpha = vColor.a;

    // Sample albedo if available and mix
    vec3 texAlbedo = sampleAlbedo(vUV);
    // fallback: if texture is fully black, prefer vertex color
    if (length(texAlbedo) > 0.001)
        baseColor *= texAlbedo;

    // ORM: occlusion, roughness, metallic
    vec3 orm = sampleORM(vUV);
    float occlusion = orm.r;                // default 1 if texture missing
    float roughness = saturate(orm.g);
    float metallic  = saturate(orm.b);

    // Normal mapping: reconstruct TBN and transform normal
    vec3 n_tangent = sampleNormalMap(vUV);
    mat3 TBN = computeTBN(normalize(vNormal), normalize(vTangent));
    vec3 N = normalize(TBN * n_tangent);
    // fallback if normal map is neutral (0,0,1) then N==vNormal in world space
    if (length(n_tangent) < 0.001) N = normalize(vNormal);

    vec3 V = normalize(vViewDir);
    vec3 L = normalize(-Light.lightDir.xyz); // Light.lightDir expected pointing from surface to light
    vec3 H = normalize(L + V);

    float NdotL = saturate(max(dot(N, L), 0.0));
    float NdotV = saturate(max(dot(N, V), 0.0));
    float NdotH = saturate(max(dot(N, H), 0.0));
    float VdotH = saturate(max(dot(V, H), 0.0));

    // Fresnel base reflectivity (non-metal -> typically 0.04)
    vec3 F0 = mix(vec3(0.04), baseColor, metallic);

    // Cook-Torrance BRDF
    float D = D_GGX(NdotH, roughness);
    float G = G_Smith(NdotV, NdotL, roughness);
    vec3 F = F_Schlick(F0, VdotH);

    vec3 specular = (D * G * F) / (4.0 * NdotV * NdotL + 1e-6);
    // diffuse (Lambert) scaled by (1 - metallic)
    vec3 kd = (vec3(1.0) - F) * (1.0 - metallic);

    // Lighting: single directional light + ambient
    vec3 radiance = Light.lightColor.rgb * Light.lightColor.a; // color * intensity
    vec3 Lo = (kd * baseColor / PI + specular) * radiance * NdotL;

    vec3 ambient = Light.ambient.rgb * baseColor * 0.5;

    // Ambient occlusion
    Lo *= mix(1.0, occlusion, 0.5);

    vec3 colorLinear = ambient + Lo;

    // Tone mapping and gamma
    vec3 mapped = ACESFilm(colorLinear);
    vec3 colorSRGB = pow(mapped, vec3(1.0 / 2.2));

    // Wireframe overlay: darken edges slightly
    float edge = edgeFactor(gBarycentric);
    float line = 1.0 - edge; // 1 at edges, 0 in center
    float lineThickness = 0.12; // tweak for visibility
    float lineMask = smoothstep(0.0, lineThickness, line);
    vec3 lineColor = vec3(0.04, 0.08, 0.12); // dark bluish lines

    vec3 finalColor = mix(lineColor, colorSRGB, lineMask);

    // Fog blend (using vFogFactor from vertex shader)
    vec3 fogColor = vec3(0.7, 0.75, 0.8);
    finalColor = mix(finalColor, fogColor, vFogFactor);

    outColor = vec4(finalColor, alpha);

    // Debug output: packed normal visualization + roughness in alpha
    outDebug = vec4(normalize(N) * 0.5 + 0.5, roughness);
}