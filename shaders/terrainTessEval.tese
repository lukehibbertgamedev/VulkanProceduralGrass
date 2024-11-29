#version 460 core

// The input to this shader stage will be 4 control points (quad), each vertex will be placed equidistant from each other. 
layout(quads, equal_spacing, ccw) in;

// Binding is 0 here because it's a uniform buffer object with binding 0 within the descriptor set layout.
layout(binding = 0) uniform CameraUniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

// A sampler to sample the height map texture.
layout(binding = 1) uniform sampler2D heightMapSampler;

layout(location = 0) in vec3 inPosition[];
layout(location = 1) in vec4 inColor[];
layout(location = 2) in vec2 inUv[];

layout(location = 0) out vec4 outColor;

void main() 
{    
    // Necessary if/when point_mode is enabled.
    gl_PointSize = 8.0f; 

    // gl_TessCoord - Barycentric coordinates : The location of a point corresponding to the tessellation patch.
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;  

    vec4 p0 = vec4(inPosition[0], 1.0);
    vec4 p1 = vec4(inPosition[1], 1.0);
    vec4 p2 = vec4(inPosition[2], 1.0);
    vec4 p3 = vec4(inPosition[3], 1.0);

    float zOffset = 1.0;
    float zScale = 1.5;

    // Interpolate UV coordinates
	vec2 uv1 = mix(inUv[0], inUv[1], u);
	vec2 uv2 = mix(inUv[3], inUv[2], u);
	vec2 outUV = mix(uv1, uv2, v);
    float height = texture(heightMapSampler, outUV).r * 64.0; //- zOffset;

    // Interpolate generated vertices' positions per-triangle and displace terrain height.
    gl_Position = (gl_TessCoord.x * p0) + (gl_TessCoord.y * p1) + (gl_TessCoord.z * p2);
    gl_Position.z = height;

    // Convert the final position into clip space.
    // Ensure the final position is passed as a homogeneous coordinate here.
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(gl_Position.xyz, 1.0); // Matrix transformations go here if necessary.

    outColor = inColor[0] * (abs(height) + 0.3); // the ground colour but it gets lighter the higher the point is. 
}