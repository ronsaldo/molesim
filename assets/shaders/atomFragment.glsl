#line 2

layout(location=0) flat in uint inAtomIndex;
layout(location=1) in vec3 inViewPosition;

layout(location=0) out vec4 outFragColor;

void main()
{
    outFragColor = AtomRenderingStateBuffer[inAtomIndex].color;
}
