#version 450

layout(std140, set = 0, binding = 0) uniform CameraStateBlock
{
    bool flipVertically;
    float nearDistance;
    float farDistance;
    float radiusScale;

    mat4 projectionMatrix;
    mat4 inverseProjectionMatrix;
    mat4 viewMatrix;
    mat4 inverseViewMatrix;
} CameraState;
