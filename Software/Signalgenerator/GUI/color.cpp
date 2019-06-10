#include "color.h"

color_t color_Tint(color_t orig, color_t tint, uint8_t factor) {
	uint8_t r = COLOR_R(orig)
			+ (uint16_t) ((COLOR_R(tint) - COLOR_R(orig)) * factor) / 255;
	uint8_t g = COLOR_G(orig)
			+ (uint16_t) ((COLOR_G(tint) - COLOR_G(orig)) * factor) / 255;
	uint8_t b = COLOR_B(orig)
			+ (uint16_t) ((COLOR_B(tint) - COLOR_B(orig)) * factor) / 255;
	return COLOR(r, g, b);
}
