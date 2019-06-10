#pragma once

#include "widget.h"
#include "display.h"
#include "font.h"
#include "common.h"

class Scopescreen : public Widget {
	friend void Widget::requestRedrawChildren();
public:
	Scopescreen(coords_t size, uint8_t horizontalDivs, uint8_t verticalDivs);
	~Scopescreen();

	int8_t AddLine(color_t c, int32_t scale, int32_t offset);
	void ChangeLineSetting(uint8_t line, int32_t scale, int32_t offset);
	void AddLineData(uint8_t line, int32_t data);

	void UpdateDisplay(void) {
		updateOnly = true;
		Widget::requestRedraw();
	}

	void UpdateFull() {
		firstDraw = true;
	}

private:
	void draw(coords_t offset) override;
	void input(GUIEvent_t *ev) override;

	Widget::Type getType() override { return Widget::Type::Scopescreen; };

	static constexpr color_t DivLines = COLOR_GRAY;
	static constexpr color_t Background = COLOR_BLACK;

	static constexpr uint8_t maxLines = 4;

	const Type type = Type::Scopescreen;

	uint8_t horDiv, vertDiv;
	bool updateOnly;
	bool firstDraw;

	using Line = struct {
		color_t c;
		uint16_t ringBufferOffset;
		int32_t scale;
		int32_t offset;
		int32_t *data;
		int16_t *oldYCoord;
	};

	uint8_t lineCnt;
	Line lines[maxLines];
};

