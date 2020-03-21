#pragma once

#include "Constellation.hpp"

namespace FPGA {

enum class Reg : uint16_t {
	CTRL = 0x0000,
	SourcePINC = 0x0001,
	Setting1 = 0x0002,
	Setting2 = 0x0003,
	Setting3 = 0x0004,
	ExtIOffset = 0x0008,
	ExtIScale = 0x0009,
	ExtQOffset = 0x000A,
	ExtQScale = 0x000B,
};

enum class ModSrc : uint16_t {
	Disabled = 0x0000,
	Fixed = 0x0100,
	Sine = 0x0200,
	RampUp = 0x0300,
	RampDown = 0x0400,
	Triangle = 0x0500,
	Square = 0x0600,
	PRBS = 0x0700,
	FIFO = 0x0800,
	ExtI = 0x0900,
	ExtQ = 0x0A00,
};

enum class ModType : uint16_t {
	Disabled = 0x0000,
	FM = 0x0004,
	AM = 0x0008,
	QAM = 0x000C,
	QAMDifferential = 0x000D,
	External = 0x000E,
};

uint16_t CalculateExtADCScaling(int32_t maxADCVoltage);

void SetConstellation(Constellation &c);
void SetFIRRaisedCosine(uint8_t sps, float beta);
void SetFIRLowpass(uint32_t fc, uint32_t fs, float beta);

}
