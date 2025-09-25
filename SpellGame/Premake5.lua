-- ================================
-- SpellGame (Executable)
-- ================================

project "SpellGame"
    location "."
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++23"
    staticruntime "on"

    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    { 
        "src/**.h", 
        "src/**.cpp" 
    }

    includedirs
    {
        "src",
        "../Clever_Engine/src",
        "../Clever_Engine/Vulkan/src",
        "../Clever_Engine/Dependencies/glm/glm",
        "../Clever_Engine/Dependencies/stb_image",
        "../Clever_Engine/Dependencies/tinyobjloader",
        "../Clever_Engine/Dependencies/spdlog/include/spdlog",
        "../Clever_Engine/Dependencies/ImGui",
        "../Clever_Engine/Dependencies/GLFW/include",
        os.getenv("VULKAN_SDK") .. "/Include"
    }

    libdirs {
        "../Clever_Engine/Dependencies/GLFW/lib-vc2022",
        os.getenv("VULKAN_SDK") .. "/Lib",
    }

    links
    {
        "Clever_Engine",
        "vulkan-1",
        "Vulkan",
        "glfw3"
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
