#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 outNormal;
layout(location = 3) in vec3 outPosition;
layout(location = 4) in vec3 outViewPosition;

layout(binding = 1) uniform LightBufferObject
{
    vec3 position;
    vec3 color;
} light;

layout(binding = 2) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main()
{
    float ambientStr = 0.1f;
    vec3 ambient = ambientStr * light.color;

    // diffuse
    vec3 norm = normalize(outNormal);
    vec3 lightDir = normalize(light.position - outPosition);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light.color;

    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(outViewPosition - outPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = specularStrength * spec * light.color;

    vec3 result = (ambient + diffuse + specular) * fragColor;
    outColor = texture(texSampler, fragTexCoord) * vec4(result, 1.0f);
}