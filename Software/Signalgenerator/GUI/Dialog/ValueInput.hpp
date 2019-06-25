#pragma once

#include <cstdint>
#include "Unit.hpp"
#include "widget.hpp"
#include "window.hpp"
#include "button.hpp"

class ValueInput {
public:
	using Callback = void (*)(void *ptr, bool OK);
	ValueInput(const char *title, int32_t *value, const Unit::unit *unit[],
			Callback cb, void *ptr, char firstChar = 0);
	~ValueInput();
private:
	void UnitPressed(Widget *w);
	void NumberPressed(Widget *w);
	void CharInput(char c);
	void ChangeSign(Widget *w);
	void Backspace(Widget *w);
	void EventCaught(Widget *w, GUIEvent_t *ev);
	Window *w;
	Label *l;
	Button *dot;
	Callback cb;
	void *ptr;
	const Unit::unit **unit;
	int32_t *value;
	static constexpr uint8_t maxInputLength = 10;
	char string[maxInputLength + 1];
	uint8_t inputIndex;
};
