#version 450

layout(push_constant) uniform PCBlock {
    float time;
    float scale;
    vec4  colors[3];
} pc;

layout(location = 0) out vec4 fragColor;

struct Vertex
{
    vec2 pos;
    vec4 color;
};

Vertex vertices[3] = {
    {{ 0.f, -0.5f }, { 1.f, 0.f, 0.f, 1.f }},
    {{ -0.5f, 0.5f }, { 0.f, 1.f, 0.f, 1.f }},
    {{ 0.5f, 0.5f }, { 0.f, 0.f, 1.f, 1.f }},
};

void main() {
    gl_Position = vec4(pc.scale * vertices[gl_VertexIndex].pos, 0.0, 1.0);
    fragColor   = pc.colors[gl_VertexIndex];
}