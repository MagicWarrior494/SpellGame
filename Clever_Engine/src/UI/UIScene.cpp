#include "UI/UIScene.h"
#include "Render/Window/Window.h"

#include <fstream>
#include <stdexcept>
#include <vector>
#include <cstring>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

// ---------------------------------------------------------------
// Push constant layout must match ui_rect.vert / ui_rect.frag
// ---------------------------------------------------------------
struct UIRectPushConstants
{
    glm::vec2 position;    // top-left in pixels
    glm::vec2 size;        // width and height in pixels
    glm::vec4 color;       // RGBA [0..1]
    glm::vec2 resolution;  // scene pixel dimensions
};
static_assert(sizeof(UIRectPushConstants) <= 128,
    "UIRectPushConstants exceeds the guaranteed 128-byte push constant budget");

// ---------------------------------------------------------------
// SPIR-V loader helper
// ---------------------------------------------------------------
static std::vector<uint32_t> ReadSpirv(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("UIScene: cannot open shader: " + path.string());

    size_t size = static_cast<size_t>(file.tellg());
    std::vector<uint32_t> buf(size / sizeof(uint32_t));
    file.seekg(0);
    file.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(size));
    return buf;
}

// ---------------------------------------------------------------
UIScene::UIScene(GraphicsCore::IRenderer* renderer,
                 AssetManager*            assetManager,
                 Window*                  window,
                 const SceneDesc&         desc)
    : m_renderer(renderer)
    , m_assetManager(assetManager)
    , m_window(window)
    , m_desc(desc)
{
    m_eventController = std::make_unique<EventController>();
    m_eventController->AttachLayer(this);

    // Color target — same format as 3D scenes so the blit path is identical
    GraphicsCore::TextureDesc colorDesc{};
    colorDesc.width       = desc.width;
    colorDesc.height      = desc.height;
    colorDesc.depth       = 1;
    colorDesc.mipLevels   = 1;
    colorDesc.arrayLayers = 1;
    colorDesc.type        = GraphicsCore::TextureType::Texture2D;
    colorDesc.format      = GraphicsCore::TextureFormat::RGBA8;
    colorDesc.usage       = GraphicsCore::TextureUsage_RenderTarget
                          | GraphicsCore::TextureUsage_TransferSrc
                          | GraphicsCore::TextureUsage_ShaderResource;
    m_colorTarget = m_renderer->CreateTexture(colorDesc);

    m_commandList = m_renderer->CreateCommandList();

    BuildPipeline();
}

UIScene::~UIScene()
{
    if (m_renderer)
    {
        m_renderer->WaitIdle();
        DestroyPipeline();
        if (m_commandList) m_renderer->DestroyCommandList(m_commandList);
        if (m_colorTarget) m_renderer->DestroyTexture(m_colorTarget);
    }
}

void UIScene::AttachToWindow(Window& window)
{
    window.RegisterScene(this);
}

