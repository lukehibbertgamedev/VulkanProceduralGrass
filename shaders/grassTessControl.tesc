#version 460 core

// Tessellation Control Shader

// A quad is used as the base geometry for the representation of a grass blade.
// This stage is used to calculate the tessellation levels, representing the number of segments the quad is divided into.
// The number of segments represents the level of smoothness of the final blade.
// Tessellation will be proportional to the distance from the camera.

// Patches as a quad representation to be sent to the evaluation shader with tessellation information from this control shader. 
layout(vertices = 1) out; 

struct BladeInstanceData {
    
    vec4 p0_and_width;
    vec4 p1_and_height;
    vec4 p2_and_direction;
    vec4 upVec_and_stiffness;             
};

// Use the vertex shaders input of the instance index to get the same blade index.
//BladeInstanceData blade = blades[inInstanceIndex[gl_InvocationID]];
layout(std140, binding = 1) buffer BladeInstanceDataBuffer {
    BladeInstanceData blades[]; 
};

// Inputs from the vertex shader, as arrays since this data will be controlled in patches.
layout(location = 0) in vec4 inColor[];
layout(location = 1) in vec4 inP0[];
layout(location = 2) in vec4 inP1[];
layout(location = 3) in vec4 inP2[];
layout(location = 4) in vec4 inP3[];

layout(location = 5) in vec4 inPosition[];

layout(location = 0) out vec4 outColor[]; // The length of this array will always be the size of the output patch.
layout(location = 1) out vec4 outP0[];
layout(location = 2) out vec4 outP1[];
layout(location = 3) out vec4 outP2[];
layout(location = 4) out vec4 outP3[];

layout(location = 5) out vec4 outPosition[];

#define TESS_LEVEL 5

void main() {

    // Controls how much tessellation to do, that's it. This will be more complex during dynamic tessellation.

    //      3       OL-3        2
    //      .-------------------. 
    //      |       IL-0        | 
    //      |        ___        |       ^
    // OL-0 |  IL-1 |   | IL-1  | OL-2  | 'v'
    //      |       '---'       |       
    //      |        IL-0       |
    //      '-------------------'
    //      0        OL-1       1
    //               -> 'u'
    //
    // U represents new vertex X, V represents new vertex Y...
    // 0 : u=0, v=0
    // 1 : u=1, v=0
    // 2 : u=1, v=1
    // 3 : u=0, v=1 
    
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    
    outP0[gl_InvocationID] = inP0[gl_InvocationID];
    outP1[gl_InvocationID] = inP1[gl_InvocationID];
    outP2[gl_InvocationID] = inP2[gl_InvocationID];
    outP3[gl_InvocationID] = inP3[gl_InvocationID];

    outPosition[gl_InvocationID] = inPosition[gl_InvocationID];

    if (gl_InvocationID == 0) {

        // For isolines:
        //gl_TessLevelOuter[0] = 5; // Number of isolines.
        //gl_TessLevelOuter[1] = 5; // Number of segments per isoline.
        //gl_TessLevelInner[0] = 16; // ???

        // For triangles:
        //gl_TessLevelOuter[0] = 4; // Outer edge left. ???
        //gl_TessLevelOuter[1] = 4; // Outer edge bottom. ???
        //gl_TessLevelOuter[2] = 4; // Outer edge right. ???
        //gl_TessLevelInner[0] = 3; // ???

        // For quads:
        gl_TessLevelOuter[0] = 5; // Segments per left edge + 1 for the endpoints.
        gl_TessLevelOuter[1] = 5; // Segments per top edge + 1 for the endpoints.
        gl_TessLevelOuter[2] = 5; // Segments per right edge + 1 for the endpoints.
        gl_TessLevelOuter[3] = 5; // Segments per bottom edge + 1 for the endpoints.

        gl_TessLevelInner[0] = 5; // Inner connections for the top and bottom edges.
        gl_TessLevelInner[1] = 5; // Inner connections for the left and right edges.
    }

    // Send the output values to the tessellation evaluation shader.
    
    outColor[gl_InvocationID] = inColor[gl_InvocationID]; 
}