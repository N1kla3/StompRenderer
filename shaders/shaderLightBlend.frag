#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 outNormal;
layout(location = 3) in vec3 outPosition;
layout(location = 4) in vec3 outViewPosition;

struct LightBufferObject
{
    vec3 position;
    float direction_flag;
    vec3 ambient;
    float amb_str;
    vec3 diffusive;
    float diff_str;
    vec3 specular;
    float spec_str;
};

struct PointLightBuffer
{
    vec3 position;
    float unused;

    vec3 ambient;
    float amb_str;
    vec3 diffusive;
    float diff_str;
    vec3 specular;
    float spec_str;

    float constant;
    float linear;
    float quadratic;
    float unusedd;
};

struct SpotLightBuffer
{
    vec3 position;float unused_position;

    vec3 ambient;float amb_str;
    vec3 diffusive;float diff_str;
    vec3 specular;float spec_str;

    vec3 direction;float unused_direction;

    float cut_off;
    float outer_cutoff;
    float constant;
    float linear;

    vec3 unused;
    float quadratic;
};

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 view;
    mat4 proj;
    vec3 viewPosition;

    int global_size;
    int point_size;
    int spot_size;
} ubo;

layout(set = 0, binding = 1) uniform GlobalLightProxy
{
    LightBufferObject object;
} light_global;
layout(set = 0, binding = 2) readonly buffer PointLightProxy
{
    PointLightBuffer[] object;
} point_light;
layout(set = 0, binding = 3) readonly buffer SpotLightProxy
{
    SpotLightBuffer[] object;
} spot_light;

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform sampler2D diffMap;
layout(set = 1, binding = 2) uniform sampler2D specMap;

//push constants block
layout( push_constant ) uniform constants
{
    mat4 model;

    vec4 pushAmbient;
    vec4 pushDiffusive;
    vec4 pushSpecular;

    int id;
} pushModel;

layout(location = 0) out vec4 outColor;
layout(location = 1) out int outId;

vec3 calcDirLight(LightBufferObject light, vec3 normal, vec3 viewDir);
vec3 calcPointLight(PointLightBuffer light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcSpotLight(SpotLightBuffer light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 norm = normalize(outNormal);
    vec3 viewDir = normalize(outViewPosition - outPosition);

    vec4 result = texture(texSampler, fragTexCoord);
    if (result.a < 0.1)
    {
        discard;
    }

    vec3 new_res = result.xyz;
    if (bool(ubo.global_size))
    {
        new_res = calcDirLight(light_global.object, norm, viewDir);
    }
    for (int i = 0; i < ubo.point_size; i++)
    {
        new_res += calcPointLight(point_light.object[i], norm, outPosition, viewDir);
    }
    for (int i = 0; i < ubo.spot_size; i++)
    {
        new_res += calcSpotLight(spot_light.object[i], norm, outPosition, viewDir);
    }

    //result *= fragColor;
    outColor = vec4(new_res, result.a);
    outId = pushModel.id;
}

vec3 calcDirLight(LightBufferObject light, vec3 normal, vec3 viewDir)
{
    vec3 LightDir = normalize(light.position - outPosition);
    float diff = max(dot(normal, LightDir), 0.0);
    vec3 reflectDir = reflect(-LightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);

    vec3 ambient = light.ambient * pushModel.pushAmbient.xyz * vec3(texture(texSampler, fragTexCoord));
    vec3 diffuse = light.diffusive * diff * pushModel.pushDiffusive.xyz * vec3(texture(diffMap, fragTexCoord));
    vec3 specular = light.specular * spec * pushModel.pushSpecular.xyz * vec3(texture(specMap, fragTexCoord));

    return ambient + diffuse + specular;
}

vec3 calcPointLight(PointLightBuffer light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 LightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, LightDir), 0.0);
    vec3 reflectDir = reflect(-LightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * pushModel.pushAmbient.xyz * vec3(texture(texSampler, fragTexCoord));
    vec3 diffuse = light.diffusive * diff * pushModel.pushDiffusive.xyz * vec3(texture(diffMap, fragTexCoord));
    vec3 specular = light.specular * spec * pushModel.pushSpecular.xyz * vec3(texture(specMap, fragTexCoord));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 calcSpotLight(SpotLightBuffer light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 LightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, LightDir), 0.0);
    vec3 reflectDir = reflect(-LightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    float theta = dot(LightDir, normalize(-light.direction));
    float epsilon = light.cut_off - light.outer_cutoff;
    float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);

    vec3 ambient = light.ambient * pushModel.pushAmbient.xyz * vec3(texture(texSampler, fragTexCoord));
    vec3 diffuse = light.diffusive * diff * pushModel.pushDiffusive.xyz * vec3(texture(diffMap, fragTexCoord));
    vec3 specular = light.specular * spec * pushModel.pushSpecular.xyz * vec3(texture(specMap, fragTexCoord));

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}
