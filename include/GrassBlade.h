#pragma once

#include <../PortableVulkanSDK1.3/include/vulkan/vulkan.h>

#include <glm/glm.hpp>  // For glm::vec2 and glm::vec3

#include <array>
#include <Vertex.h>
#include <Utility.h>

// A class representing ONE blade of grass using the Bézier representation.

constexpr uint32_t MAX_BLADES = 2 << 16;
constexpr uint32_t THREAD_GROUP_SIZE = MAX_BLADES / 256;

#define GRASS_MIN_WIDTH 0.050f	// Will modify the shader values.
#define GRASS_MAX_WIDTH 0.100f	// Will modify the shader values.

#define GRASS_MIN_HEIGHT 0.10f	// Will modify the shader values.
#define GRASS_MAX_HEIGHT 0.80f	// Will modify the shader values.

#define GRASS_STIFFNESS 0.5f	// Has little/no effect as of right now.
#define GRASS_NO_ANGLE 0.0f		// Has little/no effect as of right now.

//#define GRASS_LEAN 0.7f

// 32 bytes large, 4 byte alignment.
struct BladeInstanceData {

	glm::vec4 p0_width = glm::vec4(0.0f);
	glm::vec4 p1_height = glm::vec4(0.0f);
	glm::vec4 p2_direction = glm::vec4(0.0f);
	glm::vec4 up_stiffness = glm::vec4(0.0f);
};

class Blade {
public:

	void initialise(); // Sets up packed 4 v4 values.
	void updatePackedVec4s(); // Call this when changing the values of any packed vec4s, this will recalculate the defaults.

	// All grass members can be defined in four packed vector4s where xyz represents a vector3 and w represents a float.
	glm::vec4 p0AndWidth = glm::vec4(0.0f);		// P0 and grass width.						   
	glm::vec4 p1AndHeight = glm::vec4(0.0f);	// P1 and grass height.						   
	glm::vec4 p2AndDirection = glm::vec4(0.0f);	// P2 and blade direction angle.			   
	glm::vec4 upAndStiffness = glm::vec4(0.0f);	// Grass up vector and stiffness coefficient.  
};

// Further reading notes & implementation ideas below...



// Predefined blade of grass, subdivided in a tessellation shader.
// Additional detail can be generated without needing to store it explicitly


// 1. [X] Generate a quadratic Bézier curve using 3 control points.
// 2. [X] Generate a blade shape (typically a quad) that will be subdivided in the tessellation stage.
// 3. Map/align the base blade quad to the shape of the Bézier curve by evaluating the curve interpolation of the control points for each vertex (see De Casteljau's algorithm).
// 4. One grass blade generated???

// A quad with a base point at p0, extends by base width in orientation direction. Matches the height of blade.height.
// Subdivide quad to increase level of detail in vertices for manipulation in compute shader.


// Preprocess: Distribute blades over a 3D surface terrain.
// Render: Evaluate physical model (apply wind/update animation) > cull > render as tessellated object (using indirect? get gpu mesh data render that)

// The grass blade consists only of 3 vertices p0, p1, p2, which are the control points for a quadratic Bézier curve.
// p0 is a fixed position (as it is the base of the blade)
// p2 is moved according to wind/animation/physical models (this is the control point mostly affecting the tip)
// p1 is moved according to p2 (as this is the connecting height of the blade from the tip)
// A blade also contains height, width, stiffness, up-vector, and direction angle. (this can be covered using 4 v4s)
// Grass will be generated as a single blade.
// Blades are generated with an initial pose where p1 and p2 are in the same place(?) according to height, up vector, and p0 // why??

// v3 displacement of p2 = (recovery + gravity + wind) * change in time + "collision"(?) -> saved into a force map texture -> change in time = time required for the last frame (not normalised)
// recovery: a counterforce to other previously applied forces which follows Hooke's law dependent on the blade's stiffness coefficient.
// gravity: the paper goes into mad gravity detail but ha no this is -9.81
// wind: depends on a wind direction, strength of wind at blade position (p0), alignment of the blade toward the wind direction computed into vector wi (p0).  
// wind then uses multiple sin and cosine functions with different frequencies. wind can be stored in a height map and sampled at grass positions.

// grass blades that are 'straighter' will be affected by wind more than blades that are 'bent' 

