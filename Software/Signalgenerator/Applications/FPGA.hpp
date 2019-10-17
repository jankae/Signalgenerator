#pragma once

#include "Constellation.hpp"

namespace FPGA {

void SetConstellation(Constellation &c);
void SetFIRRaisedCosine(uint8_t sps, float beta);

}
