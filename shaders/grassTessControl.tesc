#version 460 core

// Tessellation Control Shader

// A quad is used as the base geometry for the representation of a grass blade.
// This stage is used to calculate the tessellation levels, representing the number of segments the quad is divided into.
// The number of segments represents the level of smoothness of the final blade.
// Tessellation will be proportional to the distance from the camera.

// Patches as a quad representation to be sent to the evaluation shader with tessellation information from this control shader. 
layout(vertices = 4) out; 

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
layout(location = 1) in int inInstanceIndex[];
layout(location = 2) in vec4 inPosition[];

layout(location = 0) out vec4 outColor[]; // The length of this array will always be the size of the output patch.
layout(location = 1) out vec4 outPosition[];

void main() {

    // Controls how much tessellation to do, that's it. This will be more complex during dynamic tessellation.

    //              OL-3
    //      .-------------------. 
    //      |       IL-0        | 
    //      |        ___        |
    // OL-0 |  IL-1 |   | IL-1  | OL-2
    //      |       '---'       |
    //      |        IL-0       |
    //      '-------------------'
    //               OL-1

    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = 1.0f; // Left edge.
        gl_TessLevelOuter[1] = 1.0f; // Bottom edge.
        gl_TessLevelOuter[2] = 1.0f; // Right edge.
        gl_TessLevelOuter[3] = 1.0f; // Top edge.

        gl_TessLevelInner[0] = 1.0f; // Top and bottom internal edges.
        gl_TessLevelInner[1] = 1.0f; // Left and right internal edges.
    }
    
    // Send the output values to the tessellation evaluation shader.
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    outColor[gl_InvocationID] = inColor[gl_InvocationID]; 
}