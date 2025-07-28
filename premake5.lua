newoption {
    trigger = "bootstrap",
    description = "Fetch external dependencies"
}

-- Bootstrap function: Downloads deps if missing
function bootstrap()
    print("Fetching dependencies...")

    if not os.isdir("Dependencies/ImGui") then
        os.execute("git clone https://github.com/ocornut/imgui.git Dependencies/ImGui")
    end

    if not os.isdir("Dependencies/glm") then
        os.execute("git clone https://github.com/g-truc/glm.git Dependencies/glm")
    end

    if not os.isdir("Dependencies/stb_image") then
        os.execute("git clone https://github.com/nothings/stb.git Dependencies/stb_image")
    end

    if not os.isdir("Dependencies/tinyobjloader") then
        os.execute("git clone https://github.com/tinyobjloader/tinyobjloader.git Dependencies/tinyobjloader")
    end

    if not os.isdir("Dependencies/spdlog") then
        os.execute("git clone https://github.com/gabime/spdlog.git Dependencies/spdlog")
    end

    print("Dependencies fetched successfully.")
end

if _OPTIONS["bootstrap"] then
    bootstrap()
end

workspace "SpellGame_Solution"
    architecture "x64"
    configurations { "Debug", "Release" }
    startproject "SpellGame"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- ================================
-- Clever_Engine (Static Library)
-- ================================
project "Clever_Engine"
    location "Clever_Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    { 
        "%{prj.name}/src/**.h", 
        "%{prj.name}/src/**.cpp",
        "Dependencies/ImGui/*.h",
        "Dependencies/ImGui/*.cpp"
    }

    includedirs
    {
        "%{prj.name}/src",
        "Dependencies/ImGui",
        "Dependencies/glm",
        "Dependencies/stb_image",
        "Dependencies/tinyobjloader",
        "Dependencies/spdlog/include",
        os.getenv("VULKAN_SDK") .. "/Include"
    }

    libdirs
    {
        os.getenv("VULKAN_SDK") .. "/Lib"
    }

    links
    {
        "vulkan-1"
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

-- ================================
-- SpellGame (Executable)
-- ================================
project "SpellGame"
    location "SpellGame"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files 
    { 
        "%{prj.name}/src/**.h", 
        "%{prj.name}/src/**.cpp" 
    }

    includedirs
    {
        "Clever_Engine/src"
    }

    links
    {
        "Clever_Engine"
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
