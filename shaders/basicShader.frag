#version 460 core

layout(location = 0) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = inColor;

    //outColor = mix(vec4(0.0, 0.2, 0.0, 1.0), vec4(0.0, 0.4, 0.0, 1.0), inColor.z);
}