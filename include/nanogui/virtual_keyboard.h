#pragma once
#include <nanogui/widget.h>

NAMESPACE_BEGIN(nanogui)

class NANOGUI_EXPORT VirtualKeyboard : public Widget {
public:
    VirtualKeyboard(Widget *parent);

    virtual Vector2i preferred_size(NVGcontext *ctx) const override;
    void perform_layout(NVGcontext *ctx) override;
    virtual void draw(NVGcontext *ctx) override;
};

NAMESPACE_END(nanogui)
