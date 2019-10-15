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

void FPGA::SetGPIO(GPIO g) {
	gpio |= (uint16_t) g;
}

void FPGA::ResetGPIO(GPIO g) {
	gpio &= ~(uint16_t) g;
}

void FPGA::UpdateGPIO() {
	WriteReg(Reg::GPIO, gpio);
}

//void FPGA::SetDAC(uint16_t i, uint16_t q) {
//	WriteReg(Reg::DAC_I, i);
//	WriteReg(Reg::DAC_Q, q);
//}
