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

layout(std140, set = 1, binding = 0) uniform ModelStateBlock
{
    mat4 modelMatrix;
    mat4 inverseModelMatrix;
} ModelState;

struct AtomRenderingState
{
    vec3 position;
    float radius;
    vec4 color;
};

layout(std430, set = 1, binding = 1) buffer AtomRenderingStateBufferBlock
{
    AtomRenderingState AtomRenderingStateBuffer[];
};
