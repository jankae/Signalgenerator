#include "mcp48x2.hpp"

void MCP48X2::Set(Channel c, uint16_t value, bool gain) {
	uint16_t cmd = value & 0x0FFF;
	if (c == Channel::B) {
		cmd |= 0x8000;
	}
	if (!gain) {
		cmd |= 0x2000;
	}
	cmd |= 0x1000;
	Write(cmd);
}

void MCP48X2::Shutdown(Channel c) {
	uint16_t cmd = 0;
	if (c == Channel::B) {
		cmd |= 0x8000;
	}
	Write(cmd);
}

void MCP48X2::Write(uint16_t cmd) {
	uint8_t data[2];
	data[0] = cmd >> 16;
	data[1] = cmd & 0xFF;
	CS->BSRR = CSpin << 16;
	HAL_SPI_Transmit(hspi, data, 2, 20);
	CS->BSRR = CSpin;
}
