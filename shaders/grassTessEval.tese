#version 460 core

// The input to this shader stage will be 4 control points (quad), each vertex will be placed equidistant from each other. 
layout(quads, equal_spacing, ccw) in;

layout(location = 0) in vec4 inColor[];
layout(location = 1) in vec4 inPosition[];
layout(location = 2) in float inBladeWidth[];
layout(location = 3) in float inBladeHeight[];

layout(location = 0) out vec4 outColor;

void main() 
{    
    // Necessary when point_mode is enabled.
    // gl_PointSize = 2.0f; 

    // gl_TessCoord - Barycentric coordinates : The location of a point corresponding to the tessellation patch.
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // Position the generated vertices in a quad-like shape, raised by the blade height. TODO: RAISE BY BLADE HEIGHT.
    vec4 p0 = inPosition[0] + vec4(-inBladeWidth[0], -inBladeHeight[0], 0.0, 0.0);     // Top left vertex.
    vec4 p1 = inPosition[0] + vec4(inBladeWidth[0], -inBladeHeight[0], 0.0, 0.0);      // Top right vertex.
    vec4 p2 = inPosition[0] + vec4(inBladeWidth[0], inBladeHeight[0], 0.0, 0.0);       // Bottom left vertex.
    vec4 p3 = inPosition[0] + vec4(-inBladeWidth[0], inBladeHeight[0], 0.0, 0.0);      // Bottom right vertex.

    // Interpolate between the points to correctly position the generated vertices on a per-quad data basis.
    vec4 a = mix(p0, p1, u);
    vec4 b = mix(p3, p2, u);
    gl_Position = mix(a, b, v);

    float height = clamp(gl_Position.y, 0.0, 1.0);

    vec4 baseBladeColour = vec4(0.522, 0.322, 0.11, 1.0); // Brown
    vec4 tipBladeColour = vec4(0.271, 0.812, 0.263, 1.0); // Green

    outColor = inColor[0]; 
}