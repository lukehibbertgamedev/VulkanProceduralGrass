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
layout(location = 1) out vec4 outP0;
layout(location = 2) out vec4 outP1;
layout(location = 3) out vec4 outP2;
layout(location = 4) out float outBladeWidth;
layout(location = 5) out float outBladeHeight;
layout(location = 6) out float outBladeDirection;

void main() {   

    // Set the point size for a visual on-screen.
    gl_PointSize = 12.0;

    // Get access to the instance data using the instance index.
    // gl_InstanceIndex provides the index of the current instance being processed when doing some form of instanced rendering.
    BladeInstanceData blade = blades[gl_InstanceIndex];       
    
    // Transform world position to clip space.
    vec4 clippedP0 = ubo.proj * ubo.view * vec4(blade.p0_and_width.xyz, 1.0);
    vec4 clippedP1 = ubo.proj * ubo.view * vec4(blade.p1_and_height.xyz, 1.0);
    vec4 clippedP2 = ubo.proj * ubo.view * vec4(blade.p2_and_direction.xyz, 1.0);

    gl_Position = clippedP0; // Send through the bottom centre position of the grass blade, vertices will be generated from here.

    outP0 = clippedP0;
    outP1 = clippedP1;
    outP2 = clippedP2;
    outBladeWidth = blade.p0_and_width.w;
    outBladeHeight = blade.p1_and_height.w;
    outBladeDirection = blade.p2_and_direction.w;
    outColor = vec4(0.0f, 1.0f, 0.0f, 1.0f); 
} 