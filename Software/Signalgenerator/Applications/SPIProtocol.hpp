#pragma once

#include <stdint.h>
#include "FPGA.hpp"

namespace Protocol {

using FrontToRF = struct fronttorf {
	uint64_t frequency;
	int16_t dbm;
	struct {
		uint16_t UseIntRef :1;
		uint16_t ADCCouplingDC :1;
		uint16_t ADCImp1M :1;
		uint16_t ADCRange1 :1;
		uint16_t ADCRange2 :1;
	} Status;
	int16_t offset_I;
	int16_t offset_Q;
	uint16_t modulation_registers[16];
} __attribute__((packed, aligned(8)));

static constexpr uint32_t MagicConstant = 0xAE795C0D;

using RFToFront = struct rftofront {
	struct {
		uint16_t IntRefON :1;
		uint16_t MainPLLON :1;
		uint16_t MainPLLUnlocked :1;
		uint16_t HeterodynePLLON :1;
		uint16_t HeterodynePLLUnlock :1;
		uint16_t IQModEnabled :1;
		uint16_t Filter :3;
		uint16_t n15dbm1 :1;
		uint16_t n15dbm2 :1;
		uint16_t n15dbm3 :1;
		uint16_t AmplitudeUnlevel :1;
		uint16_t IQADCAvailable :1;
		uint16_t IADCOverload :1;
		uint16_t QADCOverload :1;
	} Status;
	uint16_t reserved;
	uint32_t MagicConstant;
	uint8_t reserved2[40];
} __attribute__((packed, aligned(8)));

enum class ModulationType : uint8_t {
	AM = 0,
	FM = 1,
	FM_USB = 2,
	FM_LSB = 3,
	QAM2 = 4,
	QAM4 = 5,
	QAM8 = 6,
	QAM16 = 7,
	QAM32 = 8,
	External = 9,
};

using Modulation = struct modulation {
	ModulationType type;
	union {
		struct {
			uint32_t depth; // Unit::Percent
		} AM;
		struct {
			uint32_t deviation; // Unit::Frequency
			uint16_t phase_offset; // 0 to 360 phase difference between I and Q
		} FM;
		struct {
			uint8_t SamplesPerSymbol;
			uint32_t SymbolsPerSecond;
			bool differential;
		} QAM;
		struct {
			bool ACCoupled;
			bool Impedance50R;
			uint32_t maxVoltage;
		} External;
	};
	FPGA::ModSrc source;
	union {
		struct {
			int32_t value;
		} Fixed;
		struct {
			uint32_t frequency;
		} Periodic;
		struct {
			int32_t minVoltage;
			int32_t maxVoltage;
			bool ACCoupled;
			bool Impedance50R;
		} ExtSrc;
	};
};

void SetupModulation(FrontToRF &data, Modulation mod);

}
