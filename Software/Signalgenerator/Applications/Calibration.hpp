#pragma once

#include <stdint.h>

namespace Calibration {

void Init();
void DefaultAmplitude();
void RunAmplitude();
int16_t CorrectAmplitude(uint32_t freq, int16_t dbm);

using IQOffset = struct iqoffset {
	int16_t I;
	int16_t Q;
};

void DefaultBalance();
void RunBalance();
IQOffset CorrectBalance(uint32_t freq);

}
