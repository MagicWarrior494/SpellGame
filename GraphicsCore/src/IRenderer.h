#pragma once
#include "Buffer.h"
#include "Texture.h"
#include "Shader.h"
#include "Pipeline.h"
#include "CommandList.h"
#include "Sampler.h"
#include "Window.h"

// Windows.h may define CreateWindow as a macro, which conflicts with our method names
#ifdef CreateWindow
#undef CreateWindow
#endif

namespace GraphicsCore
{
    class IRenderer
    {
    public:
        virtual ~IRenderer() = default;

        // --- Resource Creation ---
        virtual IBuffer* CreateBuffer(const BufferDesc& desc) = 0;
        virtual void DestroyBuffer(IBuffer* buffer) = 0;

        virtual ITexture* CreateTexture(const TextureDesc& desc) = 0;
        virtual void DestroyTexture(ITexture* texture) = 0;

        virtual IShader* CreateShader(const ShaderDesc& desc) = 0;
        virtual void DestroyShader(IShader* shader) = 0;

        virtual IPipeline* CreatePipeline(const PipelineDesc& desc) = 0;
        virtual void DestroyPipeline(IPipeline* pipeline) = 0;

        virtual ICommandList* CreateCommandList() = 0;
        virtual void DestroyCommandList(ICommandList* commandList) = 0;

        virtual ISampler* CreateSampler(const SamplerDesc& desc) = 0;
        virtual void DestroySampler(ISampler* sampler) = 0;

        virtual IWindow* CreateWindow(const WindowDesc& desc) = 0;
        virtual void DestroyWindow(IWindow* window) = 0;

        // --- Data Upload ---
        // Used for "cpuAccessible" buffers to copy data from CPU to GPU
        virtual void* MapBuffer(IBuffer* buffer) = 0;
        virtual void UnmapBuffer(IBuffer* buffer) = 0;

        // Add these to the "Resource Creation" section of IRenderer
        virtual IResourceLayout* CreateResourceLayout(const ResourceLayoutDesc& desc) = 0;
        virtual void DestroyResourceLayout(IResourceLayout* layout) = 0;

        virtual IResourceSet* CreateResourceSet(IResourceLayout* layout) = 0;
        virtual void DestroyResourceSet(IResourceSet* set) = 0;

        // --- Execution ---
        // Pushes the command list to the GPU for processing
        virtual void Submit(ICommandList* commandList, IWindow* window = nullptr) = 0;
        virtual void SubmitBlit(ICommandList* commandList, IWindow* window) = 0;

        // Immediately submits and blocks until the GPU has finished — for one-shot uploads
        virtual void SubmitImmediate(ICommandList* commandList) = 0;

        // Swaps the buffers for the specific window
        virtual void Present(IWindow* window) = 0;

        // Wait for GPU to finish all work (useful during shutdown)
        virtual void WaitIdle() = 0;

        // Process platform/window events (must run in the DLL that owns the windowing system)
        virtual void PollEvents() = 0;
    };
}