#include "fpga.hpp"
#include "main.h"
#include "../System/log.h"

#define Log_FPGA 	(LevelAll)

static uint16_t gpio = 0x0000;
extern SPI_HandleTypeDef hspi1;

void FPGA::WriteReg(Reg reg, uint16_t val) {
	FPGA_CS_GPIO_Port->BSRR = FPGA_CS_Pin << 16;
	uint16_t regval = 0x8000 | (uint16_t) reg;
	uint8_t send[4] = { (uint8_t) (regval >> 8), (uint8_t) (regval & 0xFF),
			(uint8_t) (val >> 8), (uint8_t) (val & 0xFF) };
	HAL_SPI_Transmit(&hspi1, send, 4, 100);
	FPGA_CS_GPIO_Port->BSRR = FPGA_CS_Pin;
	LOG(Log_FPGA, LevelDebug, "Wrote 0x%04x to register 0x%04x", val, (int) reg);
}

FPGA::Status FPGA::ReadStatus() {
	uint16_t val;
	FPGA_CS_GPIO_Port->BSRR = FPGA_CS_Pin << 16;
	uint8_t send[4] = { 0x00, 0x00, 0x00, 0x00 };
	uint8_t recv[4];
	HAL_SPI_TransmitReceive(&hspi1, send, recv, 4, 100);
	val = (uint16_t) recv[2] << 8 | recv[3];
	FPGA_CS_GPIO_Port->BSRR = FPGA_CS_Pin;
	LOG(Log_FPGA, LevelDebug, "Read 0x%04x from FPGA status", val);
	Status ret = {0};
	if (val & 0x0001) {
		ret.IQADCAvailable = 1;
	}
	if (val & 0x0002) {
		ret.IADCOverload = 1;
	}
	if (val & 0x0004) {
		ret.QADCOverload = 1;
	}
	return ret;
}

void FPGA::SetGPIO(GPIO g) {
	gpio |= (uint16_t) g;
}

void FPGA::ResetGPIO(GPIO g) {
	gpio &= ~(uint16_t) g;
}

void FPGA::UpdateGPIO() {
	WriteReg(Reg::GPIO, gpio);
}

void FPGA::ConfigureExtADC(uint16_t maxval, bool enableI, bool enableQ,
		bool CouplingDC, bool Impedance1M, bool range1, bool range2) {
	uint16_t value = maxval & 0x01FF;
	if (enableI) {
		value |= 0x4000;
	}
	if (enableQ) {
		value |= 0x8000;
	}
	WriteReg(Reg::EXT_ADC, value);
	if (CouplingDC) {
		SetGPIO(GPIO::ADC_DC);
	} else {
		ResetGPIO(GPIO::ADC_DC);
	}
	if (Impedance1M) {
		SetGPIO(GPIO::ADC_IMP1M);
	} else {
		ResetGPIO(GPIO::ADC_IMP1M);
	}
	if (range1) {
		SetGPIO(GPIO::ADC_RANGE1);
	} else {
		ResetGPIO(GPIO::ADC_RANGE1);
	}
	if (range2) {
		SetGPIO(GPIO::ADC_RANGE2);
	} else {
		ResetGPIO(GPIO::ADC_RANGE2);
	}
	UpdateGPIO();
}

//void FPGA::SetDAC(uint16_t i, uint16_t q) {
//	WriteReg(Reg::DAC_I, i);
//	WriteReg(Reg::DAC_Q, q);
//}
