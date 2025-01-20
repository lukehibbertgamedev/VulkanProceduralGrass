# Procedural Grass Renderer in Vulkan

## Information

Procedural geometry aims to utilise graphical processing unit advancements in an attempt to visually outperform the common and reliable approach by representing individual grass blades as Bézier curves. This results in a technique that is more advanced and allows grass to feel more dynamic by adapting to natural environmental effects or player interaction. This project tests the claim that using a low-level API to manipulate graphical processing unit drivers in a way that improves the overall generation, rendering, and animation of grass blades within an immersive environment when compared to other, standard APIs.  

This project is inspried by Klemens Jahrmann and Michael Wimmer with their Interactive Grass Rendering Using Real-Time Tessellation (Jahrmann & Wimmer, 2017). 
Read the paper [here](https://publik.tuwien.ac.at/files/PubDat_220935.pdf).

You can read more on the development of this project on my portfolio, where my paper will be hosted too. Find the site [here](https://lukehibbertportfolio.wixsite.com/gamedev/procedural-grass-renderer).

## Installation

**Note**: This project is only supported on Windows.

Pre-requisites:
- [Visual Studio 2022](https://visualstudio.microsoft.com/vs/)
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)

Clone the repository using ```git clone github.com/lukehibbertgamedev/VulkanProceduralGrass.git --recurse-submodules```.

Run ```generate_vs.bat```, this will unpack and build CMake, then generate a Visual Studio 2022 solution within build_x64/.

## Vulkan SDK

If using a university computer, ensure to open the Vulkan Configurator using AppsAnywhere. This will ensure the Vulkan SDK is on the desktop, and can be accessed by the CMake files.

If using a personal computer, ensure the Vulkan SDK is downloaded to a global space on your desktop, typically the C drive.
