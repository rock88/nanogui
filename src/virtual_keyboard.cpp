#include <nanogui/virtual_keyboard.h>
#include <nanogui/screen.h>
#include <nanogui/button.h>
#include "nanovg.h"

#include <array>
#include <string>
#include <sstream>
#include <numeric>

using VirtualKeyboardButtonConfig = std::array<std::string, 5>;

VirtualKeyboardButtonConfig test = {
    "{esc} 1 2 3 4 5 6 7 8 9 0 - = {bksp}",
    "{tab} q w e r t y u i o p [ ] \\",
    "{lock} a s d f g h j k l ; ' {enter}",
    "{shift} z x c v b n m , . / {shift}",
    "{winl} {space}"
};

void for_each(VirtualKeyboardButtonConfig& config, std::function<void(std::string, int, int, int)> callback) {
    int c = 0;
    for (auto &line: config) {
        std::istringstream iss(line, std::istringstream::in);
        std::string tmp;
        std::vector<std::string> tmps;
        
        auto column = &line - &config[0];
        
        while (iss >> tmp) {
            tmps.push_back(tmp);
        }
        
        for (auto& elem: tmps) {
            auto row = &elem - &tmps[0];
            callback(elem, c, (int)column, (int)row);
            c++;
        }
    }
}

NAMESPACE_BEGIN(nanogui)

class VirtualKeyboardButtonLayoutConfig {
public:
    VirtualKeyboardButtonLayoutConfig(VirtualKeyboardButtonConfig config) {
        m_config = config;
        
        for (auto &line: config) {
            std::istringstream iss(line, std::istringstream::in);
            std::string tmp;
            std::vector<std::string> tmps;
            
            while (iss >> tmp) {
                tmps.push_back(tmp);
            }
            
            m_data.push_back(tmps);
        }
    }
    
    int number_of_rows() const {
        return (int)m_data.size();
    }
    
    int number_of_items(int row) const {
        return (int)m_data[row].size();
    }
    
    std::vector<std::string> items_in_row(int row) const {
        return m_data[row];
    }
    
private:
    VirtualKeyboardButtonConfig m_config;
    std::vector<std::vector<std::string>> m_data;
};

class VirtualKeyboardLayout {
public:
    void perform_layout(NVGcontext *ctx, VirtualKeyboard* keyboard) {
        VirtualKeyboardButtonLayoutConfig config(test);
        
        int counter = 0;
        int padding = 10;
        int height = (keyboard->height() - padding * (config.number_of_rows() + 1)) / config.number_of_rows() ;
        
        for (int i = 0; i < config.number_of_rows(); i++) {
            std::vector<std::pair<Widget*, Vector4f>> tmp;
            
            int number_of_items = config.number_of_items(i);
            
            for (int j = 0; j < number_of_items; j++) {
                auto child = keyboard->child_at(counter);
                auto size = child->preferred_size(ctx);
                tmp.push_back({child, { 0, 0, (float)size.x(), (float)size.y() }});
                counter++;
            }
            
            int width = std::accumulate(tmp.begin(), tmp.end(), 0, [](auto a, auto b) { return a + b.second.z(); });
            
            float multiplier = (float)(keyboard->width() - padding * (number_of_items + 1)) / (float)width;
            
            for (auto &pair: tmp) {
                auto index = &pair - &tmp[0];
                pair.second[1] = padding + i * padding + i * height;
                pair.second[2] = pair.second.z() * multiplier;
                pair.second[3] = height;
                
                if (index == 0) {
                    pair.second[0] = padding;
                } else {
                    pair.second[0] = padding + tmp[index - 1].second.x() + tmp[index - 1].second.z();
                }
            }
            
            for (auto &pair: tmp) {
                pair.first->set_position({ (int)round(pair.second.x()), (int)round(pair.second.y()) });
                pair.first->set_size({ (int)round(pair.second.z()), (int)round(pair.second.w()) });
            }
        }
    }
};

