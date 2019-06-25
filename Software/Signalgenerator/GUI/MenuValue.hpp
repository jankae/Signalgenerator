#pragma once

#include "menuentry.hpp"
#include "Unit.hpp"

class MenuValue: public MenuEntry {
public:
	MenuValue(const char *name, int32_t *value, const Unit::unit *unit[],
			Callback cb = nullptr, void *ptr = nullptr);

private:
	void draw(coords_t offset) override;
	void input(GUIEvent_t *ev) override;
	void ValueCallback(bool updated);

	Widget::Type getType() override { return Widget::Type::MenuValue; };

	static constexpr color_t Background = COLOR_BG_DEFAULT;
	static constexpr color_t Foreground = COLOR_FG_DEFAULT;
	static constexpr uint8_t MaxNameLength = 10;
	static constexpr const font_t *fontValue = &Font_Medium;

	Callback cb;
	void *ptr;
	int32_t *value;
    char name[MaxNameLength + 1];
    const Unit::unit **unit;
};
