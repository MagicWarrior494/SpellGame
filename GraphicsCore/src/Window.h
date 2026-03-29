#pragma once
#include <cstdint>

namespace GraphicsCore
{
    // Forward declaration
    class ITexture;

    struct WindowDesc
    {
        uint32_t width;
        uint32_t height;
        const char* title;
        bool fullscreen;
        bool vsync;
    };

    class IWindow
    {
    public:
        virtual ~IWindow() = default;
        virtual const WindowDesc& GetDesc() const = 0;
        virtual void* GetNativeHandle() const = 0;
        virtual void* GetPlatformHandle() const = 0;
        virtual ITexture* GetCurrentBackbuffer() = 0;
    };
}
