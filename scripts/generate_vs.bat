@echo off
setlocal disabledelayedexpansion

set ENABLE_IMGUI=ON
set ENABLE_PTHREAD=ON

call scripts/find_cmake
if %errorlevel% GTR 0 (
	echo find_cmake returned %errorlevel%
	exit /b 1
)

mkdir build_x64
pushd build_x64
	call %CMAKE_FILE%^
	-DENABLE_IMGUI=%ENABLE_IMGUI%^
	-DENABLE_PTHREAD=%ENABLE_PTHREAD%^
	-DBUILD_SHARED_LIBS=OFF^
	-DCMAKE_CONFIGURATION_TYPES=Debug;Release;RelWithDebInfo^
	-G "Visual Studio 17 2022" -A x64 ..
popd

echo.
echo [31;4mdon't forget to tell vulkan to use our local vulkan layers[0m
echo [31;4msee slides for more details[0m

pause