#line 2

layout(location = 0) in vec3 nearPoint;
layout(location = 1) in vec3 farPoint;

layout(location = 0) out vec4 outColor;

// Infinite grid drawing. See https://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/.
vec4 grid(vec3 fragPos3D, float scale)
{
    vec2 coordinate = fragPos3D.xz * scale;
    vec2 derivative = fwidth(coordinate);
    vec2 grid = abs(fract(coordinate -0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);

    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);

    vec4 color = vec4(0.2, 0.2, 0.2, 1.0);

    // Z axis
    if(fragPos3D.x > -0.1 * minimumx && fragPos3D.x < 0.1 * minimumx)
        color.z = 1.0;

    // X axis
    if(fragPos3D.z > -0.1 * minimumz && fragPos3D.z < 0.1 * minimumz)
        color.x = 1.0;

    color *= 1.0 - min(line, 1.0);
    return color;
}

float computeDepth(vec3 pos)
{
    vec4 clipPosition = CameraState.projectionMatrix * (CameraState.viewMatrix*vec4(pos, 1.0));
    return clipPosition.z / clipPosition.w;
}


void main()
{
    float t = -nearPoint.y / (farPoint.y - nearPoint.y);
    if(t < 0)
        discard;
    
    vec3 fragPos3D = nearPoint + t*(farPoint - nearPoint);
    gl_FragDepth = computeDepth(fragPos3D);

    outColor = grid(fragPos3D, 10);
}