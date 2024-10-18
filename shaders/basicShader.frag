#version 450

layout(location = 0) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main() {

    vec2 coord = gl_PointCoord - vec2(0.5);
    outColor = vec4(inColor.rgb, 0.5 - length(coord));
}