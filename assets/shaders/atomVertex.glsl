#line 2

const vec2 quadVertices[4] = vec2[4](
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(1.0, 1.0)
);

void main()
{
    vec3 modelCenter = vec3(0.0, 0.0, 0.0);
    vec3 worldCenter = modelCenter;
    vec3 viewCenter = (CameraState.viewMatrix*vec4(worldCenter, 1.0)).xyz;
    vec3 viewPosition = viewCenter + vec3(quadVertices[gl_VertexIndex], 1.0);

    gl_Position = CameraState.projectionMatrix*vec4(viewPosition, 1.0);
}
