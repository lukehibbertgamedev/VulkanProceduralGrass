#version 460 core

// Configure the control shader to output a single vertex per-patch. This single vertex after tessellation becomes a quad, aligned to a Bézier curve.
layout(vertices = 4) out; 

// Inputs from the vertex shader, as arrays since this data will be controlled in patches.
layout(location = 0) in vec3 inPosition[];
layout(location = 1) in vec4 inColor[];
layout(location = 2) in vec2 inUv[];

layout(location = 0) out vec3 outPosition[]; 
layout(location = 1) out vec4 outColor[];
layout(location = 2) out vec2 outUv[];

// A sampler to sample the height map texture.
layout(set = 0, binding = 1) uniform sampler2D heightMapSampler;

#define TESS_LEVEL 2
#define NO_TESS 2

void main() {
    
    // Passthrough data, the evaluation shader needs these.
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    outPosition[gl_InvocationID] = inPosition[gl_InvocationID];
    outColor[gl_InvocationID] = inColor[gl_InvocationID];

    if (gl_InvocationID == 0) {

        // For quads:
        gl_TessLevelOuter[0] = TESS_LEVEL;      // Segments per left edge + 1 for the endpoints.
        gl_TessLevelOuter[1] = NO_TESS;         // Segments per top edge + 1 for the endpoints.
        gl_TessLevelOuter[2] = TESS_LEVEL;      // Segments per right edge + 1 for the endpoints.
        gl_TessLevelOuter[3] = NO_TESS;         // Segments per bottom edge + 1 for the endpoints.

        gl_TessLevelInner[0] = NO_TESS;
        gl_TessLevelInner[1] = TESS_LEVEL;
    }
}