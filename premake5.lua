function cloneIfMissing(name, url, dir)
    if not os.isdir(dir) then
        print(">> Cloning " .. name .. "...")
        os.execute("git clone --depth=1 " .. url .. " " .. dir)
    else
        print(">> " .. name .. " already exists, skipping clone.")
    end
end

local deps = "Dependencies"

cloneIfMissing("glm",          "https://github.com/g-truc/glm.git",               deps .. "/glm")
cloneIfMissing("stb",          "https://github.com/nothings/stb.git",             deps .. "/stb_image")
cloneIfMissing("tinyobjloader","https://github.com/tinyobjloader/tinyobjloader.git", deps .. "/tinyobjloader")
cloneIfMissing("spdlog",       "https://github.com/gabime/spdlog.git",            deps .. "/spdlog")
cloneIfMissing("glfw",         "https://github.com/glfw/glfw.git",                deps .. "/GLFW")
cloneIfMissing("spirv",        "https://github.com/KhronosGroup/SPIRV-Headers.git", deps .. "/SPIRV-Headers")
cloneIfMissing("VMA",          "https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git", deps .. "/VulkanMemoryAllocator")

workspace "SpellGameSolution"
    architecture "x64"
    startproject "SpellGame"

    configurations
    {
        "Debug",
        "Release"
    }

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    group "Dependencies"
        include "Dependencies/GLFW"

    group "Core"
        include "GraphicsCore"

    group "Backends"
        include "Backend/Vulkan"

    group "Engine"
        include "Clever_Engine"

    group "Application"
        include "SpellGame"