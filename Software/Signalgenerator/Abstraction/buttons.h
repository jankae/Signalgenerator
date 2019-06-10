#ifndef BUTTONS_H_
#define BUTTONS_H_

#include "gui.h"
#include "gpio.h"

/* Button definitions */
#define BUTTON_0			0x80000000
#define BUTTON_1			0x40000000
#define BUTTON_2			0x20000000
#define BUTTON_3			0x10000000
#define BUTTON_4			0x08000000
#define BUTTON_5			0x04000000
#define BUTTON_6			0x02000000
#define BUTTON_7			0x01000000
#define BUTTON_8			0x00800000
#define BUTTON_9			0x00400000

#define BUTTON_ENCODER		0x00200000
#define BUTTON_DOT			0x00100000
#define BUTTON_DEL			0x00080000
#define BUTTON_ESC			0x00040000
#define BUTTON_UP			0x00020000
#define BUTTON_LEFT			0x00008000
#define BUTTON_DOWN			0x00004000
#define BUTTON_RIGHT		0x00002000
#define BUTTON_SIGN			0x00001000

#define BUTTON_ONOFF		0x00000800
#define BUTTON_UNIT1		0x00010000
#define BUTTON_UNITm		0x00000200

#define BUTTON_IS_INPUT(b) (b & (BUTTON_0|BUTTON_1|BUTTON_2|BUTTON_3| \
							BUTTON_4|BUTTON_5|BUTTON_6|BUTTON_7|BUTTON_8| \
							BUTTON_9|BUTTON_DOT|BUTTON_SIGN) ? 1 : 0)

#define BUTTON_IS_DIGIT(b) (b & (BUTTON_0|BUTTON_1|BUTTON_2|BUTTON_3| \
							BUTTON_4|BUTTON_5|BUTTON_6|BUTTON_7|BUTTON_8| \
							BUTTON_9) ? 1 : 0)

#define BUTTON_IS_ARROW(b) (b & (BUTTON_LEFT|BUTTON_RIGHT|BUTTON_UP|BUTTON_DOWN))

#define BUTTON_TODIGIT(b)  (b>= BUTTON_9 ? __builtin_clz(b) : 0)

#ifdef __cplusplus
extern "C"
{
#endif

void buttons_Init();

void buttons_Update(void);

#ifdef __cplusplus
}
#endif

#endif
