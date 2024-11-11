#version 460 core

// The input to this shader stage will be 4 control points (quad), each vertex will be placed equidistant from each other. 
layout(quads, fractional_even_spacing, ccw, point_mode) in;

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

vec3 evaluateTangentOnQuadraticBezier(in vec3 p0, in vec3 p1, in vec3 p2, in float t) {
    return normalize((2.0 * (1.0 - t) * (p1 - p0)) + (2.0 * t * (p2 - p1)));
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
    
    // Define quad vertices in local space, relative to the curve's control points. World space: X is right. Y is forward. Z is up. 
    vec4 topLeftWorldSpace      = vec4(-inBladeWidth[0], 0.0, inBladeHeight[0], 1.0);
    vec4 topRightWorldSpace     = vec4( inBladeWidth[0], 0.0, inBladeHeight[0], 1.0);
    vec4 bottomLeftWorldSpace   = vec4( inBladeWidth[0], 0.0, 0.0, 1.0);
    vec4 bottomRightWorldSpace  = vec4(-inBladeWidth[0], 0.0, 0.0, 1.0);    

    // Convert to clip space for each generated vertex: thanks Charlie.
    vec4 bottomCentreClipSpace = clippedP0;
    vec4 topLeftClipSpace       = bottomCentreClipSpace + (ubo.proj * ubo.view * topLeftWorldSpace);          // Top left vertex.
    vec4 topRightClipSpace      = bottomCentreClipSpace + (ubo.proj * ubo.view * topRightWorldSpace);         // Top right vertex.
    vec4 bottomLeftClipSpace    = bottomCentreClipSpace + (ubo.proj * ubo.view * bottomLeftWorldSpace);       // Bottom left vertex (the grass position is the centre bottom to the quad).
    vec4 bottomRightClipSpace   = bottomCentreClipSpace + (ubo.proj * ubo.view * bottomRightWorldSpace);      // Bottom right vertex (the grass position is the centre bottom to the quad).

    // Interpolate between the points to correctly position the generated vertices on a per-quad data basis.
    vec4 topVerticesInterpolated = mix(topLeftClipSpace, topRightClipSpace, u); 
    vec4 bottomVerticesInterpolated = mix(bottomRightClipSpace, bottomLeftClipSpace, u);

    vec4 p0p1Interpolated = clippedP0 + v * (clippedP1 - clippedP0); 
    vec4 p1p2Interpolated = clippedP1 + v * (clippedP2 - clippedP1); 
    vec4 curvePointAtV = p0p1Interpolated + v * (p1p2Interpolated - p0p1Interpolated); 

    vec4 tangentAtV = vec4(evaluateTangentOnQuadraticBezier(clippedP0.xyz, clippedP1.xyz, clippedP2.xyz, v), 1.0); 
    vec4 widthTangent = tangentAtV * 1.0 * 0.5; // scale the tangent by the blade width.

    // Compute the left and right edge points of the quad in world space.
    vec4 curvePointLeftWorldSpace = curvePointAtV - widthTangent;
    vec4 curvePointRightWorldSpace = curvePointAtV + widthTangent; 

    vec4 normalisedTangent = (p1p2Interpolated - p0p1Interpolated) / length(p1p2Interpolated - p0p1Interpolated); 
    vec4 normalisedNormal = vec4(cross(normalisedTangent.xyz, tangentAtV.xyz) / length(cross(normalisedTangent.xyz, tangentAtV.xyz)), 1.0); 
    float horizontalLerpValue = u + 0.5 * v - u * v; // parameter to interpolate between the left and right edge of the quad

    //vec4 pos = ubo.proj * ubo.view * ((1.0 - horizontalLerpValue) * curvePointLeftWorldSpace + horizontalLerpValue * curvePointRightWorldSpace);
    //gl_Position = pos;

    vec4 clipSpacePosition = bottomCentreClipSpace + (ubo.proj * ubo.view * ((1.0 - horizontalLerpValue) * curvePointLeftWorldSpace + horizontalLerpValue * curvePointRightWorldSpace));
    gl_Position = clipSpacePosition + mix(topVerticesInterpolated, bottomVerticesInterpolated, v);

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