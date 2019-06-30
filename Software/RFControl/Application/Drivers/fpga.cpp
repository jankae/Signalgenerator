#include "fpga.hpp"
#include "main.h"

static uint16_t gpio = 0x0000;
extern SPI_HandleTypeDef hspi1;

enum class Reg : uint16_t {
	GPIO = 0x0000,
	DAC_I = 0x0002,
	DAC_Q = 0x0003,
};

static void write_reg(Reg reg, uint16_t val) {
	FPGA_CS_GPIO_Port->BSRR = FPGA_CS_Pin << 16;
	uint16_t regval = 0x8000 | (uint16_t) reg;
	uint8_t send[4] = { (uint8_t) (regval >> 8), (uint8_t) (regval & 0xFF),
			(uint8_t) (val >> 8), (uint8_t) (val & 0xFF) };
	HAL_SPI_Transmit(&hspi1, send, 4, 100);
	FPGA_CS_GPIO_Port->BSRR = FPGA_CS_Pin;
}

void FPGA::SetGPIO(GPIO g) {
	gpio |= (uint16_t) g;
}

void FPGA::ResetGPIO(GPIO g) {
	gpio &= ~(uint16_t) g;
}

void FPGA::UpdateGPIO() {
	write_reg(Reg::GPIO, gpio);
}

void FPGA::SetDAC(uint16_t i, uint16_t q) {
	write_reg(Reg::DAC_I, i);
	write_reg(Reg::DAC_Q, q);
}
