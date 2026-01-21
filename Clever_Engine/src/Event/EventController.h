#pragma once
#include <vector>
#include <functional>
#include <map>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <GLFW/glfw3.h>

#include "Io/KeyCodes.h"

struct InputEvent {
    enum class Type { Key, MouseButton, MouseScroll, MouseMove, WindowClose, WindowResize };
    Type type;
	int code;      // Clever input code
    Input::Action action;    // Clever action code
    double x, y;
    double deltaX, deltaY;
    bool handled = false;

    void Consume() { handled = true; }
};

class IInputLayer {
public:
    virtual ~IInputLayer() = default;
    virtual void OnInput(InputEvent& event) = 0;
    virtual int GetZIndex() const = 0;
};

class EventController {
public:
    // Functions for GLFW callbacks
    void PostKeyEvent(int glfwKey, int scancode, int action, int mods);
    void PostMouseButtonEvent(int glfwButton, int action, double xpos, double ypos, int mods);
    void PostMouseMoveEvent(double xpos, double ypos);
    void PostMouseScrollEvent(double xoffset, double yoffset);
    void PostWindowCloseEvent(GLFWwindow* ptr);
    void PostResizeEvent(uint8_t windowId, int width, int height);
    
    // Layer Management
    void AttachLayer(IInputLayer* layer);
    void DetachLayer(IInputLayer* layer);
    void ResetLayers();

    // Global Actions
    using EventLambda = std::function<void()>;
    void RegisterGlobalAction(Input::Keyboard key, Input::Action action, EventLambda func);

    // Helpers to get string names
    static std::string GetKeyName(Input::Keyboard key);
    static std::string GetMouseButtonName(Input::Mouse button);

private:
    void Dispatch(InputEvent& event);
    std::vector<IInputLayer*> m_LayerStack;

    double lastFrameMouseX = -1;
    double lastFrameMouseY = -1;

    struct GlobalAction {
        Input::Action action;
        EventLambda callback;
    };
    std::map<Input::Keyboard, GlobalAction> m_GlobalKeyMap;
};