#version 460 core

// Configure the control shader to output a single vertex per-patch. This single vertex after tessellation becomes a quad, aligned to a Bézier curve.
layout(vertices = 1) out; 

// Inputs from the vertex shader, as arrays since this data will be controlled in patches.
layout(location = 0) in vec4 inColor[];
layout(location = 1) in vec4 inPosition[];
layout(location = 2) in float inBladeWidth[];
layout(location = 3) in float inBladeHeight[];

// The length of all output arrays will be equal to the patch size.
layout(location = 0) out vec4 outColor[]; 
layout(location = 1) out vec4 outPosition[];
layout(location = 2) out float outBladeWidth[];
layout(location = 3) out float outBladeHeight[];

#define TESS_LEVEL 6
#define NO_TESS 1

void main() {
    
    // Passthrough data, the evaluation shader needs these.
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    outPosition[gl_InvocationID] = inPosition[gl_InvocationID];
    outBladeWidth[gl_InvocationID] = inBladeWidth[gl_InvocationID];
    outBladeHeight[gl_InvocationID] = inBladeHeight[gl_InvocationID];

    if (gl_InvocationID == 0) {

        // For quads:
        gl_TessLevelOuter[0] = TESS_LEVEL;      // Segments per left edge + 1 for the endpoints.
        gl_TessLevelOuter[1] = NO_TESS;         // Segments per top edge + 1 for the endpoints.
        gl_TessLevelOuter[2] = TESS_LEVEL;      // Segments per right edge + 1 for the endpoints.
        gl_TessLevelOuter[3] = NO_TESS;         // Segments per bottom edge + 1 for the endpoints.
    }

    outColor[gl_InvocationID] = inColor[gl_InvocationID]; 
}