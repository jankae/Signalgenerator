#include "entry.h"

#include "buttons.h"

Entry::Entry(int32_t *value, const int32_t *max, const int32_t *min,
		font_t font, uint8_t length, const unit_t *unit, const color_t c) {
	/* set member variables */
	this->value = value;
	this->max = max;
	this->min = min;
	this->font = font;
	this->unit = unit;
	this->length = length;
    changeCallback = NULL;
    editing = false;
    dotSet = false;
    editPos = 0;
	size.y = font.height + 3;
	size.x = font.width * length + 3;
	inputString = new char[length + 1];
	color = c;
}

Entry::~Entry() {
	if(inputString) {
		delete inputString;
	}
}

int32_t Entry::constrainValue(int32_t val) {
    if (max && val > *max) {
        return *max;
    } else if (min && val < *min) {
        return *min;
    }
    return val;
}

int32_t Entry::InputStringValue(uint32_t multiplier) {
    int64_t value = 0;
    uint8_t i;
    uint32_t div = 0;
    for (i = 0; i < length; i++) {
        if (inputString[i] >= '0' && inputString[i] <= '9') {
            value *= 10;
            value += inputString[i] - '0';
            if (div) {
                div *= 10;
            }
        } else if (inputString[i] == '.') {
            div = 1;
        }
    }
    value *= multiplier;
    if (div) {
        value /= div;
    }
	if (inputString[0] == '-') {
		value = -value;
	}
    return value;
}

void Entry::draw(coords_t offset) {
    /* calculate corners */
    coords_t upperLeft = offset;
	coords_t lowerRight = upperLeft;
	lowerRight.x += size.x - 1;
	lowerRight.y += size.y - 1;
	if (selected) {
		display_SetForeground(COLOR_SELECTED);
	} else {
		display_SetForeground(Border);
	}
	display_Rectangle(upperLeft.x, upperLeft.y, lowerRight.x, lowerRight.y);

	/* display string */
	if (!editing) {
		/* construct value string */
		common_StringFromValue(inputString, length, *value, unit);
		if (selectable) {
			display_SetForeground(color);
		} else {
			display_SetForeground(COLOR_GRAY);
		}
	} else {
		display_SetForeground(COLOR_SELECTED);
	}
	if (selectable) {
		display_SetBackground(Background);
	} else {
		display_SetBackground(COLOR_UNSELECTABLE);
	}
	display_SetFont(font);
	display_String(upperLeft.x + 1, upperLeft.y + 2, inputString);

}
void Entry::input(GUIEvent_t *ev) {
    switch(ev->type) {
    case EVENT_BUTTON_CLICKED:
		if (BUTTON_IS_INPUT(ev->button)) {
			ev->type = EVENT_NONE;
			requestRedraw();
			if (!editing) {
				/* Start editing */
				editing = true;
				editPos = 0;
				dotSet = false;
				memset(inputString, ' ', length);
				inputString[length] = 0;
			}
			/* Add button input to inputString */
			if (ev->button == BUTTON_DOT) {
				if (editPos < length && !dotSet) {
					/* add dot */
					inputString[editPos++] = '.';
					dotSet = true;
				}
			} else if (ev->button == BUTTON_SIGN) {
				/* toggle sign */
				if(inputString[0] == '-') {
					/* remove sign */
					memmove(inputString, &inputString[1], editPos - 1);
					editPos--;
					inputString[editPos] = ' ';
				} else if(editPos < length) {
					/* add sign */
					memmove(&inputString[1], &inputString[0], editPos);
					inputString[0] = '-';
					editPos++;
				}
			} else {
				/* must be a number input */
				if(editPos < length) {
					inputString[editPos++] = '0' + BUTTON_TODIGIT(ev->button);
				}
			}
		} else if (ev->button == BUTTON_DEL && editing) {
			/* delete one char from input string */
			if (editPos <= 1) {
				/* string will be empty after deletion -> abort editing */
				editing = false;
			} else {
				editPos--;
				if (inputString[editPos] == '.') {
					/* deleted dot */
					dotSet = false;
				}
				inputString[editPos] = ' ';
			}
			requestRedraw();
			ev->type = EVENT_NONE;
		} else if (ev->button == BUTTON_ESC && editing) {
			editing = false;
			requestRedraw();
			ev->type = EVENT_NONE;
		} else if ((ev->button & (BUTTON_UNIT1 | BUTTON_ENCODER | BUTTON_UNITm))
				&& editing) {
			editing = false;
			/* TODO adjust multiplier to unit */
			uint32_t multiplier;
			if (unit == &Unit_Time) {
				multiplier = 1000;
				if (ev->button == BUTTON_UNITm) {
					multiplier = 1;
				}
			} else {
				multiplier = 1000000;
				if (ev->button == BUTTON_UNITm) {
					multiplier = 1000;
				}
			}
			int32_t newval = InputStringValue(multiplier);
			*value = constrainValue(newval);
			if (changeCallback) {
				changeCallback(*this);
			}
			requestRedraw();
			ev->type = EVENT_NONE;
		}
		break;
    case EVENT_ENCODER_MOVED:
    	if(!editing) {
			int32_t newval = *value += ev->movement
					* common_LeastDigitValueFromString(inputString, unit);
			*value = constrainValue(newval);
			if(changeCallback) {
				changeCallback(*this);
			}
			requestRedraw();
			ev->type = EVENT_NONE;
    	}
    	break;
    default:
    	break;
    }
    return;
}
