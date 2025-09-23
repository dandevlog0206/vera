#version 450

struct Test {
    int meber0;
    float meber1;
};

// Specialization Constants
layout(constant_id=17) const int Spec_Constant = 0;

// Push Constants
layout(push_constant) uniform BlockName1 {
    int member1;
    int member2[5];
    float member3;
    vec2 member4;
    vec4 member5;
    mat3x3 member6;
    mat3x4 member7;
    Test member8;
    Test member9[];
} Push_Constant;

// Samplers
layout (set=0, binding=0) uniform sampler Sampler;

// Combined Texture + Samplers
layout (set=0, binding=1) uniform sampler2D Combined_Image_Sampler;

// Textures (Sampled Images)
layout (set=0, binding=2) uniform texture2D Sampled_Image[10];
layout (set=0, binding=3) uniform texture2D Sampled_Image2[];

// Storage Images
layout (set=1, binding=0, r32f) uniform image2D Storage_Image;

// Texture Buffers (Uniform Texel Buffers)
layout (set=1, binding=1) uniform textureBuffer Uniform_Texel_Buffer;

// Image Buffers (Storage Texel Buffers)
layout (set=1, binding=2, r32f) uniform imageBuffer Storage_Texel_Buffer;

// Uniform Buffers
layout (std140, set=2, binding=0) uniform BlockName2
{
    vec4 myElement_uniform[32];
    vec4 myElement_uniform1;
    vec4 myElement_uniform2[][3];
} Uniform_Buffer;

// Storage Buffers
layout (set=2, binding=1) buffer BlockName3
{
    vec4 myElement_storage[];
} Storage_Buffer;

void main()
{
    gl_Position = vec4(0, 0, 0, 1);
}