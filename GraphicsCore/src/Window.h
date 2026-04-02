#pragma once
#include <cstdint>
#include <functional>

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

    // Minimal callback interface the backend calls into
    struct IWindowEventSink
    {
        virtual ~IWindowEventSink() = default;
        virtual void OnKey(int glfwKey, int scancode, int action, int mods) = 0;
        virtual void OnMouseButton(int button, int action, double x, double y, int mods) = 0;
        virtual void OnMouseMove(double x, double y) = 0;
        virtual void OnMouseScroll(double xoffset, double yoffset) = 0;
        virtual void OnFocus(bool focused) = 0;
        virtual void OnClose() = 0;
    };

    class IWindow
    {
    public:
        virtual ~IWindow() = default;
        virtual const WindowDesc& GetDesc() const = 0;
        virtual void* GetNativeHandle() const = 0;
        virtual void* GetPlatformHandle() const = 0;
        virtual ITexture* GetCurrentBackbuffer() = 0;
        virtual void BeginFrame() = 0;
        virtual bool IsFrameReady() const = 0;
        virtual void SetEventSink(IWindowEventSink* sink) = 0;
        virtual void SetTitle(const char* title) = 0;
        virtual void GetPosition(int& x, int& y) const = 0;
        virtual void SetPosition(int x, int y) = 0;
        virtual void SetSize(uint32_t width, uint32_t height) = 0;
    };
}
