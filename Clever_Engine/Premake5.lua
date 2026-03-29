-- Clever_Engine (Engine DLL)
project "Clever_Engine"
    location "."
    kind "SharedLib"
    language "C++"
    cppdialect "C++23"
    staticruntime "off"

    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

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
        "../GraphicsCore/include",
        "../Dependencies/glm/glm"
    }

    links
    {
        "GraphicsCore"
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        debugdir "%{wks.location}"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"