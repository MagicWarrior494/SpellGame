-- ================================
-- Clever_Engine (Static Library)
-- ================================
project "Clever_Engine"
    location "."
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    staticruntime "on"

    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    { 
        "src/**.h", 
        "src/**.cpp",
        "Dependencies/ImGui/*.h",
        "Dependencies/ImGui/*.cpp"
    }

    includedirs
    {
        "src",
        "Dependencies/glm/glm",
        "Dependencies/stb_image",
        "Dependencies/tinyobjloader",
        "Dependencies/spdlog/include/spdlog",
        "Dependencies/ImGui",
        "Dependencies/GLFW/include",
        os.getenv("VULKAN_SDK") .. "/Include",
        "Vulkan/src"
    }

    libdirs {
        "Dependencies/GLFW/lib-vc2022",
        os.getenv("VULKAN_SDK") .. "/Lib",
    }

    links
    {
        "vulkan-1",
        "Vulkan",
        "glfw3"
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        prebuildcommands {
            "{MKDIR} Docs"
        }

        postbuildcommands {
            "doxygen Doxyfile"
        }

    filter "configurations:Release"
        runtime "Release"
        optimize "on"