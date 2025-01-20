#pragma once

// ===============================================================================================================================================================================

// For use in internally calculating delta time for the application update.

// ===============================================================================================================================================================================

#include <chrono>

// ===============================================================================================================================================================================

class Timer {
public:
    Timer() {
        lastTime = std::chrono::high_resolution_clock::now();
    }

    float getDeltaTime() {
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        return deltaTime.count(); // Return delta time in seconds
    }

private:
    std::chrono::high_resolution_clock::time_point lastTime;
};