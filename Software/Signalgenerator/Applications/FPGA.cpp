#include "FPGA.hpp"

#include "FIRTapOrder.hpp"
#include "HardwareLimits.hpp"
#include "main.h"
#include <math.h>

extern SPI_HandleTypeDef hspi1;

static uint16_t htons(uint16_t h) {
	return ((h & 0xFF) << 8) | ((h & 0xFF00) >> 8);
}

static int16_t constrain(int16_t v, int16_t min, int16_t max) {
	if (v > max) {
		v = max;
	} else if (v < min) {
		v = min;
	}
	return v;
}

void FPGA::SetConstellation(Constellation &c) {
	constexpr uint16_t max_DAC = 2048;

	uint16_t data[2];
	// start loading data to address 0
	data[0] = 0x0000;
	SPI1_CS_FPGA_GPIO_Port->BSRR = SPI1_CS_FPGA_Pin << 16;
	HAL_SPI_Transmit(&hspi1, (uint8_t*) data, 2, 100);
	int16_t i_val, q_val;
	uint16_t pnt_cnt = 0;
	while(c.GetScaledPoint(pnt_cnt, max_DAC, i_val, q_val)) {
		data[0] = htons(constrain(i_val, -max_DAC, max_DAC - 1));
		data[1] = htons(constrain(q_val, -max_DAC, max_DAC - 1));
		HAL_SPI_Transmit(&hspi1, (uint8_t*) data, 4, 100);
		pnt_cnt++;
	}
	SPI1_CS_FPGA_GPIO_Port->BSRR = SPI1_CS_FPGA_Pin;
}

void FPGA::SetFIRRaisedCosine(uint8_t sps, float beta) {
	int16_t FIRdata[(HardwareLimits::FIRTaps + 1) / 2];
	static constexpr int16_t MaxAmplitude = 1400; // TODO adjust to scale (keep in mind possible overflow in FPGA FIR addition!)
	static constexpr float PI = 3.1415926535f;
	for (uint8_t i = 0; i < (HardwareLimits::FIRTaps + 1) / 2; i++) {
		int8_t t = i - (HardwareLimits::FIRTaps + 1) / 2 + 1;
		float h_t;
		if (t == 0) {
			h_t = 1.0;
		} else if (abs(t) == sps / (2 * beta)) {
			h_t = std::sin(PI / (2 * beta)) / (PI / (2 * beta)) * PI / 4;
		} else {
			h_t = std::sin(PI * t / sps) / (PI * t / sps)
					* std::cos(beta * PI * t / sps)
					/ (1 - (2 * beta * t / sps) * (2 * beta * t / sps));
		}
		printf("FIR tap %d: %f\n", i, h_t);
		FIRdata[i] = htons(h_t * MaxAmplitude);
	}
	// start loading data to address 0
	uint16_t data = htons(0x8000);
	SPI1_CS_FPGA_GPIO_Port->BSRR = SPI1_CS_FPGA_Pin << 16;
	HAL_SPI_Transmit(&hspi1, (uint8_t*) &data, 2, 100);
	// Transmit coefficients in the correct order to the FPGA
	for (uint8_t i = 0; i < (HardwareLimits::FIRTaps + 1) / 2; i++) {
		HAL_SPI_Transmit(&hspi1, (uint8_t*) &FIRdata[FIRTapOrder[i]], 2, 100);
	}
	SPI1_CS_FPGA_GPIO_Port->BSRR = SPI1_CS_FPGA_Pin;
}