// blade validation: 
// p2 can never be below p0, or the terrain but checking this takes too long so p2 = p2 - UP * min(UP * (p2 - p0), 0)

// p1 position calculation: Lproj = ||p2 - p0 - UP * ((p2 - p0) * UP|| 
// if Lproj = 0, p2 rests straight up and p1 has the same position but the blade will always need *some* curvature so:
// p1 = p0 + height * UP * max(1 - (Lproj / height), 0.05 * max(Lproj / height, 1)) // 0.05 = constant curvature factor

// ensure Bézier is not larger than the height of the blade, without this, the length of the blade would be inconsistent when influenced by forces and is a drawback of Jahrmann et al
// this can be done in approximation of Bézier curve lengths as it would take too much time to do otherwise (given a bezier curve degree n)
// https://www.sciencedirect.com/science/article/pii/0925772195000542#:~:text=In%20particular%2C%20if%20a%20B%C3%A9zier,polygon%2Dlengths%20of%20the%20pieces.
// Lbezier = 2*L0+(n-1)*L1 / n+1 where L0 is the distance between the first (p0) and last (p2) control points and L1 is the sum of all distances between a control point and its subsequent one (p0 -> p1 -> p2)
// once its length is calculated, a ratio r between the height of the blade and the length of the bezier curve is 
// calculated, on top of this, correction is applied by multiplying each segment between the control points with r where p1corr p2corr are the corrected control point positions
// r = height / Lbezier
// p1corr = p0 + r * (p1 - p0)
// p2corr = p1corr + r * (p2 - p1)

// Each blade is rendered as a tessellated 2D object, tessellation is used to provide a dynamic level of detail to the blade shape. 
// Because each blade has individual state and position, we cannot render mutliple instances of a single patch (but can single blades).

// Culling single blades requires a rendering pipeline that allows for a dynamic amount of geometry to be rendered perframe. !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Indirect rendering does not include parameters of the draw command, instead, parameters are read from a buffer that is stored in GPU memory. 
// This enables the parameter buffer to be modified in a compute shader without CPU sync. Compute shader is used to cull grass blades. 
// Blades that are approximately parallel to the viewing direction can be culled so calculate absolute value of the cosine of the angle 
// between the viewing direction Dview and vector along the width of the blade Dvec. if (0.9 > |Dview * Dvec|) then cull 
// Blade bounding boxes are calculated on a perframe basis, and if this bounding box is outside the camera's view frustum, it is culled.
// If blades are a certain distance from the camera's position and in frustum, they are culled since they wont be necesary to render.
// Dproj = ||p0 - c - UP * ((p0 - c) * UP)|| where Dproj is calculated distance, c is position of the camera.

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
// which also calculates the tangent vector t0 as intermediate results. The bitangent t1 is provided directly
// by the direction vector along the width of the blade, calculated in advance. With both of these tangent vectors,
// the normal vector can be calculated from it using cross product. 
// a = p0 + v * (p1 - p0)
// b = p1 + v * (p2 - p1)
// c = a + v * (b - a)
// c0 = c - w * t1
// c1 = c + w * t1
// t0 = b - a / ||b - a||
// n = cross(t0, t1) / ||cross(t0, t1)||
// where c is the curve point using interpolation parameter v and c1 and c2 are the two resulting curve points that span the width of the blade.
// for basic shapes, the position p of a vertex for a basic shape is calculated by interpolating between the two curve points c0 and c1 using interpolation parameter t that depends on u and v
// p + (1 - t) * c0 + t * c1
// The quad shape uses parameter u as interpolation parameter t = u so either c0, c, or c1 is emitted.

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

// Indirect structure allows the GPU to execute draw commands without use of the CPU. This will reduce 
// CPU-GPU synchronisation overhead, and efficiently perform instanced rendering, minimising draw calls 
// and data transfers. This structure will act as a single draw call for all grass blades, and allows 
// for parameters to be updated in the GPU buffers without reissuing draw calls from the CPU.
//struct BladeDrawIndirect {
//	uint32_t vertexCount;	// Number of vertices per blade instance.
//	uint32_t instanceCount;	// Number of blade instances.
//	uint32_t firstVertex;	// Index of the first vertex.
//	uint32_t firstInstance;	// Index of the first instance.
//};

//constexpr static unsigned int MAX_BLADES = 1 << 18; // 1 << 18 = 262,144
