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

const coords_t operator+(coords_t const& lhs, coords_t const& rhs);

uint32_t common_crc32(uint32_t crc, const void *data, uint32_t len);

//typedef struct {
//	/* name length up to 3 (plus string terminator) */
//	char name[4];
//	uint32_t factor;
//} unitElement_t;
//
//typedef const unitElement_t *unit_t[5];
//
//extern const unit_t Unit_Current, Unit_Voltage, Unit_Power, Unit_Temperature,
//		Unit_Resistance, Unit_Energy, Unit_Time, Unit_Memory, Unit_Capacity,
//		Unit_Percent, Unit_Charge, Unit_None;
//extern const int32_t null, maxPercent;
//
#define COORDS(v1, v2)	((coords_t){v1, v2})
#define SIZE(v1, v2)	COORDS(v1, v2)

int32_t common_Map(int32_t value, int32_t scaleFromLow, int32_t scaleFromHigh,
        int32_t scaleToLow, int32_t scaleToHigh);

static inline int16_t constrain_int16_t(int16_t val, int16_t min, int16_t max) {
	if(val < min) {
		return min;
	} else if(val > max) {
		return max;
	} else {
		return val;
	}
}

//
//uint32_t common_LeastDigitValueFromString(const char *s,
//		const unit_t * const unit);
//
//void common_StringFromValue(char *to, uint8_t len, int32_t val,
//		const unit_t * const unit);
//
//uint8_t common_ValueFromString(int32_t *value, char *s, const unit_t * const unit);

#endif
