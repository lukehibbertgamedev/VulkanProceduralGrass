#pragma once

#include <string>
#include <optional>

struct QueueFamilyIndices {
public:
	std::optional<uint32_t> graphicsAndComputeFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() { return graphicsAndComputeFamily.has_value() && presentFamily.has_value(); }
};

struct GPUData {
public:
	std::string name;
	uint32_t version = 0;
	unsigned int versionMajor = 0, versionMinor = 0;
	unsigned int apiMajor = 0, apiMinor = 0, apiPatch = 0;
	uint32_t numVisible = 0;
};