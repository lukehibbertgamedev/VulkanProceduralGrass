#version 450

// Tessellation Control Shader

// A quad is used as the base geometry for the representation of a grass blade.
// This stage is used to calculate the tessellation levels, representing the number of segments the quad is divided into.
// The number of segments represents the level of smoothness of the final blade.
// Tessellation will be proportional to the distance from the camera.

// Patches as a triangle representation to be sent to the evaluation shader with tessellation information from this control shader. 
#define PATCH_SIZE 3
layout(vertices = PATCH_SIZE) out; 

struct BladeInstanceData {
    
    vec4 p0_and_width;
    vec4 p1_and_height;
    vec4 p2_and_direction;
    vec4 upVec_and_stiffness;             
};

// Built-in:
// - int gl_PatchVerticesIn: number of vertices per-patch.
// - int gl_PrimitiveID: the index of the current patch.
// - int gl_InvocationID: the index of the control shader invocation, use this to index per-vertex.
// Control shader also takes in all built-in variables output by the vertex shader.

layout(location = 0) in vec4 inColor[];
layout(location = 1) in int inInstanceIndex[];
layout(location = 2) in vec4 inPosition[];

layout(location = 0) out vec4 outColor[]; // The length of this array will always be the size of the output patch.
layout(location = 1) out vec4 outPosition[];

layout(std140, binding = 1) buffer BladeInstanceDataBuffer {
    BladeInstanceData blades[]; 
};

void main() {

    int instanceIndex = inInstanceIndex[gl_InvocationID];
    BladeInstanceData blade = blades[instanceIndex];

    // Assign control points based on BladeInstanceData
    if (gl_InvocationID == 0) {
        gl_out[gl_InvocationID].gl_Position = blade.p0_and_width; // Base position
    } else if (gl_InvocationID == 1) {
        gl_out[gl_InvocationID].gl_Position = blade.p1_and_height; // Mid position
    } else if (gl_InvocationID == 2) {
        gl_out[gl_InvocationID].gl_Position = blade.p2_and_direction; // Tip position
    }

    // Set tessellation levels if necessary
        gl_TessLevelInner[0] = 2.0; // Adjust for inner tessellation

        gl_TessLevelOuter[0] = 2.0; // Outer level for edge 0
        gl_TessLevelOuter[1] = 2.0; // Outer level for edge 1
        gl_TessLevelOuter[2] = 2.0; // Outer level for edge 2

    outColor[gl_InvocationID] = inColor[gl_InvocationID]; // Pass color to the TES
    outPosition[gl_InvocationID] = gl_out[gl_InvocationID].gl_Position;
}