#version 460 core

// The uniform buffer binding for the model, view, and projection matrices to be used by objects with a model.
layout(set = 0, binding = 0) uniform CameraUniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColour; 
layout(location = 2) in vec2 inUv;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec4 outColor;
layout(location = 2) out vec2 outUv; 

void main() {    
    gl_PointSize = 8.0;
    //gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

    const vec4 planeColour = vec4(0.120, 0.075, 0.017, 1.0);

    gl_Position = vec4(inPosition, 1.0);
    outPosition = gl_Position.xyz;
    outColor =  planeColour; // Colour of plane model.
    outUv = inUv;
}