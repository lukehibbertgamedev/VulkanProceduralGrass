#version 450

// A shader copy of the BladeInstanceData structure defined in GrassBlade.h.
struct BladeInstanceData {
    vec3 worldPosition;
    float width;         
    float height;        
    float stiffness;     
    float directionAngle; 
    float lean;             
};

// The shader storage buffer binding for the data buffer, populated by VulkanApplication::populateBladeInstanceBuffer().
layout(set = 0, binding = 1) buffer BladeInstanceDataBuffer {
    BladeInstanceData blades[]; 
};

// An index to the current instance of blade to be processed.
layout(location = 0) in uint instanceIndex;

void main() {

    // Set the point size for a visual on-screen.
    gl_PointSize = 7.0;

    // Get access to the instance data using the instance index.
    BladeInstanceData blade = blades[instanceIndex];
    gl_Position = vec4(blade.worldPosition.xyz, 1.0f);
}