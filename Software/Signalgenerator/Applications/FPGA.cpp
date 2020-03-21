#include "FPGA.hpp"

#include "FIRTapOrder.hpp"
#include "HardwareLimits.hpp"
#include "main.h"
#include <math.h>


static constexpr float PI = 3.1415926535f;

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

static void loadFIR(int16_t *taps) {
	// start loading data to address 0
	uint16_t data = htons(0x8000);
	SPI1_CS_FPGA_GPIO_Port->BSRR = SPI1_CS_FPGA_Pin << 16;
	HAL_SPI_Transmit(&hspi1, (uint8_t*) &data, 2, 100);
	// Transmit coefficients in the correct order to the FPGA
	for (uint8_t i = 0; i < (HardwareLimits::FIRTaps + 1) / 2; i++) {
		int16_t transmit = htons(taps[FIRTapOrder[i]]);
		HAL_SPI_Transmit(&hspi1, (uint8_t*) &transmit, 2, 100);
	}
	SPI1_CS_FPGA_GPIO_Port->BSRR = SPI1_CS_FPGA_Pin;
}

void FPGA::SetFIRRaisedCosine(uint8_t sps, float beta) {
	int16_t FIRdata[(HardwareLimits::FIRTaps + 1) / 2];
	static constexpr int16_t MaxAmplitude = 1400;
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
		FIRdata[i] = h_t * MaxAmplitude;
	}
	loadFIR(FIRdata);
}

static double Bessel(double x) {
	double Sum = 0.0, XtoIpower;
	int i, j, Factorial;
	for (i = 1; i < 10; i++) {
		XtoIpower = pow(x / 2.0, (double) i);
		Factorial = 1;
		for (j = 1; j <= i; j++)
			Factorial *= j;
		Sum += pow(XtoIpower / (double) Factorial, 2.0);
	}
	return (1.0 + Sum);
}


void FPGA::SetFIRLowpass(uint32_t fc, uint32_t fs, float beta) {
	int16_t FIRdata[(HardwareLimits::FIRTaps + 1) / 2];
	const float omegaC = (float) fc / fs;
	static float coefficients[(HardwareLimits::FIRTaps + 1) / 2];
	static constexpr int16_t MaxAmplitude = 2047;
	float sum = 0.0f;
	for (uint8_t i = 0; i < (HardwareLimits::FIRTaps + 1) / 2; i++) {
		int8_t arg = i - (HardwareLimits::FIRTaps + 1) / 2 + 1;
		if (arg == 0) {
			coefficients[i] = omegaC;
			sum += coefficients[i];
		} else {
			coefficients[i] = std::sin(omegaC * 2 * arg * PI) / (2 * arg * PI);
			sum += 2*coefficients[i];
		}

		// apply kaiser window
		float k_arg = sqrt(1.0f - pow(2.0f * arg / HardwareLimits::FIRTaps, 2.0f));
		float kaiser = Bessel(k_arg * beta) / Bessel(beta);

		printf("FIR tap %d: %f %f %f\n", i, coefficients[i], kaiser,
				coefficients[i] * kaiser);
		coefficients[i] *= kaiser;
	}

	printf("Normalizing by scaling with %f\n", 1.0f/sum);
	// normalize
	for (uint8_t i = 0; i < (HardwareLimits::FIRTaps + 1) / 2; i++) {
		FIRdata[i] = coefficients[i] / sum * MaxAmplitude;
	}
	loadFIR(FIRdata);
}

uint16_t FPGA::CalculateExtADCScaling(int32_t maxADCVoltage) {
	/* ADC should report its maximum value (512) after scaling when maxADCVoltage is applied.
	 * Scaling is done in two stages in the FPGA:
	 * 1. Multiply (offset-removed) ADC value with a signed 10 bit integer
	 * 2. Right-shift the result up to 15 bits
	 */
	// Calculate ideal multiplicator (extend result by all 9 bits available via 10 bit signed multiplication)
	int32_t multiplicator = (int32_t) (HardwareLimits::MaxExtModADCValue
			* HardwareLimits::MaxExtModADCVoltage) / maxADCVoltage;
	uint8_t rightShift = 9;
	// decrease multiplicator until valid range is reached
	while (multiplicator < -HardwareLimits::MaxExtModADCValue
			|| multiplicator >= HardwareLimits::MaxExtModADCValue) {
		multiplicator >>= 1;
		rightShift--;
	}
	// assemble FPGA register value
	return ((uint32_t) rightShift) << 12 | (multiplicator & 0x03FF);
}
