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

//push constants block
layout( push_constant ) uniform constants
{
    mat4 model;

    vec3 pushAmbient;
    vec3 pushDiffusive;
    vec3 pushSpecular;
} pushModel;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 ambient = pushModel.pushAmbient * light.ambient * vec3(light.amb_str);

    // diffuse
    vec3 norm = normalize(outNormal);
    vec3 lightDir = normalize(light.position - outPosition);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = vec3(light.diff_str) * (diff * pushModel.pushDiffusive) * vec3(texture(diffMap, fragTexCoord));

    // specular
    vec3 viewDir = normalize(outViewPosition - outPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = vec3(light.spec_str) * pushModel.pushSpecular * (spec * vec3(texture(specMap, fragTexCoord)));

    vec3 result = (ambient + diffuse + specular) * fragColor;
    outColor = texture(texSampler, fragTexCoord) * vec4(result, 1.0f);
}