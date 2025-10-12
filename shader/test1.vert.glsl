#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

/*
 * This is a complex test vertex shader.
 * It uses push constants and a uniform buffer.
 * It shares descriptor set 1 with the fragment shader.
 */

// Vertex attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_tex_coord;

// Outputs to fragment shader
layout(location = 0) out vec3 frag_world_pos;
layout(location = 1) out vec3 frag_normal;
layout(location = 2) out vec2 frag_tex_coord;
layout(location = 3) out flat int frag_material_idx;

// Push constants block - shared with fragment shader
layout(push_constant) uniform PushConstantBlock {
    mat4 model;
    int material_idx;
} u_pc;

// Set 0: Per-frame camera data
layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    vec3 view_pos;
} u_camera;

// Set 1: Shared resource set (also used in fragment shader)
// This vertex shader doesn't directly use the resources in set 1,
// but it declares the layout to ensure compatibility with the pipeline layout.
// The actual data is used in the fragment shader.
layout(set = 2, binding = 0) uniform MaterialUBO {
    vec4 diffuse_color;
    float specular_power;
} u_materials[];


void main() {
    // Transform position and normal to world space
    mat4 model_view = u_camera.view * u_pc.model;
    vec4 world_pos = u_pc.model * vec4(in_position, 1.0);

    frag_world_pos = world_pos.xyz;
    // Inverse transpose of the model matrix for transforming normals
    frag_normal = transpose(inverse(mat3(u_pc.model))) * in_normal;
    
    // Pass through texture coordinates and material index
    frag_tex_coord = in_tex_coord;
    frag_material_idx = u_pc.material_idx;

    // Final vertex position in clip space
    gl_Position = u_camera.proj * u_camera.view * world_pos;
}