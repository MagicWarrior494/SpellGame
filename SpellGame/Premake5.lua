-- ================================
-- SpellGame (Executable)
-- ================================

project "SpellGame"
    location "."
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++23"
    staticruntime "off"

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
        "../Clever_Engine/src"
    }

    links
    {
        "Clever_Engine"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"