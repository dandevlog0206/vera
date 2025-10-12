#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier : enable

/*
 * This is a complex test fragment shader.
 * It uses:
 * - An unsized array of sampled images (textures).
 * - An unsized array of uniform buffers (materials).
 * - Push constants shared with the vertex shader.
 * - A shared descriptor set (set 1) with the vertex shader.
 */

// Inputs from vertex shader
layout(location = 0) in vec3 frag_world_pos;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec2 frag_tex_coord;
layout(location = 3) in flat int frag_material_idx;

// Final output color
layout(location = 0) out vec4 out_color;

// Push constants block - shared with vertex shader
layout(push_constant) uniform PushConstantBlock {
    mat4 model;
    int material_idx; // Used to index into the material UBO array
} u_pc;

// Set 0: Per-frame camera data
layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    vec3 view_pos;
} u_camera;

// Set 1: Shared resource set
// Unsized array of textures, requires VK_EXT_descriptor_indexing
layout(set = 1, binding = 0) uniform sampler2D u_textures[]; 
// Unsized array of materials
layout(set = 2, binding = 0) uniform MaterialUBO {
    vec4 diffuse_color;
    float specular_power;
} u_materials[];

// A fixed light for basic lighting
const vec3 LIGHT_POS = vec3(10.0, 10.0, 10.0);
const vec3 LIGHT_COLOR = vec3(1.0, 1.0, 1.0);
const float AMBIENT_STRENGTH = 0.2;

void main() {
    // Use the material index from push constants to get material properties
    // This demonstrates using push constants to index into a UBO array.
    vec4 material_diffuse = u_materials[u_pc.material_idx].diffuse_color;
    float material_specular_power = u_materials[u_pc.material_idx].specular_power;

    // Use the same index to select a texture from the unsized texture array.
    // The 'nonuniformEXT' qualifier is crucial for dynamically indexing descriptor arrays.
    vec4 tex_color = texture(u_textures[nonuniformEXT(u_pc.material_idx)], frag_tex_coord);

    // Combine material and texture color
    vec3 base_color = material_diffuse.rgb * tex_color.rgb;

    // Basic Blinn-Phong lighting
    // Ambient
    vec3 ambient = AMBIENT_STRENGTH * LIGHT_COLOR;

    // Diffuse
    vec3 norm = normalize(frag_normal);
    vec3 light_dir = normalize(LIGHT_POS - frag_world_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * LIGHT_COLOR;

    // Specular
    vec3 view_dir = normalize(u_camera.view_pos - frag_world_pos);
    vec3 halfway_dir = normalize(light_dir + view_dir);
    float spec_angle = max(dot(norm, halfway_dir), 0.0);
    float specular_strength = pow(spec_angle, material_specular_power);
    vec3 specular = specular_strength * LIGHT_COLOR;

    // Final color calculation
    vec3 final_color = (ambient + diffuse + specular) * base_color;
    out_color = vec4(final_color, material_diffuse.a);
}