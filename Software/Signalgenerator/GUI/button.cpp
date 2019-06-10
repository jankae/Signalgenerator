#include "button.h"
#include "buttons.h"

Button::Button(const char *name, font_t font, void (*cb)(Widget&),
		uint16_t minWidth) {
	/* set name and callback */
	uint16_t namelength = strlen(name);
	this->name = new char[namelength + 1];
	memcpy(this->name, name, namelength + 1);

	callback = cb;
	this->font = font;
	pressed = false;

	/* calculate size based on the font */
	size.y = font.height + 6;
	size.x = font.width * namelength + 5;

	if (minWidth > size.x)
		size.x = minWidth;

	/* calculate font start position */
	fontStart.y = 3;
	fontStart.x = (size.x - font.width * namelength - 1) / 2;
}

Button::~Button() {
	/* Free allocated memory */
	delete name;
}

void Button::draw(coords_t offset) {
    /* calculate corners */
    coords_t upperLeft = offset;
    coords_t lowerRight = upperLeft;
    lowerRight.x += size.x - 1;
    lowerRight.y += size.y - 1;
    /* draw outline */
	if (selected) {
		display_SetForeground(COLOR_SELECTED);
	} else if(selectable){
		display_SetForeground(Foreground);
	} else {
		display_SetForeground(COLOR_GRAY);
	}
    display_VerticalLine(upperLeft.x, upperLeft.y + 1, size.y - 2);
    display_VerticalLine(lowerRight.x, upperLeft.y + 1, size.y - 2);
    display_HorizontalLine(upperLeft.x + 1, upperLeft.y, size.x - 2);
    display_HorizontalLine(upperLeft.x + 1, lowerRight.y, size.x - 2);
	if (selectable) {
		if (!pressed)
			display_SetForeground(color_Tint(Foreground, Background, 75));
		else
			display_SetForeground(color_Tint(Foreground, COLOR_WHITE, 200));
	}
	display_VerticalLine(lowerRight.x - 1, upperLeft.y + 1, size.y - 2);
	display_HorizontalLine(upperLeft.x + 1, lowerRight.y - 1, size.x - 2);

//    display_SetForeground(color_Tint(fg, bg, 150));
//    display_VerticalLine(lowerRight.x - 2, upperLeft.y + 1, b->base.size.y - 3);
//    display_HorizontalLine(upperLeft.x + 1, lowerRight.y - 2,
//            b->base.size.x - 3);

	if (selectable) {
		if (pressed)
			display_SetForeground(
					color_Tint(Foreground, Background, 75));
		else
			display_SetForeground(color_Tint(Foreground, COLOR_WHITE, 200));
	}
	display_VerticalLine(upperLeft.x + 1, upperLeft.y + 1, size.y - 3);
	display_HorizontalLine(upperLeft.x + 1, upperLeft.y + 1, size.x - 3);

	if (name) {
		if (selectable) {
			display_SetForeground(Foreground);
		} else {
			display_SetForeground(COLOR_GRAY);
		}
		display_SetBackground(Background);
		display_SetFont(font);
		display_String(upperLeft.x + fontStart.x, upperLeft.y + fontStart.y,
				name);
	}
//    if (b->base.flags.selected) {
//        /* invert button area */
//        screen_FullRectangle(upperLeft.x + 1, upperLeft.y + 1,
//                lowerRight.x - 2, lowerRight.y - 2, PIXEL_INVERT);
//    }
    return;
}

void Button::input(GUIEvent_t *ev) {
    switch(ev->type) {
    case EVENT_TOUCH_PRESSED:
		if (!pressed) {
			pressed = true;
			requestRedraw();
		}
		break;
    case EVENT_TOUCH_RELEASED:
		if (pressed) {
			pressed = false;
			requestRedraw();
			if (callback)
				callback(*this);
		}
		break;
	case EVENT_BUTTON_CLICKED:
		if (ev->button & (BUTTON_UNIT1 | BUTTON_ENCODER)) {
			if (callback)
				callback(*this);
		}
		break;
	default:
    	break;
	}
    return;
}
