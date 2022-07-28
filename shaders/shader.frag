#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 1) uniform LightBufferObject
{
    vec3 position;
    vec3 ambient;
    float amb_str;
    vec3 diffusive;
    float diff_str;
    vec3 specular;
    float spec_str;
} light;

layout(binding = 2) uniform sampler2D texSampler;
layout(binding = 3) uniform sampler2D diffMap;
layout(binding = 4) uniform sampler2D specMap;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(texSampler, fragTexCoord) * vec4(fragColor, 1.0f);
}