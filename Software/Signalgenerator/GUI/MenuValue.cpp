#include "MenuValue.hpp"
#include "buttons.h"
#include "Dialog/ValueInput.hpp"
#include "cast.hpp"

MenuValue::MenuValue(const char* name, int32_t* value, const Unit::unit* unit[],
		Callback cb, void* ptr) {
	/* set member variables */
	this->cb = cb;
	this->ptr = ptr;
	this->unit = unit;
	this->value = value;
	strncpy(this->name, name, MaxNameLength);
	this->name[MaxNameLength] = 0;
	selectable = false;
}

void MenuValue::draw(coords_t offset) {
	display_SetForeground(Foreground);
	display_SetBackground(Background);
	display_AutoCenterString(name, COORDS(offset.x, offset.y),
			COORDS(offset.x + size.x, offset.y + size.y / 2));
	uint8_t len = size.x / fontValue->width;
	char s[len + 1];
	Unit::StringFromValue(s, len, *value, unit);
	display_AutoCenterString(s, COORDS(offset.x, offset.y + size.y / 2),
			COORDS(offset.x + size.x, offset.y + size.y));
}

void MenuValue::input(GUIEvent_t* ev) {
	char firstChar = 0;
	switch(ev->type) {
	case EVENT_BUTTON_CLICKED:
		if(BUTTON_IS_DIGIT(ev->button)) {
			firstChar = BUTTON_TODIGIT(ev->button) + '0';
		} else if(ev->button & BUTTON_DOT) {
			firstChar = '.';
		} else if(ev->button & BUTTON_SIGN) {
			firstChar = '-';
		} else if(!(ev->button & (BUTTON_ENCODER))) {
			break;
		}
		/* no break */
	case EVENT_TOUCH_PRESSED:
		new ValueInput("New value:", value, unit,
				pmf_cast<void (*)(void*, bool), MenuValue,
						&MenuValue::ValueCallback>::cfn, this, firstChar);
	}
	ev->type = EVENT_NONE;
}

void MenuValue::ValueCallback(bool updated) {
	if (updated) {
		if (cb) {
			cb(ptr, this);
		}
	}
}
