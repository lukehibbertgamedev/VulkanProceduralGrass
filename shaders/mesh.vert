#version 450

// The uniform buffer binding for the model, view, and projection matrices to be used by objects with a model.
layout(binding = 0) uniform CameraUniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;

void main() {    
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
}