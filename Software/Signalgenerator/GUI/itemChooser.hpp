#ifndef ITEMCHOOSER_HPP_
#define ITEMCHOOSER_HPP_

#include <widget.hpp>
#include "display.h"

class ItemChooser : public Widget {
public:
	ItemChooser(const char * const *items, uint8_t *value, font_t font,
			uint8_t visibleLines, uint16_t minSizeX = 0);

	void setCallback(Callback cb, void *ptr) {
		this->cb = cb;
		this->ptr = ptr;
	}
private:
	void draw(coords_t offset) override;
	void input(GUIEvent_t *ev) override;

	Widget::Type getType() override { return Widget::Type::ItemChooser; };

	static constexpr uint8_t ScrollbarSize = 20;
	static constexpr color_t ScrollbarColor = COLOR_ORANGE;
	static constexpr color_t Border = COLOR_FG_DEFAULT;
	static constexpr color_t Selected = COLOR(100, 100, 100);
	static constexpr color_t Background = COLOR_BG_DEFAULT;

    uint8_t *value;
    const char * const * itemlist;
    font_t font;
    uint8_t lines;
    Callback cb;
    void *ptr;
    uint8_t topVisibleEntry;
};

#endif
