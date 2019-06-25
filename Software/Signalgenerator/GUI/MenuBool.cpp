#include "MenuBool.hpp"

#include "buttons.h"

MenuBool::MenuBool(const char *name, bool *value, void (*cb)(Widget&)) {
	this->value = value;
	strncpy(this->name, name, MaxNameLength);
	this->name[MaxNameLength] = 0;
	this->callback = cb;
	selectable = false;
}

void MenuBool::draw(coords_t offset) {
	uint16_t shiftX = (size.x - strlen(name) * fontName->width) / 2;
	display_SetFont(*fontName);
	display_SetForeground(Foreground);
	display_SetBackground(Background);
	display_String(offset.x + shiftX, offset.y + 1, name);
	display_SetFont(*fontValue);
	if (*value) {
		display_SetForeground(ColorOn);
		shiftX = (size.x - 2 * fontValue->width) / 2;
		display_String(offset.x + shiftX, offset.y + size.y - fontValue->height - 3,
				"ON");
	} else {
		display_SetForeground(ColorOff);
		shiftX = (size.x - 3 * fontValue->width) / 2;
		display_String(offset.x + shiftX, offset.y + size.y - fontValue->height - 3,
				"OFF");
	}
}
void MenuBool::input(GUIEvent_t *ev) {
	switch(ev->type) {
	case EVENT_BUTTON_CLICKED:
		if (!(ev->button & (BUTTON_ENCODER | BUTTON_LEFT | BUTTON_RIGHT))) {
			// some button was pressed that has no functionality for menu bool
			break;
		}
		/* no break */
	case EVENT_TOUCH_PRESSED:
		*value = !*value;
		requestRedrawFull();
		if (callback)
			callback(*this);
		break;
	}
	ev->type = EVENT_NONE;
}
