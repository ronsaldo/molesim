#line 2

layout(location=0) flat in uint inAtomIndex;
layout(location=1) in vec3 inViewPosition;
layout(location=2) in vec3 inViewCenter;

layout(location=0) out vec4 outFragColor;

layout (depth_less) out float gl_FragDepth; // To keep early-z test.

bool raySphereTest(float sphereRadius, in vec3 sphereCenter, in vec3 rayDirection, out vec2 lambdas)
{
    // Ray sphere intersection formula from: https://viclw17.github.io/2018/07/16/raytracing-ray-sphere-intersection
    vec3 rayOriginSphereCenter = vec3(0.0) - sphereCenter;
    float a = dot(rayDirection, rayDirection);
    float b = 2.0 * dot(rayDirection, rayOriginSphereCenter);
    float c = dot(rayOriginSphereCenter, rayOriginSphereCenter) - sphereRadius*sphereRadius;
    float delta = b*b - 4.0*a*c;
    if (delta < 0.0)
        return false;

	float deltaSqrt = sqrt(delta);
    lambdas = vec2(-b - deltaSqrt, -b + deltaSqrt) / (2.0*a);

    return true;
}

void main()
{
    AtomRenderingState atomState = AtomRenderingStateBuffer[inAtomIndex];
    vec3 D = normalize(inViewPosition);

    vec2 lambdas;
    //raySphereTest(desc.radius, viewCenter, D, lambdas);
    bool inside = raySphereTest(atomState.radius, inViewCenter, D, lambdas);
    if (!inside)
        discard;

    // Compute the intersection point.
    vec3 P = D*min(lambdas.x, lambdas.y);

    // Compute the intersection point depth.
    vec4 clipPosition = CameraState.projectionMatrix * vec4(P, 1.0);
    gl_FragDepth = clipPosition.z / clipPosition.w;

    // Compute the normal at the intersection point.
    vec3 N = normalize(P - inViewCenter);
    vec3 V = -D;

    float NdotV = max(0.0, dot(N, V));
    //outFragColor = vec4(D*0.5 + 0.5, 1.0); 
    //outFragColor = atomState.color;
    vec4 baseColor = atomState.color;
    outFragColor = vec4(baseColor.rgb*(0.2 + NdotV*0.8), baseColor.a);
}
