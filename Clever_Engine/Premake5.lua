project "Clever_Engine"
location "."
kind "SharedLib"
language "C++"
cppdialect "C++23"
staticruntime "off"

targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
objdir   ("../bin-int/" .. outputdir .. "/%{prj.name}")

warnings "Extra"
fatalwarnings { "All" }
disablewarnings { "4251", "4275" }

files
{
    "src/**.h",
    "src/**.cpp",
    "src/World/Assets/Helper/spirv_reflect.c"
}

defines
{
    "WIN32_LEAN_AND_MEAN",
    "NOMINMAX"
}

includedirs
{
    "src",
    "../GraphicsCore/src",
    "../GraphicsCore/include",
    "../Backend/Vulkan/src",
    "../Dependencies/glm/glm",
    "../Dependencies/GLFW/include",
    "../Dependencies/SPIRV-Headers/include",
    "../Dependencies/VulkanMemoryAllocator/include",
    os.getenv("VULKAN_SDK") .. "/Include"
}

externalincludedirs
{
    "../Dependencies/tinyobjloader"
}

externalwarnings "Off"

libdirs
{
    os.getenv("VULKAN_SDK") .. "/Lib",
    "../bin/" .. outputdir .. "/Vulkan",
    "../bin/" .. outputdir .. "/GLFW"
}

links
{
    "Vulkan",
    "GLFW",
    "vulkan-1"
}

dependson { "Vulkan", "GLFW" }

-- tinyobjloader's fast_float rejects C++23 constexpr rules; compile under C++17
filter "files:src/World/AssetLoaders/Meshloader.cpp"
    cppdialect "C++17"
    warnings "Off"

-- spirv_reflect.c is plain C
filter "files:src/World/Assets/Helper/spirv_reflect.c"
    language "C"
    warnings "Off"

filter "system:windows"
    systemversion "latest"
    linkoptions { "/IMPLIB:../bin/" .. outputdir .. "/Clever_Engine/Clever_Engine.lib" }

filter "configurations:Debug"
    runtime "Debug"
    symbols "on"
    debugdir "%{wks.location}"

filter "configurations:Release"
    runtime "Release"
    optimize "on"