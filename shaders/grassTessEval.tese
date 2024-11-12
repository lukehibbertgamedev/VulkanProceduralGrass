#version 460 core

// The input to this shader stage will be 4 control points (quad), each vertex will be placed equidistant from each other. 
layout(quads, fractional_even_spacing, ccw) in;

// Binding is 0 here because it's a uniform buffer object with binding 0 within the descriptor set layout.
layout(binding = 0) uniform CameraUniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec4 inColor[];
layout(location = 1) in vec4 inP0_Width[];      // This represents the bezier curve base, where this lies in the bottom centre of the quad.
layout(location = 2) in vec4 inP1_Height[];     // This represents the 'height' of the bezier curve, where this lies directly above p0.
layout(location = 3) in vec4 inP2_Direction[];  // This represents the tip of the curve, where this is updated to animate the blade.

layout(location = 0) out vec4 outColor;

void main() 
{    
    // Necessary if/when point_mode is enabled.
    gl_PointSize = 3.0f; 

    // Control points for the Bézier curve in world space, clip space conversion is done for everything at the end.
    vec4 P0 = inP0_Width[0];
    vec4 P1 = inP1_Height[0];
    vec4 P2 = inP2_Direction[0];

    // gl_TessCoord - Barycentric coordinates : The location of a point corresponding to the tessellation patch.
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;  

    float width = P0.w;
    float direction = P2.w;
    float smoothnessFactor = 0.5; // Half the smoothing value resulting in a sharper tip.

    // De Casteljau's algorithm to get a point on the Bézier curve.
    vec3 a = P0.xyz + v * (P1.xyz - P0.xyz);
    vec3 b = P1.xyz + v * (P2.xyz - P1.xyz);
    vec3 c = a + v * (b - a);

    // Tangent and bitangent.
    vec3 t0 = normalize(b - a); 
    vec3 t1 = vec3(cos(direction), 0.0, sin(direction));    

    // Resulting curve points that span the width of the blade, lerping between these values will provide a smooth tip.
    vec3 c0 = c - width * t1;
    vec3 c1 = c + width * t1;
    
    // Define interpolation parameter ...
    float t = u + 0.5 * v - u * v; 

    // Lerp between ...
    vec3 position = mix(c0, c1, t * smoothnessFactor);

    // Convert the final position into clip space.
    gl_Position = ubo.proj * ubo.view * vec4(position, 1.0);

    // Use gl_TessCoord.y to gradient the blade to be black at the bottom and green at the top, faking shadows.
    outColor = inColor[0] * v; // (1-v) if needed
    //outColor = vec4(1.0, 0.0, v, 1.0); // for grass passing in height
    //outColor = vec4(1.0, 0.0, 0.0, 1.0); // red for debug.
}

// Example comment image of how the vertices are generated/positioned from the single input.
//p0  //    //p1
//          //
//          //
//          //
//    //    //
//          //
//          //
//          //
//p2  wp    //p3
// wp = world position sent from the CPU.
// pn = generated position from the GPU.