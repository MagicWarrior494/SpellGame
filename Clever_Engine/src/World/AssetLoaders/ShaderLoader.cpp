#include "ShaderLoader.h"
#include "World/Assets/Resource.h"

std::shared_ptr<Shader> ShaderLoader::LoadFromFile(std::shared_ptr<GraphicsAPI> graphicsAPI, const std::filesystem::path& filePath)
{
    if (!graphicsAPI)
        throw std::runtime_error("ShaderLoader: graphicsAPI is null");

    // --- 1. SPIR-V paths convention ---
    std::filesystem::path vertPath = filePath;
    vertPath.replace_extension(".vert.spv");
    std::filesystem::path fragPath = filePath;
    fragPath.replace_extension(".frag.spv");

    // --- 2. Read SPIR-V bytecode ---
    auto ReadFile = [](const std::filesystem::path& path) -> std::vector<uint32_t>
        {
            std::ifstream file(path, std::ios::ate | std::ios::binary);
            if (!file.is_open())
                throw std::runtime_error("Cannot open shader file: " + path.string());

            size_t fileSize = static_cast<size_t>(file.tellg());

            if (fileSize % sizeof(uint32_t) != 0)
                throw std::runtime_error("SPIR-V file size is not multiple of 4: " + path.string());

            std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

            file.seekg(0);
            file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

            return buffer;
        };

    std::vector<uint32_t> vertCode = ReadFile(vertPath);
    std::vector<uint32_t> fragCode = ReadFile(fragPath);

    // --- 3. Create ShaderHandles via GraphicsAPI ---
    ShaderHandle vertHandle = graphicsAPI->CreateShader(ShaderStage::Vertex, vertCode);
    ShaderHandle fragHandle = graphicsAPI->CreateShader(ShaderStage::Fragment, fragCode);

    // --- 5. Create Shader object ---
    auto shader = std::make_shared<Shader>();
    shader->vertexHandle = vertHandle;
    shader->fragmentHandle = fragHandle;
	shader->m_Reflection = ReflectCombinedShaders(vertCode, fragCode);
    
	//Create Descriptor Layouts using Spir-v reflection data
    //Gets stored in GraphicsAPI and uses the shaderHandle as the key

	//Create pipeline LAYOUT using descriptor layouts and push constant info from reflection data
	//Gets stored in GraphicsAPI and uses the shaderHandle as the key


    return shader;
}
