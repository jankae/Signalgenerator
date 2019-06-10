/*
 * color.h
 *
 *  Created on: Mar 14, 2017
 *      Author: jan
 */

#ifndef COLOR_H_
#define COLOR_H_

#include <stdint.h>

#define COLOR(r, g, b)		(uint16_t)(((r>>3)<<11)|((g>>2)<<5)|(b>>3))
#define COLOR_RED			COLOR(255,0,0)
#define COLOR_GREEN			COLOR(0,255,0)
#define COLOR_BLUE			COLOR(0,0,255)
#define COLOR_YELLOW		COLOR(255,255,0)
#define COLOR_ORANGE		COLOR(240, 120, 72)
#define COLOR_WHITE			COLOR(255,255,255)
#define COLOR_BLACK			COLOR(0,0,0)
#define COLOR_GRAY			COLOR(100, 100, 100)

#define COLOR_DARKGREEN		COLOR(0, 100, 0)
#define COLOR_LIGHTGRAY		COLOR(180, 180, 180)

#define COLOR_R(c)			((c&0xf800)>>8)
#define COLOR_G(c)			((c&0x07E0)>>3)
#define COLOR_B(c)			((c&0x001F)<<3)

#define COLOR_BG_DEFAULT	COLOR_WHITE//COLOR_LIGHTGRAY
#define COLOR_FG_DEFAULT	COLOR_BLACK

#define COLOR_UNSELECTABLE	COLOR_LIGHTGRAY
#define COLOR_SELECTED		COLOR_RED


typedef uint16_t color_t;

color_t color_Tint(color_t orig, color_t tint, uint8_t factor);


#endif /* COLOR_H_ */
