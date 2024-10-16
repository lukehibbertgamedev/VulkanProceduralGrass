#version 450

// Tessellation evaluation shader.
// Evaluate the intermediate point to generate a new vertex point.

// Evaluation shader must provide the patch_type. 
// Requires (patch_type, subdivision_spacing, winding_order)
// Patch type           : Triangle / Quads / Isolines
// Subdivision spacing  : equal_spacing / fractional_odd_spacing / fractional_even spacing: Create an even number of subdivisions broken into long and short segments.
// Winding order        : Clockwise (cw) / Counter-clockwise (ccw)
layout (triangles, fractional_odd_spacing, cw) in;

// Input from the tessellation control shader.
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

// Specify the number of vertices per patch.
layout (vertices = 3) out;

// Output to the tessellation evaluation shader.
layout (location = 0) out vec3 outPosition;
layout (location = 1) out vec4 outColor;

void main() {    

}