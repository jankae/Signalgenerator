#pragma once
#include <cstdint>

namespace HardwareLimits {

static constexpr uint32_t MinFrequency = 5000000;
static constexpr uint32_t MaxFrequency = 2100000000;
static constexpr int16_t MaxOutputLevel = 2000;
static constexpr int16_t MinOutputLevel = -6000;

static constexpr uint32_t FPGA_CLK = 100000000;
static constexpr uint32_t MaxFIRRate = 12500000;
static constexpr uint8_t FIRTaps = 32;
static constexpr uint8_t BitsModSrcPinc = 27;
static constexpr uint8_t BitsModSrcExposed = 16;
static constexpr uint8_t BitsQAMSampleratePinc = 32;
static constexpr uint8_t BitsQAMSamplerateExposed = 32;
static constexpr uint8_t BitsFMDDSPinc = 32;
static constexpr uint16_t MaxSrcValue = 4095;
static constexpr uint16_t MaxFMDeviationSetting = 65535;

static constexpr uint32_t MaxModSrcFreq = ((uint64_t) FPGA_CLK
		* ((1ULL << BitsModSrcExposed) - 1)) / (1ULL << BitsModSrcPinc);
static constexpr uint32_t MaxFMDeviation = ((uint64_t) FPGA_CLK * MaxSrcValue
		* MaxFMDeviationSetting) / (1ULL << BitsFMDDSPinc);

}
