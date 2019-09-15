#ifndef CUSTOM_HPP_
#define CUSTOM_HPP_

#include <widget.hpp>

class Custom: public Widget {
public:
	Custom(coords_t size, void (*draw)(Custom&, coords_t),
			void (*input)(Custom&, GUIEvent_t *), void *ptr) {
		this->size = size;
		this->ptr = ptr;
		drawCB = draw;
		inputCB = input;
	}

	void *GetPtr() {
		return ptr;
	}

private:
	void draw(coords_t offset) override {
		if (drawCB) {
			drawCB(*this, offset);
		}
	}

	void input(GUIEvent_t *ev) override {
		if (inputCB) {
			inputCB(*this, ev);
		}
	}

	Widget::Type getType() override { return Widget::Type::Custom; };

	void (*drawCB)(Widget&, coords_t);
	void (*inputCB)(Widget&, GUIEvent_t*);
	void *ptr;
};

#endif
