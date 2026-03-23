#pragma once
#include "Common.h"
#include <cstddef>

namespace GraphicsCore
{
    struct BufferDesc
    {
        size_t size;
        BufferUsage usage;
        bool cpuAccessible;
    };

    class IBuffer
    {
    public:
        virtual ~IBuffer() = default;
        virtual const BufferDesc& GetDesc() const = 0;
        virtual void* GetNativeHandle() const = 0;
    };
}
