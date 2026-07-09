#line 2

layout(location = 0) out vec3 nearPoint;
layout(location = 1) out vec3 farPoint;

// Infinite grid drawing. See https://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/.

const vec2 clipQuadVertices[4] = vec2[4](
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(1.0, 1.0)
);

vec3 unprojectPoint(vec3 point)
{
    vec4 unprojected = CameraState.inverseViewMatrix * (CameraState.inverseProjectionMatrix * vec4(point, 1.0));
    return unprojected.xyz / unprojected.w;
}

void main()
{
    vec2 clipPoint = clipQuadVertices[gl_VertexIndex];
    nearPoint = unprojectPoint(vec3(clipPoint, 1.0));
    farPoint = unprojectPoint(vec3(clipPoint, 0.0));
    gl_Position = vec4(clipPoint, 0.0, 1.0);
}
