#include "checkbox.h"

#include "buttons.h"

Checkbox::Checkbox(bool *value, void (*cb)(Widget&), coords_t size) {
    this->value = value;
    callback = cb;
    this->size = size;
}

void Checkbox::draw(coords_t offset) {
	/* calculate corners */
	coords_t upperLeft = offset;
	coords_t lowerRight = upperLeft;
	lowerRight.x += size.x - 1;
	lowerRight.y += size.y - 1;
	if (selected) {
		display_SetForeground(COLOR_SELECTED);
	} else {
		display_SetForeground (Border);
	}
	display_Rectangle(upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y);
	if (*value) {
		display_SetForeground (Ticked);
		display_Line(upperLeft.x + 2, lowerRight.y - size.y / 3,
				upperLeft.x + size.x / 3, lowerRight.y - 2);
		display_Line(upperLeft.x + 2, lowerRight.y - size.y / 3 - 1,
				upperLeft.x + size.x / 3 + 1, lowerRight.y - 2);
		display_Line(upperLeft.x + 2, lowerRight.y - size.y / 3 - 2,
				upperLeft.x + size.x / 3 + 2, lowerRight.y - 2);
		display_Line(lowerRight.x - 2, upperLeft.y + 2,
				upperLeft.x + size.x / 3, lowerRight.y - 2);
		display_Line(lowerRight.x - 2, upperLeft.y + 3,
				upperLeft.x + size.x / 3 + 1, lowerRight.y - 2);
		display_Line(lowerRight.x - 2, upperLeft.y + 4,
				upperLeft.x + size.x / 3 + 2, lowerRight.y - 2);
	} else {
		display_SetForeground (Unticked);
		display_Line(upperLeft.x + 3, upperLeft.y + 3, lowerRight.x - 3,
				lowerRight.y - 3);
		display_Line(upperLeft.x + 4, upperLeft.y + 3, lowerRight.x - 3,
				lowerRight.y - 4);
		display_Line(upperLeft.x + 3, upperLeft.y + 4, lowerRight.x - 4,
				lowerRight.y - 3);
		display_Line(upperLeft.x + 3, lowerRight.y - 3, lowerRight.x - 3,
				upperLeft.y + 3);
		display_Line(upperLeft.x + 4, lowerRight.y - 3, lowerRight.x - 3,
				upperLeft.y + 4);
		display_Line(upperLeft.x + 3, lowerRight.y - 4, lowerRight.x - 4,
				upperLeft.y + 3);
	}
}

void Checkbox::input(GUIEvent_t *ev) {
	switch(ev->type) {
	case EVENT_BUTTON_CLICKED:
		if(ev->button!=BUTTON_UNIT1 && ev->button != BUTTON_ENCODER) {
			break;
		}
		/* no break */
	case EVENT_TOUCH_RELEASED:
		*value = !*value;
		requestRedrawFull();
		if (callback)
			callback(*this);
		ev->type = EVENT_NONE;
		break;
	default:
		break;
	}
	return;
}
