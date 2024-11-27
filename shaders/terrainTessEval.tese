#version 460 core

// The input to this shader stage will be 4 control points (quad), each vertex will be placed equidistant from each other. 
layout(quads, equal_spacing, ccw, point_mode) in;

// Binding is 0 here because it's a uniform buffer object with binding 0 within the descriptor set layout.
layout(binding = 0) uniform CameraUniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition[];
layout(location = 1) in vec4 inColor[];
layout(location = 2) in vec2 inUv[];

layout(location = 0) out vec4 outColor;

void main() 
{    
    // Necessary if/when point_mode is enabled.
    gl_PointSize = 3.0f; 

    // gl_TessCoord - Barycentric coordinates : The location of a point corresponding to the tessellation patch.
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;  

    vec3 a = mix(inPosition[0], inPosition[1], u);
    vec3 b = mix(inPosition[3], inPosition[2], u);
    gl_Position = vec4(mix(a, b, v), 1.0);

    // Convert the final position into clip space.
    //gl_Position = ubo.proj * ubo.view * vec4(gl_Position.xyz, 1.0);

    outColor = vec4(1.0, 1.0, 1.0, 1.0); // white for visual. 
}