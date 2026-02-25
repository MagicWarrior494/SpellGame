#include "World/Assets/Shader.h"

ShaderMetadata ReflectCombinedShaders(const std::vector<uint32_t>& vertCode, const std::vector<uint32_t>& fragCode)
{
    ShaderMetadata meta;

    auto ProcessStage = [&](const std::vector<uint32_t>& code, ShaderStage stage) {
        SpvReflectShaderModule module;
        SpvReflectResult result = spvReflectCreateShaderModule(
            code.size() * sizeof(uint32_t),
            code.data(),
            &module);

        if (result != SPV_REFLECT_RESULT_SUCCESS) {
            throw std::runtime_error("SPIR-V reflection failed.");
        }

        uint32_t count = 0;
        spvReflectEnumerateDescriptorBindings(&module, &count, nullptr);
        std::vector<SpvReflectDescriptorBinding*> bindings(count);
        spvReflectEnumerateDescriptorBindings(&module, &count, bindings.data());

        for (auto* b : bindings) {
            bool found = false;
            for (auto& existing : meta.bindings) {
                if (existing.binding == b->binding && existing.set == b->set) {
                    existing.stage |= stage;
                    found = true;
                    break;
                }
            }

            if (!found) {
                ShaderBinding sb;
                sb.binding = b->binding;
                sb.set = b->set;
                sb.name = b->name;
                sb.stage = stage;
                sb.count = b->count;
                sb.type =
                    (b->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER) ? BindingType::UniformBuffer :
                    (b->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER) ? BindingType::StorageBuffer :
                    (b->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE) ? BindingType::SampledImage :
                    (b->descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) ? BindingType::CombinedImageSampler :
                    throw std::runtime_error("Unsupported descriptor type in shader reflection: " + std::string(b->name));

                meta.bindings.push_back(sb);
            }
        }

        uint32_t pcCount = 0;
        spvReflectEnumeratePushConstantBlocks(&module, &pcCount, nullptr);
        std::vector<SpvReflectBlockVariable*> pcs(pcCount);
        spvReflectEnumeratePushConstantBlocks(&module, &pcCount, pcs.data());

        for (auto* p : pcs) {
            bool found = false;
            for (auto& existing : meta.pushConstants) {
                if (existing.offset == p->offset && existing.size == p->size) {
                    existing.stage |= stage;
                    found = true;
                    break;
                }
            }

            if (!found) {
                ShaderMetadata::PushConstant pc;
                pc.name = p->name;
                pc.size = p->size;
                pc.offset = p->offset;
                pc.stage = stage;
                meta.pushConstants.push_back(pc);
            }
        }

        spvReflectDestroyShaderModule(&module);
    };

    ProcessStage(vertCode, ShaderStage::Vertex);
    ProcessStage(fragCode, ShaderStage::Fragment);

    return meta;
}