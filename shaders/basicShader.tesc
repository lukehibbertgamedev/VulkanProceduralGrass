#version 450

// Tessellation control shader.
// Controls how much tessellation/subdivision a particular patch (group of vertex data) gets. 
// Also defines the size of a patch, allowing it to modify data.
// Its main purpose is to feed the tessellation levels to the tessellation primitive generator stage, as
// well as feed patch data (as output) to the tessellation evaluation shader.

// Define patch size (also determines number of TCS invocations used to compute this patch data).
layout(vertices = 3) out;

// Input from the vertex shader.
layout(location = 0) in vec3 inPosition[];
layout(location = 1) in vec4 inColor[];

// Output to the tessellation evaluation shader.
layout (location = 0) out vec3 outPosition[];
layout (location = 1) out vec4 outColor[];

void main() {

    if (gl_InvocationID == 0) {
        gl_TessLevelInner[0] = 3.0;
        gl_TessLevelOuter[0] = 3.0;
        gl_TessLevelOuter[1] = 3.0;
        gl_TessLevelOuter[2] = 3.0;
    }

    // Passing attributes through.
    outPosition[gl_InvocationID] = inPosition[gl_InvocationID];
    outColor[gl_InvocationID] = inColor[gl_InvocationID];

}