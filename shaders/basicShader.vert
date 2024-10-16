#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

// Output to the tessellation control shader.
layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec4 outColor;

void main() {

    gl_PointSize = 14.0;
    gl_Position = vec4(inPosition.xy, 1.0, 1.0);

    // Pass through values to the TCS.
    outPosition = gl_Position;
    outColor = inColor;
}