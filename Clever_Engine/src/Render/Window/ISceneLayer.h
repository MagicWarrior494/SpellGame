#pragma once
#include <cstdint>
#include "IRenderer.h"

struct SceneDesc;
class Window;

// Minimal interface that Window::Render needs from any renderable layer.
// Both Scene and UIScene implement this.
class ISceneLayer
{
public:
    virtual ~ISceneLayer() = default;
    virtual void                    Update()                             = 0;
    virtual void                    Render()                             = 0;
    virtual void                    Resize(uint32_t w, uint32_t h)      = 0;
    virtual GraphicsCore::ITexture* GetColorTarget()             const   = 0;
    virtual const SceneDesc&        GetDesc()                    const   = 0;
    virtual void                    ResetInputState()                    = 0;
    virtual void                    SetPosition(int x, int y)           = 0;
    virtual void                    SetWindow(Window* window)            = 0;
};
