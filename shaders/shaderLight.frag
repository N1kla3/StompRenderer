#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 outNormal;
layout(location = 3) in vec3 outPosition;

layout(binding = 1) uniform LightBufferObject
{
    vec3 position;
    vec3 color;
} light;

layout(binding = 2) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main()
{

    float ambientStr = 0.7f;
    vec3 ambient = ambientStr * light.color;

    vec3 normal = normalize(outNormal);
    vec3 lightDirection = normalize(light.position - outPosition);
    float diff = max(dot(normal, lightDirection), 0.0f);
    vec3 diffusive = diff * light.color;

    vec3 result = (diffusive + ambient);
    outColor = texture(texSampler, fragTexCoord) * vec4(result, 1.0f);
}