#version 460 core

// The uniform buffer binding for the model, view, and projection matrices to be used by objects with a model.
layout(binding = 0) uniform CameraUniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColour; 

layout(location = 0) out vec4 outColor; 

void main() {    
    gl_PointSize = 7.0;
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

    const vec4 planeColour = vec4(0.239, 0.149, 0.035, 1.0);

    outColor =  planeColour; // Colour of plane model.
}