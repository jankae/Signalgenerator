#pragma once

#include <stdint.h>

namespace Calibration {

void Init();
void Run();
int16_t Correct(uint32_t freq, int16_t dbm);

}
