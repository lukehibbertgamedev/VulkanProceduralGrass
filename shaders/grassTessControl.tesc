#version 450

// Tessellation Control Shader

// A quad is used as the base geometry for the representation of a grass blade.
// This stage is used to calculate the tessellation levels, representing the number of segments the quad is divided into.
// The number of segments represents the level of smoothness of the final blade.
// Tessellation will be proportional to the distance from the camera.

// Patches as a triangle representation to be sent to the evaluation shader with tessellation information from this control shader. 
#define PATCH_SIZE 1
layout(vertices = PATCH_SIZE) out; 

// Built-in:
// - int gl_PatchVerticesIn: number of vertices per-patch.
// - int gl_PrimitiveID: the index of the current patch.
// - int gl_InvocationID: the index of the control shader invocation, use this to index per-vertex.
// Control shader also takes in all built-in variables output by the vertex shader.

layout(location = 0) in vec4 inColor[];

layout(location = 0) out vec4 outColor[]; // The length of this array will always be the size of the output patch.

void main() {

    // Set control points and pass data to the next stage
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    // Invocation ID of 0 controls tessellation levels for the entire patch, and this shader is ran once per-patch for each control point in the patch (patch_size).
    //if (gl_InvocationID == 0) {
        
        gl_TessLevelOuter[0] = 1.0f;
        gl_TessLevelOuter[1] = 1.0f;
        //gl_TessLevelOuter[2] = 2.0f;
        //gl_TessLevelOuter[3] = 2.0f;

        gl_TessLevelInner[0] = 1.0f;
        //gl_TessLevelInner[1] = 1.0f;
    //}

    // This passes the control points within the patch (the generated vertices) to the evaluation shader.
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    outColor[gl_InvocationID] = inColor[gl_InvocationID];
}