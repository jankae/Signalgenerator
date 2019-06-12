#include "MenuChooser.hpp"

#include "buttons.h"
#include "cast.hpp"
#include "Dialog/ItemChooserDialog.hpp"

MenuChooser::MenuChooser(const char* name, const char* const * items,
		uint8_t* value, Callback cb, void *ptr) {
	/* set member variables */
	this->cb = cb;
	this->ptr = ptr;
	this->items = items;
	this->value = value;
	strncpy(this->name, name, MaxNameLength);
	this->name[MaxNameLength] = 0;
	selectable = false;
}

void MenuChooser::draw(coords_t offset) {
	display_SetForeground(Foreground);
	display_SetBackground(Background);
	display_AutoCenterString(name, COORDS(offset.x, offset.y + 2),
			COORDS(offset.x + size.x, offset.y + 20));
	display_SetFont(*fontItems);
	int16_t yItemString = offset.y + size.y - fontItems->height - 2;
	display_String(offset.x, yItemString, items[*value]);
//	if (selected) {
//		// draw all available choices to the left
//		/* find number of items and longest item */
//		uint8_t maxItemLength = 0;
//		uint8_t numItems;
//		for (numItems = 0; this->items[numItems]; numItems++) {
//			uint8_t length = strlen(this->items[numItems]);
//			if (length > maxItemLength)
//				maxItemLength = length;
//		}
//
//		// calculate size of required box
//		coords_t box_size = COORDS(maxItemLength * fontItems->width + 1,
//				numItems * fontItems->height + 1);
//		coords_t top_right = COORDS(offset.x - 4, yItemString - 1);
//		int16_t display_margin = DISPLAY_HEIGHT - 1 - top_right.y + box_size.y;
//		if (display_margin < 0) {
//			// box would extend below the display, move upwards
//			top_right.y += display_margin;
//		}
//		display_Rectangle(top_right.x - box_size.x, top_right.y, top_right.x,
//				top_right.y + box_size.y);
//		display_SetFont(*fontItems);
//		for (numItems = 0; this->items[numItems]; numItems++) {
//			if(numItems == *value) {
//				display_SetBackground(COLOR_SELECTED);
//			} else {
//				display_SetBackground(Background);
//			}
//			display_String(top_right.x - box_size.x + 1,
//					top_right.y + numItems * fontItems->height + 1,
//					items[numItems]);
//		}
//	}
}

void MenuChooser::input(GUIEvent_t* ev) {
	uint8_t numItems;
	for (numItems = 0; this->items[numItems]; numItems++);

	switch(ev->type) {
	case EVENT_BUTTON_CLICKED:
		if (!(ev->button & BUTTON_ENCODER)) {
			break;
		}
		/* no break */
	case EVENT_TOUCH_PRESSED:
		new ItemChooserDialog("Select setting", items, *value,
				pmf_cast<void (*)(void*, bool, uint8_t), MenuChooser,
						&MenuChooser::ChooserCallback>::cfn, this);
		ev->type = EVENT_NONE;
		break;
	}
}

void MenuChooser::ChooserCallback(bool updated, uint8_t newval) {
	if (updated) {
		*value = newval;
		if (cb) {
			cb(ptr, this);
		}
	}
}
