#include "common.h"

#include <ctype.h>

static const unitElement_t uA = { "uA", 1 };
static const unitElement_t mA = { "mA", 1000 };
static const unitElement_t A = { "A", 1000000 };

static const unitElement_t uV = { "uV", 1 };
static const unitElement_t mV = { "mV", 1000 };
static const unitElement_t V = { "V", 1000000 };

static const unitElement_t uW = { "uW", 1 };
static const unitElement_t mW = { "mW", 1000 };
static const unitElement_t W = { "W", 1000000 };

static const unitElement_t C = {"\xF8""C", 1};

static const unitElement_t uR = {"uR", 1};
static const unitElement_t mR = {"mR", 1000};
static const unitElement_t R = {"R", 1000000};

static const unitElement_t uWh = {"uWh", 1};
static const unitElement_t mWh = {"mWh", 1000};
static const unitElement_t Wh = {"Wh", 1000000};

static const unitElement_t ms = {"ms", 1};
static const unitElement_t s = {"s", 1000};
static const unitElement_t min = {"m", 60000};
static const unitElement_t hour = {"h", 3600000};

static const unitElement_t B = {"B", 1};
static const unitElement_t kB = {"kB", 1024};

static const unitElement_t uF = {"uF", 1};
static const unitElement_t mF = {"mF", 1000};
static const unitElement_t F = {"F", 1000000};

static const unitElement_t percent = {"%", 1000000};

static const unitElement_t uAh = {"uAh", 1};
static const unitElement_t mAh = {"mAh", 1000};
static const unitElement_t Ah = {"Ah", 1000000};

static const unitElement_t none = {"", 1};

const unit_t Unit_Current = { &uA, &mA, &A, NULL };
const unit_t Unit_Voltage = { &uV, &mV, &V, NULL };
const unit_t Unit_Power = { &uW, &mW, &W, NULL };
const unit_t Unit_Temperature = {&C, NULL };
const unit_t Unit_Resistance = { &uR, &mR, &R, NULL };
const unit_t Unit_Energy = { &uWh, &mWh, &Wh, NULL };
const unit_t Unit_Time = {&ms, &s, &min, &hour, NULL };
const unit_t Unit_Memory = { &B, &kB, NULL };
const unit_t Unit_Capacity = { &uF, &mF, &F, NULL };
const unit_t Unit_Percent = { &percent, NULL };
const unit_t Unit_Charge = { &uAh, &mAh, &Ah, NULL };
const unit_t Unit_None = {&none, NULL };

const int32_t null = 0;
const int32_t maxPercent = 100000000;

int32_t common_Map(int32_t value, int32_t scaleFromLow, int32_t scaleFromHigh,
		int32_t scaleToLow, int32_t scaleToHigh) {
	int32_t result;
	value -= scaleFromLow;
	int32_t rangeFrom = scaleFromHigh - scaleFromLow;
	int32_t rangeTo = scaleToHigh - scaleToLow;
	result = ((int64_t) value * rangeTo) / rangeFrom;
	result += scaleToLow;
	return result;
}

uint32_t common_LeastDigitValueFromString(const char *s,
		const unit_t * const unit) {
	uint32_t dotdivisor = 0;
	while (*s) {
		if (*s == '.') {
			dotdivisor = 1;
		} else if (*s == '-' || *s == ' ' || isdigit((uint8_t )*s)) {
			dotdivisor *= 10;
		} else {
			/* end of value string */
			break;
		}
		s++;
	}
	if(!dotdivisor)
		dotdivisor = 1;
	if (*s) {
		/* try to find matching unit */
		uint8_t i = 0;
		while ((*unit)[i]) {
			if (!strncmp((*unit)[i]->name, s, strlen((*unit)[i]->name))) {
				/* this unit matches the value string */
				return (*unit)[i]->factor / dotdivisor;
			}
			i++;
		}
		/* no matching unit found */
		return 0;
	} else {
		/* string ended before detecting unit */
		return 1;
	}
}

void common_StringFromValue(char *to, uint8_t len, int32_t val,
		const unit_t * const unit) {
	/* store sign */
	int8_t negative = 0;
	if (val < 0) {
		val = -val;
		negative = 1;
	}
	/* find applicable unit */
	const unitElement_t *selectedUnit = (*unit)[0];
	uint8_t i = 1;
	while ((*unit)[i]) {
		if (val / (*unit)[i]->factor) {
			/* this unit is a better fit */
			selectedUnit = (*unit)[i];
		}
		i++;
	}
	if (!selectedUnit) {
		/* this should not be possible */
		*to = 0;
		return;
	}
	/* calculate number of available digits */
	uint8_t digits = len - strlen(selectedUnit->name) - negative;
	/* calculate digits before the dot */
	uint32_t intval = val / selectedUnit->factor;
	uint8_t beforeDot = 0;
	while (intval) {
		intval /= 10;
		beforeDot++;
	}
	if (beforeDot > digits) {
		/* value does not fit available space */
		*to = 0;
		return;
	}
	if (!beforeDot)
		beforeDot = 1;
	int8_t spaceAfter = digits - 1 - beforeDot;
	uint32_t factor = 1;
	int8_t afterDot = 0;
	while (spaceAfter > 0 && factor < selectedUnit->factor) {
		factor *= 10;
		spaceAfter--;
		afterDot++;
	}
	val = ((uint64_t) val * factor) / selectedUnit->factor;

	/* compose string from the end */
	/* copy unit */
	uint8_t pos = digits + negative;
	strcpy(&to[pos], selectedUnit->name);
	/* actually displayed digits */
	digits = beforeDot + afterDot;
	while (digits) {
		afterDot--;
		digits--;
		to[--pos] = val % 10 + '0';
		val /= 10;
		if (afterDot == 0) {
			/* place dot at this position */
			to[--pos] = '.';
		}
	}
	if (negative) {
		to[--pos] = '-';
	}
	while (pos > 0) {
		to[--pos] = ' ';
	}
}

// TODO merge with common_LeastDigitValueFromString?
uint8_t common_ValueFromString(int32_t *value, char *s,
		const unit_t * const unit) {
	*value = 0;
	int8_t sign = 1;
	uint32_t dotDivider = 0;
	/* skip any leading spaces */
	while (*s && *s == ' ') {
		s++;
	}
	if(!*s) {
		/* string contains only spaces */
		return 0;
	}
	/* evaluate value part */
	while (*s && (*s == '-' || isdigit((uint8_t )*s) || *s == '.')) {
		if (*s == '-') {
			if (sign == -1) {
				/* this is the second negative */
				return 0;
			}
			sign = -1;
		} else if (*s == '.') {
			if (dotDivider) {
				/* already seen a dot */
				return 0;
			} else {
				dotDivider = 1;
			}
		} else {
			/* must be a digit */
			*value *= 10;
			*value += *s - '0';
			dotDivider *= 10;
		}
		s++;
	}
	if(!dotDivider)
		dotDivider = 1;
	/* finished the value part, now look for matching unit */
	/* skip any trailing spaces */
	while (*s && *s == ' ') {
		s++;
	}
	if(!*s) {
		/* string contains no unit */
		if (unit == &Unit_None) {
			*value *= sign;
			return 1;
		}
		return 0;
	}
	/* try to find matching unit */
	uint8_t i = 0;
	while ((*unit)[i]) {
		if (!strncmp((*unit)[i]->name, s, strlen((*unit)[i]->name))) {
			/* this unit matches the value string */
			*value = (int64_t) (*value) * (*unit)[i]->factor / dotDivider;
			*value *= sign;
			return 1;
		}
		i++;
	}
	/* no matching unit found */
	return 0;
}
