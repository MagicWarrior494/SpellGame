@echo off
setlocal

:: Path to your Vulkan glslc
set GLSLC="C:\VulkanSDK\1.4.335.0\Bin\glslc.exe"

echo Compiling all shaders...

:: Loop through all .vert and .frag files in this folder and subfolders
for /r %%f in (*.vert *.frag) do (

    echo Compiling %%f

    %GLSLC% "%%f" -o "%%f.spv"

    if errorlevel 1 (
        echo Failed compiling %%f
        pause
        exit /b 1
    )
)

echo.
echo Shader compilation complete.
pause