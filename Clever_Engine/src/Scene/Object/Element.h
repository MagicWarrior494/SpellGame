#pragma once
#include "Render/Window/Window.h"

class Element {
public:
    virtual void Render(Window& window) = 0;
    virtual void OnClick(Window& window) {}
    virtual void OnKey(Window& window, int key) {}
    virtual bool IsHovered(double mouseX, double mouseY) = 0;
    bool focused = false;
};

class ViewPort : public Element {
    public:
    int x, y, width, height;
    void Render(Window& window) override;
	void OnClick(Window& window) override { focused = true; }
	void OnKey(Window& window, int key) override;
    //bool IsHovered(double mouseX, double mouseY) override;
};