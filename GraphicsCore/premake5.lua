project "GraphicsCore"
kind "StaticLib"
language "C++"
cppdialect "C++23"
staticruntime "off"

targetdir ("../bin/" .. outputdir .. "/%{prj.name}")
objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

warnings "Extra"
fatalwarnings { "All" }

includedirs
{
    "include",
    "src"
}

files
{
    "include/**.h",
    "src/**.h",
    "src/**.cpp"
}

filter "system:windows"
    systemversion "latest"

filter "configurations:Debug"
    runtime "Debug"
    symbols "on"

filter "configurations:Release"
    runtime "Release"
    optimize "on"