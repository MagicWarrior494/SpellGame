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

    disablewarnings { "4251", "4275" }  -- STL dll-interface warnings, safe within same runtime

    defines
    {
        "WIN32_LEAN_AND_MEAN",
        "NOMINMAX"
    }

    includedirs
    {
        "src",
        "../Clever_Engine/src",
        "../GraphicsCore/src",
        "../GraphicsCore/include",
        "../Dependencies/glm/glm",
        "../Dependencies/GLFW/include",
        "../Dependencies/SPIRV-Headers/include",
        os.getenv("VULKAN_SDK") .. "/Include"
    }

    links
    {
        "Clever_Engine",
        "Vulkan"
    }

    filter "system:windows"
        systemversion "latest"

        -- Copy runtime DLLs next to the executable after each build
        postbuildcommands
        {
            'xcopy /Q /Y "%{wks.location}bin\\' .. outputdir .. '\\Clever_Engine\\Clever_Engine.dll" "%{cfg.targetdir}\\" > nul',
            'xcopy /Q /Y "%{wks.location}bin\\' .. outputdir .. '\\Vulkan\\Vulkan.dll"              "%{cfg.targetdir}\\" > nul',
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        debugdir "%{wks.location}"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
        debugdir "%{wks.location}"