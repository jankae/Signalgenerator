/*
 * MenuBool.hpp
 *
 *  Created on: Jun 9, 2019
 *      Author: jan
 */

#ifndef MENUBACK_HPP_
#define MENUBACK_HPP_

#include "menuentry.hpp"

class MenuBack : public MenuEntry {
public:
	MenuBack(){};

private:
	void draw(coords_t offset) override;

	Widget::Type getType() override { return Widget::Type::MenuBack; };

	static constexpr color_t Background = COLOR_BG_DEFAULT;
	static constexpr color_t Foreground = COLOR_FG_DEFAULT;
};



#endif /* MENUBOOL_HPP_ */
