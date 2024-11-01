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

// The w component of these vector4s represent the float value on the end (i.e., p0_Width -> xyz = p0, w = width).
layout(location = 0) in vec4 p0_Width;
layout(location = 1) in vec4 p1_Height;
layout(location = 2) in vec4 p2_DirectionAngle;
layout(location = 3) in vec4 up_Stiffness;

layout(location = 0) out vec4 outColor; 

void main() {   

    // Get access to the instance data using the instance index.
    // gl_InstanceIndex provides the index of the current instance being processed.
    BladeInstanceData blade = blades[gl_InstanceIndex];

    // Transform world position to clip space.
    gl_Position = ubo.proj * ubo.view * vec4(blade.worldPosition, 1.0f);
    //gl_Position = vec4(0.f, 0.f, 0.f, 1.0f);
    // Set the point size for a visual on-screen.
    gl_PointSize = 5.0;

    outColor = vec4(0.0f, 1.0f, 0.0f, 1.0f); 
}