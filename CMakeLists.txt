cmake_minimum_required(VERSION 3.10)

project (ProceduralGrass LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/VulkanApplication.cpp
)

set(INCLUDE
	${CMAKE_CURRENT_SOURCE_DIR}/include/VulkanApplication.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/stb_image.h
)

add_executable(${PROJECT_NAME} ${SRC} ${INCLUDE})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)
add_definitions(-DVULKAN_SDK=${VulkanSDK})

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/third_party/glfw")	# Add glfw's target to my project.
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/third_party/glfw/include")	
target_link_libraries(${PROJECT_NAME} glfw)	# Adds the glfw library and its link-time dependencies as it is currently configured (include dir for the glfw header).
target_link_libraries(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/third_party/AdditionalDLLs/glfw3.lib")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/third_party/glm")	# Add glm's target to my project.
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/third_party/glm")	
#target_link_libraries(${PROJECT_NAME} glm)

# Compile ImGui as a static library
file(GLOB IMGUI_SOURCES 
	${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui.h
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imconfig.h
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui_internal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imstb_rectpack.h
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imstb_textedit.h
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imstb_truetype.h
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui_widgets.cpp
    # Specific bindings.
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_glfw.h
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_vulkan.h
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_vulkan.cpp
)
add_library("ImGui" STATIC ${IMGUI_SOURCES})
target_include_directories("ImGui" PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui)
target_include_directories("ImGui" PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends)
target_link_libraries(${PROJECT_NAME} ImGui)

file(GLOB VK_SOURCES     
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/PortableVulkanSDK1.3/include/empty.cpp # To force the cache to understand which target language this library is.
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/PortableVulkanSDK1.3/include/vulkan/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/PortableVulkanSDK1.3/include/vulkan/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/PortableVulkanSDK1.3/include/vulkan/utility/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/PortableVulkanSDK1.3/include/vulkan/utility/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/PortableVulkanSDK1.3/include/vk_video/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/PortableVulkanSDK1.3/include/vma/vk_mem_alloc.h
)
add_library("PortableVulkan" SHARED ${VK_SOURCES})
target_include_directories("PortableVulkan" PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/PortableVulkanSDK1.3/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/PortableVulkanSDK1.3/include/vulkan
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/PortableVulkanSDK1.3/include/vma
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/PortableVulkanSDK1.3/include/vk_video
)
#target_link_libraries(${PROJECT_NAME} PortableVulkan)
target_link_libraries(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/third_party/PortableVulkanSDK1.3/lib/vulkan-1.lib")

# Copy DLLs to output directory without directly targeting them.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/AdditionalDLLs/glfw3.dll
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

