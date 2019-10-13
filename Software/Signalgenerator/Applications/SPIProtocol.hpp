#pragma once

#include <stdint.h>

namespace Protocol {

using FrontToRF = struct fronttorf {
	uint64_t frequency;
	int16_t dbm;
	struct {
		uint16_t UseIntRef :1;
		uint16_t reserved :15;
	} Status;
	int16_t offset_I;
	int16_t offset_Q;
	uint16_t modulation_registers[8];
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
		uint16_t reserved :3;
	} Status;
	uint16_t reserved1;
	uint32_t MagicConstant;
	uint8_t reserved2[24];
} __attribute__((packed, aligned(8)));;

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
};
enum class SourceType : uint8_t {
	Disabled = 0,
	FixedValue = 1,
	Sine = 2,
	RampUp = 3,
	RampDown = 4,
	Triangle = 5,
	Square = 6,
	PRBS = 7,
	Stream = 8,
};

using Modulation = struct modulation {
	ModulationType type;
	union {
		struct {
			uint32_t depth; // Unit::Percent
		} AM;
		struct {
			uint32_t deviation; // Unit::Frequency
		} FM;
		struct {
			uint8_t SamplesPerSymbol;
			uint32_t SymbolsPerSecond;
		} QAM;
	};
	SourceType source;
	union {
		struct {
			int32_t value;
		} Fixed;
		struct {
			uint32_t frequency;
		} Periodic;
	};
};

void SetupModulation(FrontToRF &data, Modulation mod);

}
