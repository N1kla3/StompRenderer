#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragTexCoord;

layout(set = 0, binding = 1) uniform samplerCube texSampler;

layout (location = 0) out vec4 outFragColor;

void main()
{
    outFragColor = texture(texSampler, fragTexCoord);
}
