#include "System.hpp"

#include "gui.hpp"
#include "window.hpp"
#include "textfield.hpp"
#include "EventCatcher.hpp"

void System::ViewFont() {
	Window *w = new Window("Font", Font_Big, SIZE(100, 157));
	char font[256 + 16];
	for (uint8_t i = 0; i < 16; i++) {
		for (uint8_t j = 0; j < 16; j++) {
			font[i * 17 + j] = i * 16 + j;
		}
		font[i * 17 + 16] = '\n';
	}
	font[0] = ' ';
	font[sizeof(font) - 1] = 0;
	Textfield *t = new Textfield(font, Font_Medium);

	w->setMainWidget(t);
	w->select();
}
