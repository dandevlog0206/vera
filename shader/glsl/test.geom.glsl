#version 450
// Geometry shader that forwards per-vertex varyings and injects barycentric
// coordinates for wireframe/debug rendering. Emits a triangle_strip with three
// vertices so fragment shader can compute smooth wireframe edges.

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

// Inputs from vertex shader (locations must match vertex outputs)
layout(location = 0) in vec3 vWorldPos[];
layout(location = 1) in vec3 vNormal[];
layout(location = 2) in vec2 vUV[];
layout(location = 3) in vec3 vViewDir[];
layout(location = 4) in vec3 vTangent[];
layout(location = 5) in vec4 vColor[];
layout(location = 6) in float vFogFactor[];

// Outputs to fragment shader (reuse same locations)
layout(location = 0) out vec3 gWorldPos;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec2 gUV;
layout(location = 3) out vec3 gViewDir;
layout(location = 4) out vec3 gTangent;
layout(location = 5) out vec4 gColor;
layout(location = 6) out float gFogFactor;

// Extra output: barycentric coordinates for wireframe; location 7 is free.
layout(location = 7) out vec3 gBarycentric;

void main()
{
    // Emit the triangle with per-vertex barycentric coords (1,0,0), (0,1,0), (0,0,1)
    for (int i = 0; i < 3; ++i) {
        gWorldPos     = vWorldPos[i];
        gNormal       = normalize(vNormal[i]);
        gUV           = vUV[i];
        gViewDir      = normalize(vViewDir[i]);
        gTangent      = normalize(vTangent[i]);
        gColor        = vColor[i];
        gFogFactor    = vFogFactor[i];

        if (i == 0) gBarycentric = vec3(1.0, 0.0, 0.0);
        else if (i == 1) gBarycentric = vec3(0.0, 1.0, 0.0);
        else gBarycentric = vec3(0.0, 0.0, 1.0);

        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
