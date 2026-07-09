#line 2

layout(location = 0) flat out uint outAtomIndex;
layout(location = 1) out vec3 outViewPosition;

const vec2 quadVertices[4] = vec2[4](
    vec2(-1.0, -1.0),
    vec2(-1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(1.0, 1.0)
);

void main()
{
    // Pass the instance index
    outAtomIndex = gl_InstanceIndex;

    vec3 atomPosition = AtomRenderingStateBuffer[gl_InstanceIndex].position;
    float atomRadius = AtomRenderingStateBuffer[gl_InstanceIndex].radius;

    vec3 modelCenter = atomPosition;
    vec3 worldCenter = (ModelState.modelMatrix *vec4(modelCenter, 1.0)).xyz;
    vec3 viewCenter = (CameraState.viewMatrix*vec4(worldCenter, 1.0)).xyz;
    vec3 viewPosition = viewCenter + vec3(quadVertices[gl_VertexIndex], 1.0)*atomRadius;

    gl_Position = CameraState.projectionMatrix*vec4(viewPosition, 1.0);
}
