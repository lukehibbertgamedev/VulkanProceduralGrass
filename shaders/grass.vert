#version 460 core

// Note: The up vector for a grass blade can be calculated from the normalised vector pointing from p0 to p1.
// Note: The vector along the width can be calculated by combining the angle of direction with its up vector.

// A shader copy of the BladeInstanceData structure defined in GrassBlade.h.
// This contains all instances of all grass blades, indexed by gl_InstanceIndex.
struct BladeInstanceData {
    
    vec4 p0_and_width;
    vec4 p1_and_height;
    vec4 p2_and_direction;
    vec4 upVec_and_stiffness;             
};

// Binding is 0 here because it's a uniform buffer object with binding 0 within the descriptor set layout.
layout(binding = 0) uniform CameraUniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

// Binding is 1 here because it's a shader storage buffer object with binding 1 within the descriptor set layout.
// The shader storage buffer binding for the data buffer, populated by VulkanApplication::populateBladeInstanceBuffer().
layout(std140, binding = 1) buffer BladeInstanceDataBuffer {
    BladeInstanceData blades[]; 
};

layout(location = 0) out vec4 outColor; 
layout(location = 1) out vec4 outP0_Width;
layout(location = 2) out vec4 outP1_Height;
layout(location = 3) out vec4 outP2_Direction;

void main() {   

    // Set the point size for a visual on-screen.
    gl_PointSize = 12.0;

    // Get access to the instance data using the instance index.
    // gl_InstanceIndex provides the index of the current instance being processed when doing some form of instanced rendering.
    BladeInstanceData blade = blades[gl_InstanceIndex];     

    gl_Position = vec4(blade.p0_and_width.xyz, 1.0); // World position of the grass blade (bottom centre of the quad).

    outP0_Width = blade.p0_and_width;
    outP1_Height = blade.p1_and_height;
    outP2_Direction = blade.p2_and_direction;
    outColor = vec4(0.0f, 1.0f, 0.0f, 1.0f); 
} 