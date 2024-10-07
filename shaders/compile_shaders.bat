@echo off
setlocal

rem Path to the glslc executable
set GLSLC_PATH="C:/Users/c1027117/OneDrive - SHeffield Hallam University/_Year 4/Vulkan Shader Compiler/bin/glslc.exe"
rem set GLSLC_PATH="C:/Users/lukeh/OneDrive - SHeffield Hallam University/_Year 4/Vulkan Shader Compiler/bin/glslc.exe"

rem Compile .vert files
for %%f in (*.vert) do (
    echo Compiling %%f to %%~nf.vert.spv...
    %GLSLC_PATH% %%f -o %%~nf.vert.spv
)

rem Compile .frag files
for %%f in (*.frag) do (
    echo Compiling %%f to %%~nf.frag.spv...
    %GLSLC_PATH% %%f -o %%~nf.frag.spv
)

rem pause
echo All shaders compiled!
endlocal