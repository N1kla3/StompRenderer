#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 view;
    mat4 proj;
    vec3 viewPosition;

    int global_size;
    int point_size;
    int spot_size;
} ubo;

//push constants block
layout( push_constant ) uniform constants
{
    mat4 model;

    vec4 pushAmbient;
    vec4 pushDiffusive;
    vec4 pushSpecular;

    int id;
} pushModel;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 outPosition;
layout(location = 4) out vec3 outViewPosition;

void main()
{
    gl_Position = ubo.proj * ubo.view * pushModel.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    outNormal = inNormal;
    outPosition = vec3(pushModel.model * vec4(inPosition, 1.0));
    outViewPosition = ubo.viewPosition;
}