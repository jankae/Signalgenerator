/*
 * menu.hpp
 *
 *  Created on: Jun 9, 2019
 *      Author: jan
 */

#ifndef MENU_HPP_
#define MENU_HPP_

#include "widget.h"
#include "display.h"
#include "menuentry.hpp"

class Menu : public Widget {
public:
	Menu(coords_t size);
	bool AddEntry(MenuEntry *e);
	bool AddWidget(Widget *w);
	coords_t getAvailableArea();
private:
	void draw(coords_t offset) override;
	void input(GUIEvent_t *ev) override;
	void drawChildren(coords_t offset) override;

	Widget::Type getType() override { return Widget::Type::MenuEntry; };

	void PageSwitched();

	static constexpr uint16_t EntrySizeX = 60;
	static constexpr uint16_t EntrySizeY = 40;
	static constexpr color_t Foreground = COLOR_FG_DEFAULT;
	static constexpr color_t Selected = COLOR_SELECTED;
	static constexpr color_t Background = COLOR_BG_DEFAULT;

    uint8_t nentries, selectedEntry;
    uint8_t entriesPerPage;
    bool usePages;
    MenuEntry *firstEntry;
};


#endif /* MENU_HPP_ */
