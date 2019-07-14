#pragma once

#include <stdint.h>

namespace Stream {

void Init();
uint8_t *GetData(uint16_t maxlen, uint16_t *len);
uint8_t LoadToFPGA(uint32_t maxTime);

}