// ---------------------------------------------------------------
void UIScene::BuildPipeline()
{
    // Resolve the asset root from the AssetManager so we find the compiled .spv files
    std::filesystem::path assetRoot = m_assetManager->GetAssetRoot();
    auto vertCode = ReadSpirv(assetRoot / "shaders/ui_rect.vert.spv");
    auto fragCode = ReadSpirv(assetRoot / "shaders/ui_rect.frag.spv");

    GraphicsCore::ShaderDesc vd{};
    vd.stage        = GraphicsCore::ShaderStage::Vertex;
    vd.bytecode     = vertCode.data();
    vd.bytecodeSize = vertCode.size() * sizeof(uint32_t);
    vd.entryPoint   = "main";
    m_vertShader = m_renderer->CreateShader(vd);

    GraphicsCore::ShaderDesc fd{};
    fd.stage        = GraphicsCore::ShaderStage::Fragment;
    fd.bytecode     = fragCode.data();
    fd.bytecodeSize = fragCode.size() * sizeof(uint32_t);
    fd.entryPoint   = "main";
    m_fragShader = m_renderer->CreateShader(fd);

    GraphicsCore::PipelineDesc pd{};
    pd.vertexShader   = m_vertShader;
    pd.fragmentShader = m_fragShader;
    // No vertex bindings — the quad is generated from gl_VertexIndex

    pd.topology = GraphicsCore::PrimitiveTopology::TriangleList;

    pd.rasterizerState.cullMode              = GraphicsCore::CullMode::None;
    pd.rasterizerState.frontCounterClockwise = true;
    pd.rasterizerState.depthClipEnable       = false;

    // No depth test — UI always draws on top
    pd.depthStencilState.depthTestEnable   = false;
    pd.depthStencilState.depthWriteEnable  = false;
    pd.depthStencilState.depthCompareOp    = GraphicsCore::CompareOp::Always;
    pd.depthStencilState.stencilTestEnable = false;

    // Alpha blending — standard src-alpha over
    pd.blendState.blendEnable          = true;
    pd.blendState.srcColorBlendFactor  = GraphicsCore::BlendFactor::SrcAlpha;
    pd.blendState.dstColorBlendFactor  = GraphicsCore::BlendFactor::OneMinusSrcAlpha;
    pd.blendState.colorBlendOp         = GraphicsCore::BlendOp::Add;
    pd.blendState.srcAlphaBlendFactor  = GraphicsCore::BlendFactor::One;
    pd.blendState.dstAlphaBlendFactor  = GraphicsCore::BlendFactor::OneMinusSrcAlpha;
    pd.blendState.alphaBlendOp         = GraphicsCore::BlendOp::Add;

    pd.colorAttachmentCount      = 1;
    pd.colorAttachmentFormats[0] = GraphicsCore::TextureFormat::RGBA8;
    // No depth attachment for UI
    pd.depthStencilFormat = GraphicsCore::TextureFormat::RGBA8;

    m_pipeline = m_renderer->CreatePipeline(pd);
}

void UIScene::DestroyPipeline()
{
    if (m_pipeline)    { m_renderer->DestroyPipeline(m_pipeline);    m_pipeline    = nullptr; }
    if (m_fragShader)  { m_renderer->DestroyShader(m_fragShader);    m_fragShader  = nullptr; }
    if (m_vertShader)  { m_renderer->DestroyShader(m_vertShader);    m_vertShader  = nullptr; }
}

// ---------------------------------------------------------------
void UIScene::Resize(uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0) return;
    if (width == m_desc.width && height == m_desc.height) return;

    m_renderer->WaitIdle();
    if (m_colorTarget) m_renderer->DestroyTexture(m_colorTarget);

    m_desc.width  = width;
    m_desc.height = height;

    GraphicsCore::TextureDesc colorDesc{};
    colorDesc.width       = width;
    colorDesc.height      = height;
    colorDesc.depth       = 1;
    colorDesc.mipLevels   = 1;
    colorDesc.arrayLayers = 1;
    colorDesc.type        = GraphicsCore::TextureType::Texture2D;
    colorDesc.format      = GraphicsCore::TextureFormat::RGBA8;
    colorDesc.usage       = GraphicsCore::TextureUsage_RenderTarget
                          | GraphicsCore::TextureUsage_TransferSrc
                          | GraphicsCore::TextureUsage_ShaderResource;
    m_colorTarget = m_renderer->CreateTexture(colorDesc);
}

// ---------------------------------------------------------------
void UIScene::Update()
{
    // Update hover state for all buttons based on last known cursor position
    for (auto& btn : m_buttons)
    {
        btn.isHovered = (m_cursorX >= btn.position.x &&
                         m_cursorX <  btn.position.x + btn.size.x &&
                         m_cursorY >= btn.position.y &&
                         m_cursorY <  btn.position.y + btn.size.y);
    }
}

