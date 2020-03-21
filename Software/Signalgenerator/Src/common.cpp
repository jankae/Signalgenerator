#include <common.hpp>
#include <ctype.h>

//static const unitElement_t uA = { "uA", 1 };
//static const unitElement_t mA = { "mA", 1000 };
//static const unitElement_t A = { "A", 1000000 };
//
//static const unitElement_t uV = { "uV", 1 };
//static const unitElement_t mV = { "mV", 1000 };
//static const unitElement_t V = { "V", 1000000 };
//
//static const unitElement_t uW = { "uW", 1 };
//static const unitElement_t mW = { "mW", 1000 };
//static const unitElement_t W = { "W", 1000000 };
//
//static const unitElement_t C = {"\xF8""C", 1};
//
//static const unitElement_t uR = {"uR", 1};
//static const unitElement_t mR = {"mR", 1000};
//static const unitElement_t R = {"R", 1000000};
//
//static const unitElement_t uWh = {"uWh", 1};
//static const unitElement_t mWh = {"mWh", 1000};
//static const unitElement_t Wh = {"Wh", 1000000};
//
//static const unitElement_t ms = {"ms", 1};
//static const unitElement_t s = {"s", 1000};
//static const unitElement_t min = {"m", 60000};
//static const unitElement_t hour = {"h", 3600000};
//
//static const unitElement_t B = {"B", 1};
//static const unitElement_t kB = {"kB", 1024};
//
//static const unitElement_t uF = {"uF", 1};
//static const unitElement_t mF = {"mF", 1000};
//static const unitElement_t F = {"F", 1000000};
//
//static const unitElement_t percent = {"%", 1000000};
//
//static const unitElement_t uAh = {"uAh", 1};
//static const unitElement_t mAh = {"mAh", 1000};
//static const unitElement_t Ah = {"Ah", 1000000};
//
//static const unitElement_t none = {"", 1};
//
//const unit_t Unit_Current = { &uA, &mA, &A, NULL };
//const unit_t Unit_Voltage = { &uV, &mV, &V, NULL };
//const unit_t Unit_Power = { &uW, &mW, &W, NULL };
//const unit_t Unit_Temperature = {&C, NULL };
//const unit_t Unit_Resistance = { &uR, &mR, &R, NULL };
//const unit_t Unit_Energy = { &uWh, &mWh, &Wh, NULL };
//const unit_t Unit_Time = {&ms, &s, &min, &hour, NULL };
//const unit_t Unit_Memory = { &B, &kB, NULL };
//const unit_t Unit_Capacity = { &uF, &mF, &F, NULL };
//const unit_t Unit_Percent = { &percent, NULL };
//const unit_t Unit_Charge = { &uAh, &mAh, &Ah, NULL };
//const unit_t Unit_None = {&none, NULL };

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

const coords_t operator+(coords_t const& lhs, coords_t const& rhs) {
	coords_t ret = lhs;
	ret.x += rhs.x;
	ret.y += rhs.y;
	return ret;
}

#define CRC32_POLYGON 0xEDB88320

uint32_t common_crc32(uint32_t crc, const void *data, uint32_t len) {
	uint8_t *u8buf = (uint8_t*) data;
	int k;

	crc = ~crc;
	while (len--) {
		crc ^= *u8buf++;
		for (k = 0; k < 8; k++)
			crc = crc & 1 ? (crc >> 1) ^ CRC32_POLYGON : crc >> 1;
	}
	return ~crc;
}

