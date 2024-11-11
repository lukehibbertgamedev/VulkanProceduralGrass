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
layout(location = 1) in vec4 inPosition[];
layout(location = 2) in float inBladeWidth[];
layout(location = 3) in float inBladeHeight[];

layout(location = 0) out vec4 outColor;

void main() 
{    
    // Necessary when point_mode is enabled.
    gl_PointSize = 3.0f; 

    // gl_TessCoord - Barycentric coordinates : The location of a point corresponding to the tessellation patch.
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;


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
    // pn = generated position from the GPU (below).

    // Position the generated vertices in a quad-like shape, raised by the blade height. 

    // World space: X is right. Y is forward. Z is up.
    vec4 topLeftWorldSpace      = vec4(-inBladeWidth[0], 0.0, inBladeHeight[0], 1.0);
    vec4 topRightWorldSpace     = vec4( inBladeWidth[0], 0.0, inBladeHeight[0], 1.0);
    vec4 bottomLeftWorldSpace   = vec4( inBladeWidth[0], 0.0, 0.0, 1.0);
    vec4 bottomRightWorldSpace  = vec4(-inBladeWidth[0], 0.0, 0.0, 1.0);

    // Convert to clip space for each generated vertex: thanks Charlie.
    // inPosition[0] = blade.p0_and_width.
    vec4 bladeWorldPositionBottomMiddle = inPosition[0];
    vec4 topLeftClipSpace       = bladeWorldPositionBottomMiddle + (ubo.proj * ubo.view * topLeftWorldSpace);          // Top left vertex.
    vec4 topRightClipSpace      = bladeWorldPositionBottomMiddle + (ubo.proj * ubo.view * topRightWorldSpace);         // Top right vertex.
    vec4 bottomLeftClipSpace    = bladeWorldPositionBottomMiddle + (ubo.proj * ubo.view * bottomLeftWorldSpace);       // Bottom left vertex (the grass position is the centre bottom to the quad).
    vec4 bottomRightClipSpace   = bladeWorldPositionBottomMiddle + (ubo.proj * ubo.view * bottomRightWorldSpace);      // Bottom right vertex (the grass position is the centre bottom to the quad).

    // Interpolate between the points to correctly position the generated vertices on a per-quad data basis.
    vec4 a = mix(topLeftClipSpace, topRightClipSpace, u);
    vec4 b = mix(bottomRightClipSpace, bottomLeftClipSpace, u);
    gl_Position = mix(a, b, v);
    
    // Use gl_TessCoord.y to gradient the blade to be black at the bottom and green at the top, faking shadows.
    outColor = inColor[0] * (1 - v); 
}
