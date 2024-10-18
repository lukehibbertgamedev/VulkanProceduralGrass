#version 450

// Tessellation evaluation shader.
// Evaluate the intermediate point to generate a new vertex point.

// Evaluation shader must provide the patch_type. 
// Requires (patch_type, subdivision_spacing, winding_order)
// Patch type           : Triangle / Quads / Isolines
// Subdivision spacing  : equal_spacing / fractional_odd_spacing / fractional_even spacing
// Winding order        : Clockwise (cw) / Counter-clockwise (ccw)
layout (triangles, equal_spacing, ccw) in;

// Input from the tessellation control shader.
layout(location = 0) in vec2 inPosition[];
layout(location = 1) in vec4 inColor[];

// Output to the tessellation evaluation shader.
layout (location = 0) out vec2 outPosition;
layout (location = 1) out vec4 outColor;

void main() {    

    // Interpolate vertex positions and colors
    vec2 pos = 
        gl_TessCoord.x * inPosition[0] +
        gl_TessCoord.y * inPosition[1];

    vec4 color = 
        gl_TessCoord.x * inColor[0] +
        gl_TessCoord.y * inColor[1] +
        gl_TessCoord.z * inColor[2];

    outPosition = pos;
    outColor = color;

    // Output position
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
}