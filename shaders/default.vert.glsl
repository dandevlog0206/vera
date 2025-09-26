#version 450

layout(push_constant) uniform PCBlock
{
    float time;
    float scale;
    vec4  colors[3];
} pc;

layout(location=0) in vec2 inPos;
layout(location=1) in vec4 inColor;
layout(location=2) in vec2 inUV;

layout(location=0) out vec4 outColor;
layout(location=1) out vec2 outUV;

void main()
{
    gl_Position = vec4(pc.scale * inPos, 0.0, 1.0);
    outColor = inColor;
    outUV    = inUV;
}