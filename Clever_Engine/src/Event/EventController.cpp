#include "EventController.h"

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	EventController* eventController = static_cast<EventController*>(glfwGetWindowUserPointer(window));

	InputCodes::Keyboard cleverKey = (InputCodes::Keyboard)keyboardGLFWtoCleverKeyCodes.at(key);

	if (action == GLFW_PRESS)
		eventController->setKey(cleverKey, true);
	else
		eventController->setKey(cleverKey, false);
}

static void mouseButton_callback(GLFWwindow* window, int key, int action, int mods)
{
	EventController* eventController = static_cast<EventController*>(glfwGetWindowUserPointer(window));

	InputCodes::Mouse cleverMouseButton = (InputCodes::Mouse)mouseGLFWtoCleverButtonCodes.at(key);

	if (action == GLFW_PRESS)
		eventController->setMouseButton(cleverMouseButton, true);
	else
		eventController->setMouseButton(cleverMouseButton, false);
}

EventController::EventController()
{

}

void EventController::Init(Window& window)
{
	eventData->p_window = glfwData.p_GLFWwindow;

	keyInputData->eventData = eventData;

	glfwSetWindowUserPointer(eventData->p_window, this);
	glfwSetKeyCallback(eventData->p_window, key_callback);
	glfwSetMouseButtonCallback(eventData->p_window, mouseButton_callback);
}

void EventController::Update()
{
	glfwPollEvents();

	for (const auto& [key, value] : keyboardGLFWtoCleverKeyCodes)
	{
		if (glfwGetKey(eventData->p_window, key))
			keyInputData->keys[value] = true;
		else
			keyInputData->keys[value] = false;
	}
}

void EventController::CleanUp()
{

}

void EventController::setKey(InputCodes::Keyboard key, bool state)
{
	keyInputData->keys[key] = state;
}

void EventController::setMouseButton(InputCodes::Mouse button, bool state)
{
	keyInputData->mouseButtons[button] = state;
}