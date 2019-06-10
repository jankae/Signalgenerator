#ifndef GUI_CHECKBOX_H_
#define GUI_CHECKBOX_H_

#include "widget.h"
#include "display.h"

class Checkbox : public Widget {
public:
	Checkbox(bool *value, void (*cb)(Widget&), coords_t size = COORDS(29, 29));

private:
	void draw(coords_t offset) override;
	void input(GUIEvent_t *ev) override;

	Widget::Type getType() override { return Widget::Type::Checkbox; };

	static constexpr color_t Background = COLOR_BG_DEFAULT;
	static constexpr color_t Border = COLOR_FG_DEFAULT;
	static constexpr color_t Ticked = COLOR(0, 192, 0);
	static constexpr color_t Unticked = COLOR(238, 0, 0);

    void (*callback)(Widget& source);
    bool *value;
};

#endif
