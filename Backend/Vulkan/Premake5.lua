-- ================================
-- Vulkan Backend (DLL Plugin)
-- ================================
project "Vulkan"
    location "."
    kind "SharedLib"
    language "C++"
    cppdialect "C++23"
    staticruntime "off"

    targetdir ("../../bin/" .. outputdir .. "/%{prj.name}")
    objdir   ("../../bin-int/" .. outputdir .. "/%{prj.name}")

    warnings "Extra"

    files 
    { 
        "src/**.h", 
        "src/**.cpp" 
    }

    includedirs 
    {
        "src",
        "../../GraphicsCore/src",
        "../../GraphicsCore/include",
        "../../Dependencies/GLFW/include",
        "../../Dependencies/glm/glm",
        "../../Dependencies/VulkanMemoryAllocator/include",
        os.getenv("VULKAN_SDK") .. "/Include"
    }

    libdirs 
    {
        os.getenv("VULKAN_SDK") .. "/Lib"
    }

    links 
    {
        "GLFW",
        "vulkan-1"
    }

    defines
    {
        "VK_USE_PLATFORM_WIN32_KHR",
        "WIN32_LEAN_AND_MEAN",
        "NOMINMAX"
    }

    filter "system:windows"
        systemversion "latest"
        linkoptions { "/IMPLIB:../../bin/" .. outputdir .. "/Vulkan/Vulkan.lib" }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        defines { "_DEBUG" }

    filter "configurations:Release"
        runtime "Release"
        optimize "on"