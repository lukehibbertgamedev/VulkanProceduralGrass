@echo off

set CHECK_GLOBAL=ON
if not "%1."=="." if "%1"=="OFF" (
	set CHECK_GLOBAL=OFF
)

set CMAKE_VERSION_MAJOR=3.30
set CMAKE_VERSION=%CMAKE_VERSION_MAJOR%.4

rem check if cmake is installed globally
set CMAKE_FILE=cmake.exe
set CTEST_FILE=ctest.exe
if %CHECK_GLOBAL%==ON if exist %CMAKE_FILE% if exist %CTEST_FILE% (
	goto cmake_found
)

rem check if cmake has already been downloaded locally
set CMAKE_BIN=cmake_bin
set CMAKE_FILE=%CD%\vendor\%CMAKE_BIN%\bin\cmake.exe
set CTEST_FILE=%CD%\vendor\%CMAKE_BIN%\bin\ctest.exe
if exist %CMAKE_FILE% if exist %CTEST_FILE% (
	goto cmake_found
)

echo downloading cmake...

mkdir vendor
pushd vendor
	set TEMP_NAME=temp

	rem download cmake
	curl -L https://github.com/Kitware/CMake/releases/download/v%CMAKE_VERSION%/cmake-%CMAKE_VERSION%-windows-x86_64.zip > %TEMP_NAME%.zip
	rem unzip cmake into a temporary folder called %TEMP_NAME%
	Powershell.exe Expand-Archive -LiteralPath %TEMP_NAME%.zip -Force

	rem make bin folder
	rmdir /s /q %CMAKE_BIN%
	mkdir %CMAKE_BIN%
	rem move cmake.exe into vendor
	move %TEMP_NAME%\cmake-%CMAKE_VERSION%-windows-x86_64\bin %CMAKE_BIN%
	move %TEMP_NAME%\cmake-%CMAKE_VERSION%-windows-x86_64\share %CMAKE_BIN%

	rem delete temporary folder
	rmdir /s /q %TEMP_NAME%
	rem delete zip file
	del /f %TEMP_NAME%.zip
popd

if exist %CMAKE_FILE% if exist %CTEST_FILE% (
	goto cmake_found
)
echo failed to download cmake
exit /b 1

:cmake_found

echo cmake @ %CMAKE_FILE%
