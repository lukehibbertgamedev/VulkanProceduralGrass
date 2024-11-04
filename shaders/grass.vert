#version 450

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

// The shader storage buffer binding for the data buffer, populated by VulkanApplication::populateBladeInstanceBuffer().
layout(std140, binding = 1) buffer BladeInstanceDataBuffer {
    BladeInstanceData blades[]; 
};

layout(location = 0) out vec4 outColor; 

void main() {   

    // Get access to the instance data using the instance index.
    // gl_InstanceIndex provides the index of the current instance being processed.
    BladeInstanceData blade = blades[gl_InstanceIndex];

    // Transform world position to clip space.
    gl_Position = ubo.proj * ubo.view * vec4(blade.p0_and_width.xyz, 1.0f);

    // Set the point size for a visual on-screen.
    gl_PointSize = 2.0;

    outColor = vec4(1.0f, 0.0f, 0.0f, 1.0f); 
}