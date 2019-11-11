#pragma once

#include "Constellation.hpp"

namespace FPGA {

void SetConstellation(Constellation &c);
void SetFIRRaisedCosine(uint8_t sps, float beta);
void SetFIRLowpass(uint32_t fc, uint32_t fs, float beta);

}
