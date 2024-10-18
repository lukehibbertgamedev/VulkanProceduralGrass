#pragma once

#include <glm/glm.hpp>  // For glm::vec2 and glm::vec3



// A class representing ONE blade of grass using the Bézier representation.

// Predefined blade of grass, subdivided in a tessellation shader.
// Additional detail can be generated without needing to store it explicitly


// 1. Generate a cubic Bézier curve using 3 control points.
// 2. Generate a blade shape (typically a quad) that will be subdivided in the tessellation stage.
// 3. Map/align the base blade quad to the shape of the Bézier curve by evaluating the curve interpolation of the control points for each vertex (see De Casteljau's algorithm).
// 4. One grass blade generated???


// https://dl.acm.org/doi/pdf/10.1145/3023368.3023380
// Initially, the blade geometry is a flat quad that is defined by the interpolation parameters u and v, 
// where u indicates the interpolation along the width of the blade and v the interpolation along the
// height. 
// 
// width = 5
// height = 6
// 
// u = lerp(width_start_point, width_end_point, interpolation)
// v = lerp(height_start_point, height_end_point, interpolation)
// 
// eg if interpolation = 0.5 then width = 2.5 height = 3.
// 
//   - - - - - 
//   | \     |
//   |  \    |
//   |   \   | 
//   |    \  |
//   |     \ |
//   |      \|
//   - - - - - 
// 
// By evaluating the curve interpolation of the control points for each generated vertex, the 
// quad becomes aligned to the Bezier curve. This is achieved by using De Casteljau’s algorithm, 
// which also calculates the tangent vector t0 as intermediate results.

//
//	P3 Tip -> o---o <- P1 Height
//	              |
//	              |
//	              |
//	              |
//	              |
//	              |
//	              o <- P0 Base
//

// void MakePersistentLength(in vec3 groundPos, inout vec3 v1, inout vec3 v2, in float height)
// {
//     //Persistent length
//     vec3 v01 = v1 - groundPos;
//     vec3 v12 = v2 - v1;
//     float lv01 = length(v01);
//     float lv12 = length(v12);
// 
//     float L1 = lv01 + lv12;
//     float L0 = length(groundPosV2);
//     float L = (2.0f * L0 + L1) / 3.0f; //http://steve.hollasch.net/cgindex/curves/cbezarclen.html
// 
//     float ldiff = height / L;
//     v01 = v01 * ldiff;
//     v12 = v12 * ldiff;
//     v1 = groundPos + v01;
//     v2 = v1 + v12;
// }

// float3 bezierDerivative(float3 p0, float3 p1, float3 p2, float t)
// {
// 	   return 2. * (1. - t) * (p1 - p0) + 2. * t * (p2 - p1);
// }

// float3 bezier(float3 p0, float3 p1, float3 p2, float t)
// {
// 	float3 a = lerp(p0, p1, t);
// 	float3 b = lerp(p1, p2, t);
// 	return lerp(a, b, t);
// }




namespace bezier {

	const uint8_t kNumPointsAlongBezier = 10;

	struct bezier { 
	public: 
		glm::vec3 p0, p1, p2; // Control points where p0 is the base, p1 is the height, p2 is the tip.
		float interpolation; // Linear interpolation value between 0-1. If this = 0.5 then it is halfway between point A and point B.
	};

	// Given the coordinates of control points Pi, the first control point has coordinates P1 = (x1, y1), the second, P2 = (x2, y2) and so on.
	// The curve coordinates are described by the equation that depends on parameter t between 0-1.
	// This will generate a given point on the *final* bezier curve using three control points.
	// p0, p1, and p2 are control points. interpolationValue is usually known as 't' within a lerp, the amount of positional blend between some point A and point B.

	// Using De Casteljau's algorithm, which recursively evaluates polynomials in Bézier curves.

	glm::vec3 lerp(const glm::vec3& p0, const glm::vec3& p1, float t) {
		return glm::vec3((1 - t) * p0.x + t * p1.x, (1 - t) * p0.y + t * p1.y, (1 - t) * p0.z + t * p1.z);
	}

	glm::vec3 deCasteljau(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, float t) {
		glm::vec3 p0p1 = lerp(p0, p1, t); 
		glm::vec3 p1p2 = lerp(p1, p2, t); 
		return lerp(p0p1, p1p2, t);
	}
}


class Blade {
public:

	Blade();

	float lean = 0.3f;
	float height = 1.0f;

	glm::vec3 position; // The world space position of this blade.
	glm::vec3 direction; // In the direction that it will curve/lean.

	glm::vec3 p0; // Blade base.
	glm::vec3 p1; // Blade height.
	glm::vec3 p2; // Blade tip.
};