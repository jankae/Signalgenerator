#pragma once

#include <stdint.h>

namespace FPGA {

enum class GPIO : uint16_t {
	SW1_CTL1 = 0x0001,
	SW1_CTL2 = 0x0002,
	SW1_CTL3 = 0x0004,
	SW2_CTL1 = 0x0010,
	SW2_CTL2 = 0x0020,
	SW2_CTL3 = 0x0040,
	MOD_DISABLE = 0x0080,
	LED1 = 0x0100,
	LED2 = 0x0200,
	LED3 = 0x0400,
	LED4 = 0x0800,
	LED5 = 0x1000,
	ADC_DC = 0x1000,
	ADC_RANGE1 = 0x2000,
	ADC_IMP1M = 0x4000,
	ADC_RANGE2 = 0x8000,
};

enum class Reg : uint16_t {
	GPIO = 0x0000,
	EXT_ADC = 0x0001,
//	DAC_I = 0x0002,		// direct write to I/Q DAC not supported by FPGA anymore
//	DAC_Q = 0x0003,
	MOD_REG0 = 0x0004,
	MOD_REG1 = 0x0005,
	MOD_REG2 = 0x0006,
	MOD_REG3 = 0x0007,
	MOD_REG4 = 0x0008,
};

inline GPIO operator|(GPIO a, GPIO b) {
	return static_cast<GPIO>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}

void WriteReg(Reg reg, uint16_t val);

using Status = struct status {
	uint16_t IQADCAvailable :1;
	uint16_t IADCOverload :1;
	uint16_t QADCOverload :1;
};

Status ReadStatus();
void SetGPIO(GPIO g);
void ResetGPIO(GPIO g);
void UpdateGPIO();
void ConfigureExtADC(uint16_t maxval, bool enableI, bool enableQ,
		bool CouplingDC, bool Impedance1M, bool range1, bool range2);

//void SetDAC(uint16_t i, uint16_t q);

}
