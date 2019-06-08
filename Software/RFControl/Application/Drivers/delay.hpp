#pragma once

#include <stdint.h>
#include "stm32f0xx_hal.h"

namespace Delay {

void ms(uint32_t t);
void us(uint32_t t);

}
