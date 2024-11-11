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
layout(location = 1) in vec4 inP0Clipped[];
layout(location = 2) in vec4 inP1Clipped[];
layout(location = 3) in vec4 inP2Clipped[];
layout(location = 4) in float inBladeWidth[];
layout(location = 5) in float inBladeHeight[];

layout(location = 0) out vec4 outColor;

// Evaluates the bezier curve at any point using t as the interpolation value.
vec3 evaluateBernsteinPolynomial(in vec3 p0, in vec3 p1, in vec3 p2, in float t) {
    
    // Formula: B(t) = (1 - t)^2 * p0 + 2 * t * (1 - t) * p1 + t^2 * p2

    float t_sqr = t * t;
    float t_inv = 1 - t;
    float t_inv_sqr = t_inv * t_inv;
    return t_inv_sqr * p0 + 2.0 * t * t_inv * p1 + t_sqr * p2;
}

vec3 calculateTangentOfSpline(in vec3 pb, in vec3 pt, in vec3 h, in vec3 v) {

    // Calculates the tangent vector of a spline.

    // pb: bottom vertex.
    // pt: top vertex.
    // h: additional control point.
    // cv: desired curve points 
    // v: domain coordinate = i / level, for the ith subquad (see ladder looking image for more).

    vec3 a = pb + v * (h - pb);
    vec3 b = h + v * (pt - h);
    vec3 cv = a + v * (b - a);
    return (b - a) / length(b - a); // Tangent.
}

void main() 
{    
    // Necessary if/when point_mode is enabled.
    gl_PointSize = 3.0f; 

    // gl_TessCoord - Barycentric coordinates : The location of a point corresponding to the tessellation patch.
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // These values do not need to be multiplied by the view-projection matrix, as this happens in the vertex shader and is passed through.
    vec4 clippedP0 = inP0Clipped[0]; // This represents the bezier curve base, where this lies in the bottom centre of the quad.
    vec4 clippedP1 = inP1Clipped[0]; // This represents the 'height' of the bezier curve, where this lies directly above p0.
    vec4 clippedP2 = inP2Clipped[0]; // This represents the tip of the curve, where this is updated to animate the blade.

    // Position the generated vertices in a quad-like shape, raised by the blade height:

    // World space: X is right. Y is forward. Z is up.
    vec4 topLeftWorldSpace      = vec4(-inBladeWidth[0], 0.0, inBladeHeight[0], 1.0);
    vec4 topRightWorldSpace     = vec4( inBladeWidth[0], 0.0, inBladeHeight[0], 1.0);
    vec4 bottomLeftWorldSpace   = vec4( inBladeWidth[0], 0.0, 0.0, 1.0);
    vec4 bottomRightWorldSpace  = vec4(-inBladeWidth[0], 0.0, 0.0, 1.0);

    // Convert to clip space for each generated vertex: thanks Charlie.
    vec4 topLeftClipSpace       = clippedP0 + (ubo.proj * ubo.view * topLeftWorldSpace);          // Top left vertex.
    vec4 topRightClipSpace      = clippedP0 + (ubo.proj * ubo.view * topRightWorldSpace);         // Top right vertex.
    vec4 bottomLeftClipSpace    = clippedP0 + (ubo.proj * ubo.view * bottomLeftWorldSpace);       // Bottom left vertex (the grass position is the centre bottom to the quad).
    vec4 bottomRightClipSpace   = clippedP0 + (ubo.proj * ubo.view * bottomRightWorldSpace);      // Bottom right vertex (the grass position is the centre bottom to the quad).

    // Interpolate between the points to correctly position the generated vertices on a per-quad data basis.
    vec4 a = mix(topLeftClipSpace, topRightClipSpace, u);
    vec4 b = mix(bottomRightClipSpace, bottomLeftClipSpace, u);
    gl_Position = mix(a, b, v);
    
    // Use gl_TessCoord.y to gradient the blade to be black at the bottom and green at the top, faking shadows.
    outColor = inColor[0] * (1 - v); 
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