#include "scopescreen.h"

Scopescreen::Scopescreen(coords_t size, uint8_t horizontalDivs, uint8_t verticalDivs) {
	this->size = size;
	horDiv = horizontalDivs;
	vertDiv = verticalDivs;
	selectable = false;
	lineCnt = 0;
	updateOnly = false;
	firstDraw = true;
}

Scopescreen::~Scopescreen() {
	for(uint8_t i=0;i<lineCnt;i++) {
		delete lines[i].data;
		delete lines[i].oldYCoord;
	}
}

int8_t Scopescreen::AddLine(color_t c, int32_t scale, int32_t offset) {
	if(lineCnt < maxLines) {
		lines[lineCnt].c = c;
		lines[lineCnt].ringBufferOffset = 0;
		lines[lineCnt].data = new int32_t[size.x];
		if (!lines[lineCnt].data) {
			return -1;
		}
		lines[lineCnt].oldYCoord = new int16_t[size.x];
		if (!lines[lineCnt].oldYCoord) {
			return -1;
		}
		for (uint16_t x = 0; x < size.x; x++) {
			lines[lineCnt].data[x] = 0;
		}
		lineCnt++;
		ChangeLineSetting(lineCnt - 1, scale, offset);
		return lineCnt - 1;
	} else {
		/* Already at maximum number of lines */
		return -1;
	}
}

void Scopescreen::AddLineData(uint8_t line, int32_t data) {
	if(line < lineCnt) {
		uint16_t index = (lines[line].ringBufferOffset + size.x) % size.x;
		lines[line].data[index] = data;
		lines[line].ringBufferOffset++;
	}
}

void Scopescreen::ChangeLineSetting(uint8_t line, int32_t scale, int32_t offset) {
	if(line < lineCnt) {
		lines[line].scale = scale;
		lines[line].offset = offset;
	}
	requestRedraw();
}

void Scopescreen::draw(coords_t offset) {
    /* calculate corners */
    coords_t upperLeft = offset;
    coords_t lowerRight = upperLeft;
    lowerRight.x += size.x - 1;
	lowerRight.y += size.y - 1;
	int16_t midX = size.x / 2;
	int16_t midY = size.y / 2;
	int16_t horDivSize = size.x / horDiv;
	int16_t vertDivSize = size.y / vertDiv;
	if (firstDraw) {
		updateOnly = false;
		firstDraw = false;
	}
	if (!updateOnly) {
		display_SetForeground(Background);
		display_RectangleFull(upperLeft.x, upperLeft.y, lowerRight.x,
				lowerRight.y);
		/* draw center lines */
		display_SetForeground(COLOR_WHITE);
		display_VerticalLine(upperLeft.x + midX, upperLeft.y, lowerRight.y);
		display_HorizontalLine(upperLeft.x, upperLeft.y + midY, lowerRight.x);
		/* draw division lines */
		display_SetForeground(DivLines);
		for (int16_t y = upperLeft.y + midY - vertDivSize; y >= upperLeft.y; y -=
				vertDivSize) {
			display_HorizontalLine(upperLeft.x, y, lowerRight.x);
		}
		for (int16_t y = upperLeft.y + midY + vertDivSize; y <= lowerRight.y; y +=
				vertDivSize) {
			display_HorizontalLine(upperLeft.x, y, lowerRight.x);
		}
		for (int16_t x = upperLeft.x + midX - horDivSize; x >= upperLeft.x; x -= horDivSize) {
			display_VerticalLine(x, upperLeft.y, lowerRight.y);
		}
		for (int16_t x = upperLeft.x + midX + horDivSize; x <= lowerRight.x; x += horDivSize) {
			display_VerticalLine(x, upperLeft.y, lowerRight.y);
		}
	}

	display_SetActiveArea(upperLeft.x, lowerRight.x, upperLeft.y, lowerRight.y);
	/* Draw data lines */
	for (uint8_t i = 0; i < lineCnt; i++) {
		display_SetForeground(lines[i].c);
		int16_t offset = -lines[i].offset * vertDivSize / lines[i].scale;
		/* Draw arrow indicating offset */
		constexpr uint8_t arrowSize = 5;
		for (uint8_t j = 0; j < arrowSize; j++) {
			display_VerticalLine(upperLeft.x + j,
					midY + offset - (arrowSize - j), 2 * (arrowSize - j));
		}
		/* Draw data points */
		for (uint16_t x = 0; x < size.x; x++) {
			uint16_t index = (x + lines[i].ringBufferOffset) % size.x;
			int16_t yCoord = midY + offset;
			yCoord -= lines[i].data[index] * vertDivSize / lines[i].scale;
			if (updateOnly) {
				int16_t oldY = lines[i].oldYCoord[x];
				if (yCoord != oldY) {
					/* Check if the old datapoint is already active in a different line.
					 * Otherwise it has to be overwritten with background color */
					uint8_t k;
					for (k = 0; k < i; k++) {
						if (lines[k].oldYCoord[x] == oldY)
							/* already in use in different line */
							break;
					}
					if (k >= i) {
						/* Need to overwrite old point with background */
						color_t c = COLOR_BLACK;
						if (x == midX || oldY == midY) {
							/* Background is a center line */
							c = COLOR_WHITE;
						} else if (abs(x - midX) % horDivSize == 0
								|| abs(oldY - midY) % vertDivSize == 0) {
							/* This coordinate is a division line */
							c = DivLines;
						}
						display_Pixel(upperLeft.x + x, upperLeft.y + oldY, c);
					}
				}
			}
			/* draw new point */
			display_Pixel(upperLeft.x + x, yCoord, lines[i].c);

			lines[i].oldYCoord[x] = yCoord;
		}
	}
	display_SetDefaultArea();
	updateOnly = false;
    return;
}

void Scopescreen::input(GUIEvent_t *ev) {

    return;
}
