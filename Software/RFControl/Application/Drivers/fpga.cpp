#include "fpga.hpp"

static uint16_t gpio = 0x0000;

void FPGA::SetGPIO(GPIO g) {
	gpio |= (uint16_t) g;
}

void FPGA::ResetGPIO(GPIO g) {
	gpio &= ~(uint16_t) g;
}

void FPGA::UpdateGPIO() {
	// TODO
}