// ---------------------------------------------------------------
void UIScene::RenderWidget(const UIWidget& widget, const glm::vec4& color)
{
    UIRectPushConstants pc{};
    pc.position   = widget.position;
    pc.size       = widget.size;
    pc.color      = color;
    pc.resolution = glm::vec2(static_cast<float>(m_desc.width),
                              static_cast<float>(m_desc.height));

    m_commandList->PushConstants(m_vertShader, 0, sizeof(UIRectPushConstants), &pc);
    m_commandList->Draw(6, 1, 0, 0); // 6 vertices — two triangles, no index buffer
}

void UIScene::Render()
{
    if (!m_renderer || !m_pipeline) return;

    GraphicsCore::IWindow* iw = m_window ? m_window->GetIWindow() : nullptr;
    if (iw && !iw->IsFrameReady()) return;

    m_commandList->Begin();

    m_commandList->TextureBarrier(m_colorTarget,
        GraphicsCore::TextureUsage_ShaderResource,
        GraphicsCore::TextureUsage_RenderTarget);

    // Clear the UI panel to transparent black
    GraphicsCore::ColorAttachment ca{};
    ca.texture        = m_colorTarget;
    ca.loadOp         = GraphicsCore::AttachmentLoadOp::Clear;
    ca.storeOp        = GraphicsCore::AttachmentStoreOp::Store;
    ca.clearColor[0]  = 0.15f;
    ca.clearColor[1]  = 0.15f;
    ca.clearColor[2]  = 0.15f;
    ca.clearColor[3]  = 1.0f;

    m_commandList->BeginRendering(1, &ca, nullptr);

    GraphicsCore::Viewport vp{};
    vp.x        = 0.0f;
    vp.y        = 0.0f;
    vp.width    = static_cast<float>(m_desc.width);
    vp.height   = static_cast<float>(m_desc.height);
    vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;
    m_commandList->SetViewport(vp);

    GraphicsCore::Scissor sc{};
    sc.x      = 0;
    sc.y      = 0;
    sc.width  = m_desc.width;
    sc.height = m_desc.height;
    m_commandList->SetScissor(sc);

    m_commandList->BindPipeline(m_pipeline);

    // Draw buttons
    for (const auto& btn : m_buttons)
    {
        if (!btn.visible) continue;

        glm::vec4 color = btn.color;
        if      (btn.isPressed) color = btn.pressColor;
        else if (btn.isHovered) color = btn.hoverColor;

        RenderWidget(btn, color);
    }

    m_commandList->EndRendering();

    m_commandList->TextureBarrier(m_colorTarget,
        GraphicsCore::TextureUsage_RenderTarget,
        GraphicsCore::TextureUsage_TransferSrc);

    m_commandList->End();

    m_renderer->Submit(m_commandList, iw);
}

// ---------------------------------------------------------------
void UIScene::OnInput(InputEvent& event)
{
    if (event.type == InputEvent::Type::MouseMove)
    {
        // Convert window-space cursor to scene-local pixel space
        m_cursorX = event.x - m_desc.posX;
        m_cursorY = event.y - m_desc.posY;
    }

    if (event.type == InputEvent::Type::MouseButton)
    {
        for (auto& btn : m_buttons)
        {
            bool hit = (m_cursorX >= btn.position.x &&
                        m_cursorX <  btn.position.x + btn.size.x &&
                        m_cursorY >= btn.position.y &&
                        m_cursorY <  btn.position.y + btn.size.y);

            if (hit)
            {
                if (event.action == Input::Action::PRESS)
                    btn.isPressed = true;
                else if (event.action == Input::Action::RELEASE)
                    btn.isPressed = false;
            }
            else
            {
                btn.isPressed = false;
            }
        }
    }
}

void UIScene::ResetInputState()
{
    m_cursorX = -1.0;
    m_cursorY = -1.0;
    for (auto& btn : m_buttons)
        btn.isPressed = false;
}

// ---------------------------------------------------------------
ButtonWidget& UIScene::AddButton(const ButtonWidget& widget)
{
    m_buttons.push_back(widget);
    return m_buttons.back();
}
