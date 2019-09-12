#ifndef GUI_SEVEN_H_
#define GUI_SEVEN_H_

#include <common.hpp>
#include <widget.hpp>
#include "display.h"
//#include "font.h"
//#include "dialog.h"

template<typename T>
class SevenSegment : public Widget {
public:
	SevenSegment(T *value, uint8_t sLength, uint8_t sWidth,
			uint8_t length, uint8_t dot, color_t color) {
		/* set member variables */
		this->value = value;
		segmentLength = sLength;
		segmentWidth = sWidth;
		this->length = length;
		this->dot = dot;
		this->color = color;
		this->selectable = false;

		uint16_t height = sWidth + 2 * sLength + 4;
		uint16_t digitWidth = sWidth + sLength;
		size.y = height;
		size.x = digitWidth * length + sWidth * (length - 1) + 2;
	}

private:
	void draw_Digit(int16_t x, int16_t y, uint8_t digit) {
		uint8_t i;
		for (i = 0; i < 7; i++) {
			/* Select color for this segment */
			if ((1 << i) & digitToSegments[digit]) {
				display_SetForeground(color);
			} else {
				display_SetForeground (Background);
			}

			/* draw the segment */
			int16_t offsetX = x + segmentStartX[i] * (segmentLength + 1);
			int16_t offsetY = y + segmentStartY[i] * (segmentLength + 1);

			if ((1 << i) & segmentOrientation) {
				/* this is a horizontal segment */
				display_HorizontalLine(offsetX + 1, offsetY, segmentLength);
				uint8_t j;
				for (j = 1; j <= segmentWidth / 2; j++) {
					display_HorizontalLine(offsetX + j + 1, offsetY + j,
							segmentLength - 2 * j);
					display_HorizontalLine(offsetX + j + 1, offsetY - j,
							segmentLength - 2 * j);
				}
			} else {
				/* this is a vertical segment */
				display_VerticalLine(offsetX, offsetY + 1, segmentLength);
				uint8_t j;
				for (j = 1; j <= segmentWidth / 2; j++) {
					display_VerticalLine(offsetX + j, offsetY + j + 1,
							segmentLength - 2 * j);
					display_VerticalLine(offsetX - j, offsetY + j + 1,
							segmentLength - 2 * j);
				}
			}
		}
	}

	void draw(coords_t offset) override {
		T buf = *value;
		uint8_t neg = 0;
		if (buf < 0) {
			buf = -buf;
			neg = 1;
		}
		uint8_t i;
		int16_t x = offset.x
				+ (length - 1) * (segmentLength + 2 * segmentWidth)
				+ segmentWidth / 2;
		int16_t y = offset.y + segmentWidth / 2;
		for (i = 0; i < length; i++) {
			if (i == length - 1) {
				/* this is the negative sign position */
				if (neg) {
					draw_Digit(x, y, 10);
				} else {
					draw_Digit(x, y, 11);
				}
			} else {
				draw_Digit(x, y, buf % 10);
			}
			if (dot && (dot == i + 1)) {
				/* draw dot in front of current digit */
				int16_t offsetX = x - segmentWidth;
				int16_t offsetY = y + 2 * (segmentLength + 1)
						- segmentWidth / 2;
				display_SetForeground(color);
				display_VerticalLine(offsetX, offsetY + 1, segmentWidth + 1);
				uint8_t j;
				for (j = 1; j <= segmentWidth / 2 + 1; j++) {
					display_VerticalLine(offsetX + j, offsetY + j + 1,
							segmentWidth - 2 * j + 1);
					display_VerticalLine(offsetX - j, offsetY + j + 1,
							segmentWidth - 2 * j + 1);
				}
			}
			x -= segmentLength + 2 * segmentWidth;
			buf /= 10;
		}
	}

	Widget::Type getType() override { return Widget::Type::Sevensegment; };

	static constexpr color_t Background = COLOR_BG_DEFAULT;
	/* Index			Symbol
	 * 0-9				0-9
	 * 10				Negative-sign
	 * 11				Blank
	 */
	static constexpr inline uint8_t digitToSegments[12] = {
			0b00111111,
			0b00000110,
			0b01011011,
			0b01001111,
			0b01100110,
			0b01101101,
			0b01111101,
			0b00000111,
			0b01111111,
			0b01101111,
			0b01000000,
			0b00000000,
	};
	/* 0 = vertical, 1 = horizontal */
	static constexpr inline uint8_t segmentOrientation = 0b01001001;
	static constexpr inline uint8_t segmentStartX[7] = {
			0, 1, 1, 0, 0, 0, 0
	};
	static constexpr inline uint8_t segmentStartY[7] = {
			0, 0, 1, 2, 1, 0, 1
	};

    T *value;
    uint8_t segmentLength;
    uint8_t segmentWidth;
    uint8_t length;
    uint8_t dot;
    color_t color;
};



#endif
