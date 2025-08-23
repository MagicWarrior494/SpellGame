function cloneIfMissing(name, url, dir)
    if not os.isdir(dir) then
        print(">> Cloning " .. name .. "...")
        os.execute("git clone --depth=1 " .. url .. " " .. dir)
    else
        print(">> " .. name .. " already exists, skipping clone.")
    end
end

local deps = "Clever_Engine/Dependencies"

cloneIfMissing("glm",          "https://github.com/g-truc/glm.git",               deps .. "/glm")
cloneIfMissing("stb",          "https://github.com/nothings/stb.git",             deps .. "/stb_image")
cloneIfMissing("tinyobjloader","https://github.com/tinyobjloader/tinyobjloader.git", deps .. "/tinyobjloader")
cloneIfMissing("spdlog",       "https://github.com/gabime/spdlog.git",            deps .. "/spdlog")
cloneIfMissing("imgui",        "https://github.com/ocornut/imgui.git",            deps .. "/ImGui")
cloneIfMissing("glfw",         "https://github.com/glfw/glfw.git",                deps .. "/GLFW")

workspace "SpellGame_Solution"
    architecture "x64"
    configurations { "Debug", "Release" }
    startproject "SpellGame"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


include "Clever_Engine/Vulkan" 
include "Clever_Engine"
include "SpellGame"