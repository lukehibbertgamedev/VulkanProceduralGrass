#pragma once

#include <iostream>
#include <random>
#include <chrono>

#include <glm/glm.hpp>  // For glm::vec2 and glm::vec3

namespace Utils {

	// Returns a random float value in the range [min, max].
	static float getRandomFloat(float min, float max) {

		static std::random_device device;
		static std::mt19937 generator(device());
		std::uniform_real_distribution<float> distribution(min, max);
		return distribution(generator);
	}

	// Returns a vector3 containing 3 random floats within the range +/-xBounds, +/-yBounds, +/-zBounds.
	// By default, this function will negate the x component of vec2 parameters for you. Set negate to false if you wish to pass in your own negative bounds.
	static glm::vec3 getRandomVec3(glm::vec2 xBounds, glm::vec2 yBounds, glm::vec2 zBounds, bool negate = true) {
		if (negate) {
			return glm::vec3(getRandomFloat(-xBounds.x, xBounds.y), getRandomFloat(-yBounds.x, yBounds.y), getRandomFloat(-zBounds.x, zBounds.y));
		}
		else {
			return glm::vec3(getRandomFloat(xBounds.x, xBounds.y), getRandomFloat(yBounds.x, yBounds.y), getRandomFloat(zBounds.x , zBounds.y));
		}
	}
}