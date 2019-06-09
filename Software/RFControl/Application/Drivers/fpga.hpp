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
};

inline GPIO operator|(GPIO a, GPIO b) {
	return static_cast<GPIO>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}

void SetGPIO(GPIO g);
void ResetGPIO(GPIO g);
void UpdateGPIO();

}
