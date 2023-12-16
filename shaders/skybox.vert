#version 450

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec3 inColor;

layout(set = 0, binding = 0) uniform UBO
{
    mat4 model;
    mat4 projection;
    mat4 view;
} ubo;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragTexCoord;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = ubo.projection * ubo.view * inPos;
    fragColor = inColor;
    fragTexCoord = inPos.xyz;
}
