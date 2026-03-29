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
    objdir ("../../bin-int/" .. outputdir .. "/%{prj.name}")

    warnings "Extra"

    -- Don't treat all warnings as fatal errors - some VMA warnings are unavoidable
    -- fatalwarnings { "All" }

    files 
    { 
        "src/**.h", 
        "src/**.cpp" 
    }

    includedirs 
    {
        "src",
        os.getenv("VULKAN_SDK") .. "/Include",
        "../../Dependencies/glm/glm",
        "../../Dependencies/VulkanMemoryAllocator/include",
        "../../GraphicsCore/src",
        "../../Dependencies/GLFW/include"
    }

    libdirs 
    {
        os.getenv("VULKAN_SDK") .. "/Lib"
    }

    links 
    {
        "vulkan-1",
        "GLFW"
    }

    -- GraphicsCore is a header-only library, so we just need the include path
    -- No library linking required

    defines
    {
        "VK_USE_PLATFORM_WIN32_KHR"  -- Required for Windows Vulkan surface extensions
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        defines { "_DEBUG" }

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
