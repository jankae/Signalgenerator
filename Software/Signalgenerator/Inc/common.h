#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "fatfs.h"

typedef struct {
	uint16_t width;
	uint16_t height;
	const uint16_t *data;
} Image_t;

typedef struct {
    int16_t x;
    int16_t y;
} coords_t;

typedef struct {
	/* name length up to 3 (plus string terminator) */
	char name[4];
	uint32_t factor;
} unitElement_t;

typedef const unitElement_t *unit_t[5];

extern const unit_t Unit_Current, Unit_Voltage, Unit_Power, Unit_Temperature,
		Unit_Resistance, Unit_Energy, Unit_Time, Unit_Memory, Unit_Capacity,
		Unit_Percent, Unit_Charge, Unit_None;
extern const int32_t null, maxPercent;

#define COORDS(v1, v2)	((coords_t){v1, v2})
#define SIZE(v1, v2)	COORDS(v1, v2)

int32_t common_Map(int32_t value, int32_t scaleFromLow, int32_t scaleFromHigh,
        int32_t scaleToLow, int32_t scaleToHigh);

uint32_t common_LeastDigitValueFromString(const char *s,
		const unit_t * const unit);

void common_StringFromValue(char *to, uint8_t len, int32_t val,
		const unit_t * const unit);

uint8_t common_ValueFromString(int32_t *value, char *s, const unit_t * const unit);

#endif
