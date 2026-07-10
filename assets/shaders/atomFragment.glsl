#line 2

layout(location=0) flat in uint inAtomIndex;
layout(location=1) in vec2 inTexcoord;
layout(location=2) in vec3 inViewPosition;
layout(location=3) in vec3 inViewCenter;

layout(location=0) out vec4 outFragColor;

layout (depth_less) out float gl_FragDepth; // To keep early-z test.

void main()
{
    AtomRenderingState atomState = AtomRenderingStateBuffer[inAtomIndex];

    // Impostor formula from https://paroj.github.io/gltut/Illumination/Tutorial%2013.html
    float texcoordLength = length(inTexcoord);
    bool isInside = texcoordLength <= 1.0;
    if(!isInside)
        discard;

    // Normal and position.
    vec3 N = vec3(inTexcoord, sqrt(1.0 - texcoordLength));
    vec3 P = N * atomState.radius + inViewCenter;

    // Compute the intersection point depth.
    vec4 clipPosition = CameraState.projectionMatrix * vec4(P, 1.0);
    gl_FragDepth = clipPosition.z / clipPosition.w;

    // NdotV
    vec3 V = normalize(-inViewPosition);
    float NdotV = max(0.0, dot(N, V));
    
    //outFragColor = vec4(inTexcoord*0.5 + 0.5, 0.0, 1.0);
    //outFragColor = vec4(N*0.5 + 0.5, 1.0);
    vec4 baseColor = atomState.color;
    outFragColor = vec4(baseColor.rgb*(0.2 + NdotV*0.8), baseColor.a);
}
