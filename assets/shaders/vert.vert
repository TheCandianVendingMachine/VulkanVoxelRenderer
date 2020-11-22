#version 450
#extension GL_KHR_vulkan_glsl : enable
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform modelViewProjection
    {
        mat4 model;
        mat4 view;
        mat4 projection;
    } ubo;
    
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColour;
layout(location = 2) in vec2 inTextureCoordinate;

layout(location = 0) out vec3 fragColour;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragColour = inColour;
    fragTexCoord = inTextureCoordinate;
}
