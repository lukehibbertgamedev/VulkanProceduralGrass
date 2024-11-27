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

    //vec4 p00 = gl_in[0].gl_Position;
    //vec4 p01 = gl_in[1].gl_Position;
    //vec4 p10 = gl_in[2].gl_Position;
    //vec4 p11 = gl_in[3].gl_Position;

    vec4 p0 = vec4(inPosition[0], 1.0);
    vec4 p1 = vec4(inPosition[1], 1.0);
    vec4 p2 = vec4(inPosition[2], 1.0);
    vec4 p3 = vec4(inPosition[3], 1.0);

    vec2 t0 = inUv[0];
    vec2 t1 = inUv[1];
    vec2 t2 = inUv[2];
    vec2 t3 = inUv[3];

    //vec2 t0 = (t01 - t00) * u + t00;
    //vec2 t1 = (t11 - t10) * u + t10;
    //vec2 texCoord = (t1 - t0) * v + t0;

    //vec2 uv1 = mix(t00, t01, u);
    //vec2 uv2 = mix(t11, t10, u);
    //vec2 uvCoord = mix(uv1, uv2, v);

    // Interpolate UV coordinates
	vec2 uv1 = mix(inUv[0], inUv[1], gl_TessCoord.x);
	vec2 uv2 = mix(inUv[3], inUv[2], gl_TessCoord.x);
	vec2 outUV = mix(uv1, uv2, gl_TessCoord.y);
    float height = texture(heightMapSampler, outUV).r * 64.0 - 16.0;

    //vec4 uVec = p01 - p00;
    //vec4 vVec = p10 - p00;
    //vec4 normal = normalize(vec4(cross(vVec.xyz, uVec.xyz), 1.0));

    //vec4 p0 = (p01 - p00) * u + p00;
    //vec4 p1 = (p11 - p10) * u + p10;
    //vec4 p = (p1 - p0) * v + p0;
    //p += normal * height;

    //vec4 pos1 = mix(p00, p01, u);
    //vec4 pos2 = mix(p10, p11, u);
    //vec4 pos = mix(pos1, pos2, v);

    // Interpolate positions
	//vec4 pos1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	//vec4 pos2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
	//vec4 pos = mix(pos1, pos2, gl_TessCoord.y);

    //pos.z -= height;

    //gl_Position = ubo.proj * ubo.view * ubo.model * pos;
    
    // Convert the final position into clip space.
    //gl_Position = ubo.proj * ubo.view * ubo.model * vec4(p.xyz, 1.0);

    vec4 a = mix(vec4(inPosition[0], 1.0), vec4(inPosition[1], 1.0), u);
    vec4 b = mix(vec4(inPosition[3], 1.0), vec4(inPosition[2], 1.0), u);
    vec4 mixedPos = mix(a, b, v);
    //mixedPos.z -= height;
    // Convert the final position into clip space.

    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) + (gl_TessCoord.y * gl_in[1].gl_Position) + (gl_TessCoord.z * gl_in[2].gl_Position);
    gl_Position = ubo.proj * ubo.view * ubo.model * mixedPos; // Matrix transformations go here if necessary.

    outColor = vec4(1.0, 1.0, 1.0, 1.0); // white for visual. 
}