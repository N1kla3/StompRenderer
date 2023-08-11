#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(set = 0, binding = 1) uniform LightBufferObject
{
    vec3 position;
    vec3 ambient;
    float amb_str;
    vec3 diffusive;
    float diff_str;
    vec3 specular;
    float spec_str;
} light;

//push constants block
layout( push_constant ) uniform constants
{
    mat4 model;

    vec4 pushAmbient;
    vec4 pushDiffusive;
    vec4 pushSpecular;

    int id;
} pushModel;

layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 1) uniform sampler2D diffMap;
layout(set = 1, binding = 2) uniform sampler2D specMap;

layout(location = 0) out vec4 outColor;
layout(location = 1) out int outId;

void main()
{
    outColor = texture(texSampler, fragTexCoord) * vec4(fragColor, 1.0f);
    outId = pushModel.id;
}