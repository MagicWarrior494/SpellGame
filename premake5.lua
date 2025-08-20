workspace "SpellGame_Solution"
    architecture "x64"
    configurations { "Debug", "Release" }
    startproject "SpellGame"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

include "Clever_Engine/Vulkan" 
include "Clever_Engine"
include "SpellGame"