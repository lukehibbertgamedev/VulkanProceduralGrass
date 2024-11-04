#version 450

// Tessellation Control Shader

// A quad is used as the base geometry for the representation of a grass blade.
// This stage is used to calculate the tessellation levels, representing the number of segments the quad is divided into.
// The number of segments represents the level of smoothness of the final blade.
// Tessellation will be proportional to the distance from the camera.

// Patches as a triangle representation to be sent to the evaluation shader with tessellation information from this control shader. 
#define PATCH_SIZE 1
layout(vertices = PATCH_SIZE) out; 

struct BladeInstanceData {
    
    vec4 p0_and_width;
    vec4 p1_and_height;
    vec4 p2_and_direction;
    vec4 upVec_and_stiffness;             
};

layout(std140, binding = 1) buffer BladeInstanceDataBuffer {
    BladeInstanceData blades[]; 
};

// Built-in:
// - int gl_PatchVerticesIn: number of vertices per-patch.
// - int gl_PrimitiveID: the index of the current patch.
// - int gl_InvocationID: the index of the control shader invocation, use this to index per-vertex.
// Control shader also takes in all built-in variables output by the vertex shader.

layout(location = 0) in vec4 inColor[];
layout(location = 1) in int inInstanceIndex[];

layout(location = 0) out vec4 outColor[]; // The length of this array will always be the size of the output patch.

void main() {

    int instanceIndex = inInstanceIndex[gl_InvocationID];
    BladeInstanceData blade = blades[instanceIndex];
    
    if (gl_InvocationID == 0) {
        gl_out[gl_InvocationID].gl_Position = vec4(0.0, 0.0, 0.0, 1.0); // Example control point
    } else if (gl_InvocationID == 1) {
        gl_out[gl_InvocationID].gl_Position = vec4(1.0, 0.0, 0.0, 1.0); // Example control point
    } else if (gl_InvocationID == 2) {
        gl_out[gl_InvocationID].gl_Position = vec4(0.0, 1.0, 0.0, 1.0); // Example control point
    }

    // Set tessellation levels if necessary
    if (gl_InvocationID == 0) {
        gl_TessLevelInner[0] = 1.0f; // Set inner tessellation level
        gl_TessLevelOuter[0] = 1.0f; // Set outer level for edge 0
        gl_TessLevelOuter[1] = 1.0f; // Set outer level for edge 1
        gl_TessLevelOuter[2] = 1.0f; // Set outer level for edge 2
    }

    // This passes the control points within the patch (the generated vertices) to the evaluation shader.
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    outColor[gl_InvocationID] = inColor[gl_InvocationID];
}