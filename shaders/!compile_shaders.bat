@echo off
setlocal

rem Path to the glslc executable
rem set GLSLC_PATH="C:/Users/c1027117/OneDrive - SHeffield Hallam University/_Year 4/Vulkan Shader Compiler/bin/glslc.exe"  rem Uni PC.
rem set GLSLC_PATH="C:/Users/lukeh/OneDrive - SHeffield Hallam University/_Year 4/Vulkan Shader Compiler/bin/glslc.exe"     rem Home PC.
set GLSLC_PATH="C:/VulkanSDK/1.3.216.0/Bin/glslc.exe"

rem Compile .vert (vertex) files
for %%f in (*.vert) do (
    echo Compiling %%f to %%~nf.vert.spv...
    %GLSLC_PATH% %%f -o %%~nf.vert.spv
)

rem Compile .frag (fragment) files
for %%f in (*.frag) do (
    echo Compiling %%f to %%~nf.frag.spv...
    %GLSLC_PATH% %%f -o %%~nf.frag.spv
)

rem Compile .comp (compute) files
for %%f in (*.comp) do (
    echo Compiling %%f to %%~nf.comp.spv...
    %GLSLC_PATH% %%f -o %%~nf.comp.spv
)

rem Compile .tesc (tessellation control shader) files
for %%f in (*.tesc) do (
    echo Compiling %%f to %%~nf.tesc.spv...
    %GLSLC_PATH% %%f -o %%~nf.tesc.spv
)

rem Compile .tese (tessellation evaluation shader) files
for %%f in (*.tese) do (
    echo Compiling %%f to %%~nf.tese.spv...
    %GLSLC_PATH% %%f -o %%~nf.tese.spv
)

pause
echo All shaders compiled!
endlocal