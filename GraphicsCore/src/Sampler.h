#pragma once
#include "Common.h"

namespace GraphicsCore
{
    struct SamplerDesc
    {
        FilterMode minFilter;
        FilterMode magFilter;
        FilterMode mipFilter;
        WrapMode wrapU;
        WrapMode wrapV;
        WrapMode wrapW;
        float mipLodBias;
        float minLod;
        float maxLod;
        bool anisotropyEnable;
        float maxAnisotropy;
        bool compareEnable;
        CompareOp compareOp;
    };

    class ISampler
    {
    public:
        virtual ~ISampler() = default;
        virtual const SamplerDesc& GetDesc() const = 0;
        virtual void* GetNativeHandle() const = 0;
    };
}
