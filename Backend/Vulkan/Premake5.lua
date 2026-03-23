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
    fatalwarnings { "All" }

    files 
    { 
        "src/**.h", 
        "src/**.cpp" 
    }

    includedirs 
    {
        "src",
        os.getenv("VULKAN_SDK") .. "/Include",
        "../Dependencies/glm/glm",
        "../../GraphicsCore/include"
    }

    libdirs 
    {
        os.getenv("VULKAN_SDK") .. "/Lib"
    }

    links 
    {
        "vulkan-1",
        "GraphicsCore"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"