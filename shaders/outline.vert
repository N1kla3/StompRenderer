#version 450

layout (location = 0) in vec4 inPos;

layout(set = 0, binding = 0) uniform UBO
{
    mat4 model;
    mat4 projection;
    mat4 view;
} ubo;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = ubo.projection * ubo.view * ubo.model * inPos;
}
