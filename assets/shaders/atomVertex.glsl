#line 2

const vec2 quadVertices[4] = vec2[4](
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(1.0, 1.0)
);

void main()
{
    vec3 worldPosition = vec3(quadVertices[gl_VertexIndex], 1.0);
    vec4 viewPosition4 = CameraState.viewMatrix*vec4(worldPosition, 1.0);
    gl_Position = CameraState.projectionMatrix*viewPosition4;
}
