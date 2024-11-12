#version 460 core

// The input to this shader stage will be 4 control points (quad), each vertex will be placed equidistant from each other. 
layout(quads, equal_spacing, ccw) in;

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
layout(location = 6) in float inBladeDirection[];

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

    // IF THE MIDDLE AXIS OF THE BLADE RESULTS IN A V SHAPE IN ITS CROSS SECTION, USE THIS
    // DisplacementVector = Width * Normal * (0.5 - |u -0.5| * (1 - v));

    // Convert the bezier control points into world space temporarily as modifying values that are already in clip space will break the output.
    // TODO: Pass in the world positions and convert to clip space within this shader.
    mat4 inverseVP = inverse(ubo.proj * ubo.view);
    vec4 worldP0 = inverseVP * clippedP0;
    vec4 worldP1 = inverseVP * clippedP1;
    vec4 worldP2 = inverseVP * clippedP2;
    
    // Define quad vertices in local space, relative to the curve's control points. World space: X is right. Y is forward. Z is up. 
    vec4 topLeftWorldSpace      = vec4(-inBladeWidth[0], 0.0, inBladeHeight[0], 1.0);
    vec4 topRightWorldSpace     = vec4( inBladeWidth[0], 0.0, inBladeHeight[0], 1.0);
    vec4 bottomLeftWorldSpace   = vec4(-inBladeWidth[0], 0.0, 0.0, 1.0);
    vec4 bottomRightWorldSpace  = vec4( inBladeWidth[0], 0.0, 0.0, 1.0); 

    // Calculate a point along the bezier curve.
    vec4 bezierPos = (1.0 - u) * (1.0 - u) * worldP0 + 2.0 * (1.0 - u) * u * worldP1 + u * u * worldP2; 

    // If the shape has a tip, it is important that the translation has to decrease the nearer the generated point is to the top.
    //vec4 displacement = inBladeWidth[0] * normal * (0.5 - abs(u - 0.5) * (1 - v));

    //v0 p0 dir
    //v1 p1 height
    //v2 p2 width
    //up up stiff

    vec3 a = worldP0.xyz + v * (worldP1.xyz - worldP0.xyz);
    vec3 b = worldP1.xyz + v * (worldP2.xyz - worldP1.xyz);
    vec3 c = a + v * (b - a);
    vec3 t1 = vec3(sin(inBladeDirection[0]), 0.0, cos(inBladeDirection[0])); // bitan
    float w = inBladeWidth[0];
    vec3 c0 = c - w * t1;
    vec3 c1 = c + w * t1;
    vec3 t0 = normalize(b - a); 
    float t = u + 0.5 * v - u * v; // triangle

    vec4 position;
    position.xyz = (1.0 - t) * c0 + t * c1;
    position.w = 1.0;
    gl_Position = ubo.proj * ubo.view * position;





    // Define quad vertices in local space, relative to the curve's control points. World space: X is right. Y is forward. Z is up. 
    //vec4 topLeftWorldSpace      = vec4(-inBladeWidth[0], 0.0, inBladeHeight[0], 1.0);
    //vec4 topRightWorldSpace     = vec4( inBladeWidth[0], 0.0, inBladeHeight[0], 1.0);
    //vec4 bottomLeftWorldSpace   = vec4( inBladeWidth[0], 0.0, 0.0, 1.0);
    //vec4 bottomRightWorldSpace  = vec4(-inBladeWidth[0], 0.0, 0.0, 1.0);    

    //vec4 a = mix(topLeftWorldSpace, topRightWorldSpace, u);
    //vec4 b = mix(bottomRightWorldSpace, bottomLeftWorldSpace, u);
    //gl_Position = mix(a, b, v); // Returns correctly standing blades with no alignment.

    // Clip space conversion for each generated vertex: thanks Charlie.
    //vec4 bottomCentreClipSpace = clippedP0;
    //vec4 topLeftQuadPointClipSpace       = bottomCentreClipSpace + (ubo.proj * ubo.view * topLeftWorldSpace);          // Top left vertex.
    //vec4 topRightQuadPointClipSpace      = bottomCentreClipSpace + (ubo.proj * ubo.view * topRightWorldSpace);         // Top right vertex.
    //vec4 bottomLeftQuadPointClipSpace    = bottomCentreClipSpace + (ubo.proj * ubo.view * bottomLeftWorldSpace);       // Bottom left vertex (the grass position is the centre bottom to the quad).
    //vec4 bottomRightQuadPointClipSpace   = bottomCentreClipSpace + (ubo.proj * ubo.view * bottomRightWorldSpace);      // Bottom right vertex (the grass position is the centre bottom to the quad).

    //vec4 a = mix(topLeftQuadPointClipSpace, topRightQuadPointClipSpace, u);
    //vec4 b = mix(bottomRightQuadPointClipSpace, bottomLeftQuadPointClipSpace, u);
    //gl_Position = mix(a, b, v); // Returns correctly standing blades with no alignment.

    //float t = u - ((u * v) * (u * v));

    // Interpolate between the points to correctly position the generated vertices on a per-quad data basis.
    //vec4 topVerticesInterpolated = mix(topLeftQuadPointClipSpace, topRightQuadPointClipSpace, u);           // Horizontal lerp between the top two vertices.
    //vec4 bottomVerticesInterpolated = mix(bottomRightQuadPointClipSpace, bottomLeftQuadPointClipSpace, u);  // Horizontal lerp between the bottom two vertices.

    // De Casteljau's Bézier deritative:
    // Q0 = (1 - t)P0 + tP1, Q1 = (1 - t)P1 + tP2
    // B(t) = (1 - t)Q0 + tQ1
    //vec4 q0 = (1 - v) * clippedP0 + v * clippedP1;
    //vec4 q1 = (1 - v) * clippedP1 + v * clippedP2;
    //vec4 bt = (1 - v) * q0 + v * q1;

    //vec4 p0p1Interpolated = clippedP0 + v * (clippedP1 - clippedP0);                                        // Lerp between p0 and p1.
    //vec4 p1p2Interpolated = clippedP1 + v * (clippedP2 - clippedP1);                                        // Lerp between p1 and p2.
    //vec4 curvePointAtV = p0p1Interpolated + v * (p1p2Interpolated - p0p1Interpolated);                      // Lerp between those values (as if using de Castlejau).

    // Tangent, Bitangent & Normal vectors.
    //vec4 tangentAtV = vec4(evaluateTangentOnQuadraticBezier(clippedP0.xyz, clippedP1.xyz, clippedP2.xyz, v), 1.0); 
    //vec4 normalisedTangent = (p1p2Interpolated - p0p1Interpolated) / length(p1p2Interpolated - p0p1Interpolated);
    //vec4 widthTangent = tangentAtV * 1.0 * 0.5; // scale the tangent by the blade width.
    
    //vec4 normalisedNormal = vec4(cross(normalisedTangent.xyz, tangentAtV.xyz) / length(cross(normalisedTangent.xyz, tangentAtV.xyz)), 1.0); 
    
    //vec3 bitangentAtV = normalize(cross(normalisedTangent.xyz, normalisedNormal.xyz));
    //vec4 widthBitangent = vec4(bitangentAtV * 1.0 * 0.5, 1.0); // scale the bitangent by the blade width.

    // Compute the left and right edge points of the quad in world space.
    //vec4 curvePointLeftWorldSpace = curvePointAtV - widthTangent;
    //vec4 curvePointRightWorldSpace = curvePointAtV + widthTangent; 

    //float horizontalLerpValue = u + 0.5 * v - u * v; // parameter to interpolate between the left and right edge of the quad

    //vec4 pos = ubo.proj * ubo.view * ((1.0 - horizontalLerpValue) * curvePointLeftWorldSpace + horizontalLerpValue * curvePointRightWorldSpace);
    //gl_Position = pos;

    //vec4 clipSpacePosition = bottomCentreClipSpace + (ubo.proj * ubo.view * ((1.0 - horizontalLerpValue) * curvePointLeftWorldSpace + horizontalLerpValue * curvePointRightWorldSpace));
    //gl_Position = clipSpacePosition + mix(topVerticesInterpolated, bottomVerticesInterpolated, v);

    //vec4 clip = mix(curvePointLeftWorldSpace, curvePointRightWorldSpace, horizontalLerpValue);
    //gl_Position = ubo.proj * ubo.view * clip;

    // Use gl_TessCoord.y to gradient the blade to be black at the bottom and green at the top, faking shadows.
    outColor = inColor[0] * (1 - v); 
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
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