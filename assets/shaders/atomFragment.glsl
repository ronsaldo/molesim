#line 2

layout(location=0) flat in uint inAtomIndex;
layout(location=1) in vec2 inTexcoord;
layout(location=2) in vec3 inViewPosition;

layout(location=0) out vec4 outFragColor;

//layout (depth_less) out float gl_FragDepth; // To keep early-z test.

void main()
{
    AtomRenderingState atomState = AtomRenderingStateBuffer[inAtomIndex];
    bool isInside = length(inTexcoord) <= 1.0;
    if(!isInside)
        discard;

    vec3 N = normalize(vec3(inTexcoord, 1.0));
    vec3 V = normalize(-inViewPosition);
    float NdotV = max(0.0, dot(N, V));
    
    //outFragColor = vec4(inTexcoord*0.5 + 0.5, 0.0, 1.0);
    //outFragColor = vec4(N*0.5 + 0.5, 1.0);
    vec4 baseColor = atomState.color;
    outFragColor = vec4(baseColor.rgb*(0.2 + NdotV*0.8), baseColor.a);
}
