#version 460 core

// Configure the control shader to output a single vertex per-patch. This single vertex after tessellation becomes a quad, aligned to a Bézier curve.
layout(vertices = 1) out; 

layout(binding = 0) uniform CameraUniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

// Inputs from the vertex shader, as arrays since this data will be controlled in patches.
layout(location = 0) in vec4 inColor[];
layout(location = 1) in vec4 inP0_Width[];
layout(location = 2) in vec4 inP1_Height[];
layout(location = 3) in vec4 inP2_Direction[];

// The length of all output arrays will be equal to the patch size.
layout(location = 0) out vec4 outColor[]; 
layout(location = 1) out vec4 outP0_Width[];
layout(location = 2) out vec4 outP1_Height[];
layout(location = 3) out vec4 outP2_Direction[];

#define TESS_LEVEL 12
#define NO_TESS 2

float calculateTessellationLevel(in vec3 cameraPosition, in vec3 p0, in vec3 p1, in vec3 p2, in float maxLevel, in float minLevel, in float maxDistance) {

    vec3 centreOfPoints = (p0 + p1 + p2) / 3.0;
    float dist = length(cameraPosition - centreOfPoints);

    float tessellationLevel = maxLevel * (1.0 - dist / maxDistance);    // Further tessellated less.
    //tessellationLevel = maxLevel * (dist / maxDistance);                // Closer tessellated less (testing).

    return clamp(tessellationLevel, minLevel, maxLevel);
}

void main() {
    
    // Passthrough data, the evaluation shader needs these.
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    outP0_Width[gl_InvocationID] = inP0_Width[gl_InvocationID];
    outP1_Height[gl_InvocationID] = inP1_Height[gl_InvocationID];
    outP2_Direction[gl_InvocationID] = inP2_Direction[gl_InvocationID];
    
    // Extract the camera's position from the view matrix.
    vec4 pos = inverse(ubo.view)[3];

    float minLevel = 2.0;
    float maxLevel = 32.0;
    float maxDistance = 40.0;
    float tessellationLevel = calculateTessellationLevel(pos.xyz, 
        inP0_Width[gl_InvocationID].xyz, inP1_Height[gl_InvocationID].xyz, inP2_Direction[gl_InvocationID].xyz, 
        maxLevel, minLevel, maxDistance);

    if (gl_InvocationID == 0) {

        // For quads:
        gl_TessLevelOuter[0] = tessellationLevel;      // Segments per left edge + 1 for the endpoints.
        gl_TessLevelOuter[1] = NO_TESS;         // Segments per top edge + 1 for the endpoints.
        gl_TessLevelOuter[2] = tessellationLevel;      // Segments per right edge + 1 for the endpoints.
        gl_TessLevelOuter[3] = NO_TESS;         // Segments per bottom edge + 1 for the endpoints.

        gl_TessLevelInner[0] = NO_TESS;
        gl_TessLevelInner[1] = tessellationLevel;
    }

    outColor[gl_InvocationID] = inColor[gl_InvocationID]; 
}