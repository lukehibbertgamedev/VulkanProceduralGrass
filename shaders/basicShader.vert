#version 450

#define MESH_INSTANCE_COUNT 14

layout(binding = 0) uniform CameraUniformBufferObject {
    mat4 model[MESH_INSTANCE_COUNT];
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

// Output to the tessellation control shader.
layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec4 outColor;

void main() {

    gl_PointSize = 7.0;
    //gl_Position = vec4(inPosition.xy, 1.0, 1.0);
    gl_Position = ubo.proj * ubo.view * ubo.model[gl_InstanceIndex] * vec4(inPosition, 1.0);

    // Pass through values to the TCS.
    outPosition = gl_Position.xyz;
    outColor = inColor;
}