-- ================================
-- Vulkan Static Library
-- ================================
project "Vulkan"
    location "."
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    staticruntime "on"

    targetdir ("../../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../../bin-int/" .. outputdir .. "/%{prj.name}")

    files { "src/**.h", "src/**.cpp" }

    includedirs {
        "src",
        os.getenv("VULKAN_SDK") .. "/Include",
        "../Dependencies/glm/glm",
        "../Dependencies/GLFW/include"
    }

    libdirs {
        os.getenv("VULKAN_SDK") .. "/Lib",
        "../Dependencies/GLFW/lib-vc2022"
    }

    links {
        "vulkan-1",
        "glfw3"
    }

    -- filter "configurations:Debug"
    --     runtime "Debug"
    --     symbols "on"
    --     prebuildcommands {
    --         "{MKDIR} Docs"
    --     }
    --     postbuildcommands {
    --         "doxygen Doxyfile"
    --     }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        staticruntime "Off" -- /MDd

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
        staticruntime "Off" -- /MD