#pragma once

#include <iostream>
#include <random>
#include <chrono>
#include <fstream> 
#include <vector>

#include <glm/glm.hpp>  

namespace Utils {

	// Returns a random float value in the range [min, max].
	static float getRandomFloat(float min, float max) {

		static std::random_device device;
		static std::mt19937 generator(device());
		std::uniform_real_distribution<float> distribution(min, max);
		return distribution(generator);
	}

	// Returns a random float, no range limitations.
	static float getRandomFloat() {
		return rand() / (float)RAND_MAX;
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

	// Read the character data from a text file.
	static std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		return buffer;
	}
}