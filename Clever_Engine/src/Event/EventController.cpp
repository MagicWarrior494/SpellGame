#include "EventController.h"
#include "Event/Io/ConversionData.h"

void EventController::PostKeyEvent(int glfwKey, int scancode, int glfwAction, int mods) {
    if (keyboardGLFWtoCleverKeyCodes.find(glfwKey) == keyboardGLFWtoCleverKeyCodes.end()) return;

    InputEvent ev;
    ev.type = InputEvent::Type::Key;
    ev.code = keyboardGLFWtoCleverKeyCodes[glfwKey];
    ev.action = TranslateAction(glfwAction);
    Dispatch(ev);
}
void EventController::PostMouseButtonEvent(int glfwButton, int glfwAction, double xpos, double ypos, int mods) {
    if (mouseGLFWtoCleverKeyCodes.find(glfwButton) == mouseGLFWtoCleverKeyCodes.end()) return;

    InputEvent ev;
    ev.type = InputEvent::Type::MouseButton;
    ev.code = mouseGLFWtoCleverKeyCodes[glfwButton];
    ev.x = xpos;
    ev.y = ypos;
    ev.action = TranslateAction(glfwAction);
    Dispatch(ev);
}
void EventController::PostMouseMoveEvent(double xpos, double ypos) {
    if (lastFrameMouseX == -1 || lastFrameMouseY == -1)
    {
        lastFrameMouseX = xpos;
        lastFrameMouseY = ypos;
    }

    InputEvent ev;
    ev.type = InputEvent::Type::MouseMove;
    ev.x = xpos;
    ev.y = ypos;
    ev.deltaX = xpos - lastFrameMouseX;
    ev.deltaY = ypos - lastFrameMouseY;
    ev.action = Input::Action::UNDEFINED;
    Dispatch(ev);

    lastFrameMouseX = xpos;
    lastFrameMouseY = ypos;
}
void EventController::PostMouseScrollEvent(double xoffset, double yoffset) {
    InputEvent ev;
    ev.type = InputEvent::Type::MouseScroll;
    ev.x = xoffset;
    ev.y = yoffset;
    ev.action = Input::Action::UNDEFINED;
    Dispatch(ev);
}

void EventController::PostWindowCloseEvent(GLFWwindow* ptr)
{
    glfwSetWindowShouldClose(ptr, true);
}

void EventController::PostResizeEvent(uint8_t windowId, int width, int height)
{

}

void EventController::Dispatch(InputEvent& event) {
    // 1. Layer Dispatch (Z-Order)
    for (auto* layer : m_LayerStack) {
        layer->OnInput(event);
        if (event.handled) return;
    }

    // 2. Global Actions
    if (event.type == InputEvent::Type::Key) {
        auto cleverKey = static_cast<Input::Keyboard>(event.code);
        auto it = m_GlobalKeyMap.find(cleverKey);
        if (it != m_GlobalKeyMap.end() && it->second.action == event.action) {
            it->second.callback();
        }
    }
}

void EventController::RegisterGlobalAction(Input::Keyboard key, Input::Action action, EventLambda func) {
    m_GlobalKeyMap[key] = { action, func };
}

void EventController::AttachLayer(IInputLayer* layer) {
    if (std::find(m_LayerStack.begin(), m_LayerStack.end(), layer) != m_LayerStack.end()) {
        return;
    }

    m_LayerStack.push_back(layer);
    std::sort(m_LayerStack.begin(), m_LayerStack.end(), [](IInputLayer* a, IInputLayer* b) {
        return a->GetZIndex() > b->GetZIndex();
    });
}

void EventController::DetachLayer(IInputLayer* layer)
{
	auto it = std::find(m_LayerStack.begin(), m_LayerStack.end(), layer);

    if (it != m_LayerStack.end()) {
		m_LayerStack.erase(it);
    }

    std::sort(m_LayerStack.begin(), m_LayerStack.end(), [](IInputLayer* a, IInputLayer* b) {
        return a->GetZIndex() > b->GetZIndex();
    });
}
