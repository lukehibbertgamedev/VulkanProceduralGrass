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

layout(location = 0) in vec3 inPosition; // For the quad mesh data.
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor; 
layout(location = 1) out int outInstanceIndex;

void main() {   

    // Get access to the instance data using the instance index.
    // gl_InstanceIndex provides the index of the current instance being processed.
    BladeInstanceData blade = blades[gl_InstanceIndex];

    // Note: The up vector for a grass blade can be calculated from the normalised vector pointing from p0 to p1.

    // Note: The vector along the width can be calculated by combining the angle of direction with its up vector.
    // vec3 tmp = [sin(direction), sin(direction) + cos(direction), cos(direction)] / ||[sin(direction), sin(direction) + cos(direction), cos(direction)]||
    // vec3 widthV = (up cross tmp) / ||(up cross tmp)||

    // Transform world position to clip space.
    gl_Position = ubo.proj * ubo.view * vec4(blade.p0_and_width.xyz, 1.0f);

    // Set the point size for a visual on-screen.
    gl_PointSize = 5.0;

    outColor = vec4(1.0f, 0.0f, 0.0f, 1.0f); 
    outInstanceIndex = gl_InstanceIndex;
}