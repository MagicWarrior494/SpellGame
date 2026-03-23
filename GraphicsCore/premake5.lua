project "GraphicsCore"
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    staticruntime "On"

    targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
    objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

    warnings "Extra"
    fatalwarnings { "All" }

    includedirs
    {
        "include"
    }

    files
    {
        "include/**.h",
        "src/**.cpp"
    }