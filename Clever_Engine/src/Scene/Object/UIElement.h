#pragma once
#include "Render/Window/Window.h"
#include "Element.h"

class UIElement : public Element{
};

class Button : public UIElement {
public:
    std::string label;
    std::function<void(Window&)> onClick;
    // Position and size
    int x, y, width, height;

    void Render(Window& window) override;
    void OnClick(Window& window) override { if (onClick) onClick(window); }
    bool IsHovered(double mouseX, double mouseY) override;
};

class TextBox : public UIElement {
public:
    std::string text;
    int x, y, width, height;

    void Render(Window& window) override;
    void OnClick(Window& window) override { focused = true; }
    void OnKey(Window& window, int key) override;
    bool IsHovered(double mouseX, double mouseY) override;
};