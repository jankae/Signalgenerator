#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "fatfs.h"

namespace Unit {

using unit = struct unit {
	/* name length up to 4 (plus string terminator) */
	char name[5];
	uint32_t factor;
};

//typedef const unitElement_t *Unit::unit[];

extern const unit *Current[], *Voltage[], *Power[], *Temperature[],
		*Resistance[], *Energy[], *Time[], *Memory[], *Capacity[], *Percent[],
		*Charge[], *Weight[], *Force[], *None[], *Fixed3[], *Hex[],
		*Frequency[], *dbm[], *SampleRate[], *Degree[];
extern const int32_t null, maxPercent;

#define COORDS(v1, v2)	((coords_t){(int16_t) (v1), (int16_t) (v2)})
#define SIZE(v1, v2)	COORDS(v1, v2)

uint32_t LeastDigitValueFromString(const char *s,
		const unit *unit[]);

void StringFromValue(char *to, uint8_t len, int64_t val,
		const unit *unit[]);

uint8_t ValueFromString(int32_t *value, char *s, const unit *unit[]);
int64_t ValueFromString(const char *s, uint32_t multiplier);

}

