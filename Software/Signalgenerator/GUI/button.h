#ifndef GUI_BUTTON_H_
#define GUI_BUTTON_H_

#include "widget.h"
#include "display.h"
#include "font.h"
#include "common.h"

class Button : public Widget {
public:
	Button(const char *name, font_t font, void (*cb)(Widget&), uint16_t minWidth = 0);
	~Button();

	char *getName() {
		return name;
	}
private:
	void draw(coords_t offset) override;
	void input(GUIEvent_t *ev) override;

	Widget::Type getType() override { return Widget::Type::Button; };

	static constexpr color_t Foreground = COLOR_BLACK;
	static constexpr color_t Background = COLOR_BG_DEFAULT;

    void (*callback)(Widget& source);
    char *name;
    font_t font;
    coords_t fontStart;
    bool pressed;
};

#endif
