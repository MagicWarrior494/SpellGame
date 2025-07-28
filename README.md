# SpellGame Solution

This is a C++ game development project structured around a modular engine (`Clever_Engine`) and a game executable (`SpellGame`). The project uses **Premake5** for cross-platform build configuration and **Doxygen** to generate documentation for the engine.

## 🔨 Building the Project

1. Open a terminal in the root folder (`SpellGame_Solution/`)
2. Run the following commands:

```bash
premake5 --bootstrap     # Fetch dependencies
premake5 vs2022          # Generate Visual Studio 2022 solution
```
Open SpellGame_Solution.sln in Visual Studio

📖 Generating Documentation
We use Doxygen to generate HTML documentation for the Clever_Engine library.

➤ To generate the documentation:
```bash
cd Clever_Engine
doxygen Doxyfile
```
Building  Clever_Engine will automatically generate the documentation in the `Docs/html/` folder.

➤ To view the documentation:
Open the following file in your web browser:
Docs/html/index.html


🧰 Dependencies
These libraries are automatically cloned into the Dependencies/ folder via the Premake bootstrap step:

Vulkan SDK
ImGui
glm
stb_image
tinyobjloader
spdlog

these will be autmatically retrieved when running the bootstrap of premake5
```bash
premake5 --bootstrap
```

📎 Notes
Make sure the Vulkan SDK is installed and the VULKAN_SDK environment variable is set in your path