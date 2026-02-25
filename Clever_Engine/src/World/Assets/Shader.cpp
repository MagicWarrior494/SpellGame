#include "World/Assets/Shader.h"
#include "Helper/spirv_reflect.h"

ShaderReflection ReflectCombinedShaders(
    const std::vector<uint32_t>& vertCode,
    const std::vector<uint32_t>& fragCode)
{
    ShaderReflection reflection{};

    std::vector<const std::vector<uint32_t>*> shaders =
    {
        &vertCode,
        &fragCode
    };

    for (const auto* shaderCode : shaders)
    {
        SpvReflectShaderModule module{};
        SpvReflectResult result = spvReflectCreateShaderModule(
            shaderCode->size() * sizeof(uint32_t),
            shaderCode->data(),
            &module);

        if (result != SPV_REFLECT_RESULT_SUCCESS)
            continue; // or assert

        // -------------------------------
        // Reflect Descriptor Bindings
        // -------------------------------
        uint32_t count = 0;
        spvReflectEnumerateDescriptorBindings(&module, &count, nullptr);

        std::vector<SpvReflectDescriptorBinding*> bindings(count);
        spvReflectEnumerateDescriptorBindings(&module, &count, bindings.data());

        for (uint32_t i = 0; i < count; ++i)
        {
            const SpvReflectDescriptorBinding* b = bindings[i];

            ShaderResource resource{};
            resource.name = b->name ? b->name : "";

            resource.set = b->set;
            resource.binding = b->binding;
            resource.descriptorCount = b->count;
            resource.stages = ShaderStage::None;
            resource.size = b->block.size;

            // Map Vulkan type → Your enum
            switch (b->descriptor_type)
            {
            case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                resource.type = ShaderResourceType::UniformBuffer;
                break;

            case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
                resource.type = ShaderResourceType::StorageBuffer;
                break;

            case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
                resource.type = ShaderResourceType::CombinedImageSampler;
                break;

            case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                resource.type = ShaderResourceType::SampledImage;
                break;

            case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
                resource.type = ShaderResourceType::StorageImage;
                break;

            case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
                resource.type = ShaderResourceType::Sampler;
                break;

            default:
                resource.type = ShaderResourceType::Unknown;
                break;
            }

            // Convert stage flags
            if (module.shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT)
                resource.stages |= ShaderStage::Vertex;

            if (module.shader_stage == SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT)
                resource.stages |= ShaderStage::Fragment;

            reflection.resources.push_back(resource);
        }

        // -------------------------------
        // Reflect Push Constants
        // -------------------------------
        uint32_t pcCount = 0;
        spvReflectEnumeratePushConstantBlocks(&module, &pcCount, nullptr);

        std::vector<SpvReflectBlockVariable*> pushConstants(pcCount);
        spvReflectEnumeratePushConstantBlocks(&module, &pcCount, pushConstants.data());

        if (pcCount > 0)
        {
            const auto* block = pushConstants[0];

            reflection.pushConstants.totalSize = block->size;

            for (uint32_t i = 0; i < block->member_count; ++i)
            {
                const auto& member = block->members[i];

                PushConstantMember pcm{};
                pcm.name = member.name ? member.name : "";
                pcm.offset = member.offset;
                pcm.size = member.size;

                reflection.pushConstants.members.push_back(pcm);
            }

            if (module.shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT)
                reflection.pushConstants.stages |= ShaderStage::Vertex;

            if (module.shader_stage == SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT)
                reflection.pushConstants.stages |= ShaderStage::Fragment;
        }

        // -------------------------------
        // Reflect Vertex Inputs
        // -------------------------------
        uint32_t attrCount = 0;
        spvReflectEnumerateInputVariables(&module, &attrCount, nullptr);

        std::vector<SpvReflectInterfaceVariable*> attributes(attrCount);
        spvReflectEnumerateInputVariables(&module, &attrCount, attributes.data());

        for (uint32_t i = 0; i < attrCount; ++i)
        {
            const auto* attr = attributes[i];

            if (!attr->name)
                continue;

            // Skip built-ins like gl_Position
            if (attr->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
                continue;

            VertexAttribute vattr{};
            vattr.name = attr->name;
            vattr.location = attr->location;

            reflection.vertexLayout.attributes.push_back(vattr);
        }

        spvReflectDestroyShaderModule(&module);
    }

    return reflection;
}