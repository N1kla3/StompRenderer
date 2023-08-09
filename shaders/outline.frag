#version 450

layout (location = 0) out vec4 outFragColor;
layout (location = 1) out int outId;

void main()
{
    outFragColor = vec4(0.7, 0.2, 0.3, 1.0);
    outId = 0;
}