class VirtualKeyboardButton : public Widget {
public:
    VirtualKeyboardButton(Widget *parent, const std::string &caption) : Widget(parent) {
        m_caption = caption;
    }
    
    Vector2i preferred_size(NVGcontext *ctx) const override {
        if (m_caption.size() == 1) {
            return {30, 40};
        }
        if (m_caption.at(0) == '{') {
            return {80, 40};
        }
        
        int font_size = m_font_size == -1 ? m_theme->m_button_font_size : m_font_size;
        nvgFontSize(ctx, font_size);
        nvgFontFace(ctx, "sans-bold");
        float width = nvgTextBounds(ctx, 0,0, m_caption.c_str(), nullptr, nullptr);
        return Vector2i(width + 20, font_size + 10);
    }
    
    void draw(NVGcontext *ctx) override {
        Widget::draw(ctx);
        
        nvgFillColor(ctx, Color(181, 181, 181, 255));
        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, position().x(), position().y(), width(), height(), 5);
        nvgFill(ctx);
        
        if (m_is_pressed) {
            nvgFillColor(ctx, Color(239, 239, 239, 255));
        } else {
            nvgFillColor(ctx, Color(255, 255, 255, 255));
        }
        
        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, position().x(), position().y(), width(), height() - 1, 5);
        nvgFill(ctx);
        
        int font_size = m_font_size == -1 ? m_theme->m_button_font_size : m_font_size;
        nvgFontSize(ctx, font_size);
        nvgFontFace(ctx, "sans");
        float tw = nvgTextBounds(ctx, 0,0, m_caption.c_str(), nullptr, nullptr);
        
        Vector2f center = Vector2f(m_pos) + Vector2f(m_size) * 0.5f;
        Vector2f text_pos(center.x() - tw * 0.5f, center.y() - 1);
        
        nvgFontSize(ctx, font_size);
        nvgFontFace(ctx, "sans");
        nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
        nvgFillColor(ctx, Color(33, 37, 41, 255));
        nvgText(ctx, text_pos.x(), text_pos.y(), m_caption.c_str(), nullptr);
    }
    
    bool mouse_button_event(const Vector2i &p, int button, bool down, int modifiers) override {
        if (down && !m_is_pressed) {
            nanogui::keyboard_character_event('1');
            m_is_pressed = true;
        } else if (!down && m_is_pressed) {
            m_is_pressed = false;
        }
        return true;
    }

private:
    std::string m_caption;
    bool m_is_pressed = false;
};

VirtualKeyboard::VirtualKeyboard(Widget *parent) : Widget(parent) {
    set_fixed_size(parent->screen()->size());
    
    for_each(test, [this](auto caption, int, int, int) {
        auto button = add<VirtualKeyboardButton>(caption);
//        button->set_callback([]() {
//            nanogui::keyboard_character_event('r');
//            printf("Tap...\n");
//        });
    });
    
    screen()->perform_layout();
}

Vector2i VirtualKeyboard::preferred_size(NVGcontext *ctx) const {
    return { screen()->size().x(), screen()->size().y() / 2 };
}

void VirtualKeyboard::perform_layout(NVGcontext *ctx) {
    Widget::perform_layout(ctx);
    
    VirtualKeyboardLayout layout;
    layout.perform_layout(ctx, this);
}

void VirtualKeyboard::draw(NVGcontext *ctx) {
    //nvgSave(ctx);
   // nvgReset(ctx);
    
    // Draw bg
    nvgFillColor(ctx, Color(236, 236, 236, 255));
    nvgBeginPath(ctx);
    nvgRect(ctx, 0, position().y(), width(), height());
    nvgFill(ctx);
    
    //VirtualKeyboardLayout layout;
    //layout.draw(ctx, preferred_size(ctx));
    
    //nvgTranslate(ctx, 0, -height());
    
    
   // nvgRestore(ctx);
    
    Widget::draw(ctx);
}

NAMESPACE_END(nanogui)
