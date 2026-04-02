#pragma once
#include <glm.hpp>
#include <string>
#include <cstdint>

// ---------------------------------------------------------------
// UIWidget — base data shared by every UI element.
// All positions and sizes are in pixel-space relative to the
// top-left corner of the UIScene they belong to.
// ---------------------------------------------------------------
struct UIWidget
{
    glm::vec2 position{ 0.0f, 0.0f }; // top-left corner in pixels
    glm::vec2 size{ 100.0f, 40.0f };  // width and height in pixels
    glm::vec4 color{ 0.3f, 0.3f, 0.3f, 1.0f }; // RGBA [0..1]
    bool      visible = true;
};

// ---------------------------------------------------------------
// ButtonWidget — a pressable rectangular region.
// ---------------------------------------------------------------
struct ButtonWidget : UIWidget
{
    glm::vec4 hoverColor { 0.45f, 0.45f, 0.45f, 1.0f };
    glm::vec4 pressColor { 0.2f,  0.2f,  0.2f,  1.0f };

    // Runtime state written by UIScene each frame
    bool isHovered = false;
    bool isPressed = false;
};
