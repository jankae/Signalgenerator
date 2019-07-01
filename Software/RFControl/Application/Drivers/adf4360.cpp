#include "adf4360.hpp"

#include "delay.hpp"
#include "../System/log.h"

#define Log_ADF4369		(LevelAll)

bool ADF4360::Init() {
	LR = LN = LControl = 0;

	// set a 1MHz PFD frequency (assuming 10MHz reference)
	LR = (10 << 2);
	// lock detect precision 5 cycles
	LR |= (1UL << 18);

	SetCorePower(CorePower::mA15);
	SetPower(Power::n6dbm);
	SetCurrent(4);

	// set MUX pin to digital lock detect
	LControl |= (1 << 5);

	// select 16/17 as P prescaler
	LControl |= (1UL << 22);

	LControl |= (1UL << 8);

	// divide output by 2
	LN |= (1UL << 22);

	SetFrequency(1000000000);

	Update();
	return true;
}

void ADF4360::ChipEnable(bool on) {
	if (on) {
		CE->BSRR = CEpin;
	} else {
		CE->BSRR = CEpin << 16;
	}
}

bool ADF4360::Locked() {
	return MUX->IDR & MUXpin;
}

bool ADF4360::SetFrequency(uint32_t f) {
	LOG(Log_ADF4369, LevelInfo, "Setting output frequency to %luHz", f);
	// PFD frequency is 1MHz, output divide-by-2 is enabled
	uint32_t f_vco = f * 2;
	if (f_vco < 1850000000 || f_vco > 2170000000) {
		LOG(Log_ADF4369, LevelError,
				"F_VCO out of range, should be between 1.85 and 2.17GHz, is %luHz",
				f_vco);
		return false;
	}
	uint32_t B = (f_vco / 1000000ULL) / 16;
	uint32_t rem_f = f_vco - (B * 16 * 1000000ULL);
	uint32_t A = rem_f / 1000000;
	LOG(Log_ADF4369, LevelDebug, "Selected counters: A=%u, B=%u", A, B);
	if (B < 3 || B > 8191 || A > 31 || A > B) {
		LOG(Log_ADF4369, LevelError, "Invalid A/B values, aborting");
		return false;
	}
	// change N counter latch
	LN &= ~0x1FFFFC;
	LN |= (B << 8) | (A << 2);
	return true;
}

void ADF4360::Update() {
	Write(Latch::R, LR);
	Write(Latch::Control, LControl);
	Delay::ms(5);
	Write(Latch::N, LN);
}

void ADF4360::SetCorePower(CorePower p) {
	LControl &= ~0x0C;
	LControl |= ((uint8_t) p << 2);
}

void ADF4360::SetPower(Power p) {
	LControl &= ~0x03000;
	LControl |= ((uint8_t) p << 12);
}

void ADF4360::SetCurrent(uint8_t inc) {
	if (inc < 1) {
		inc = 1;
	} else if (inc > 8) {
		inc = 8;
	}
	inc--;
	LControl &= ~0x1C000;
	LControl |= ((uint8_t) inc << 14);
}

void ADF4360::Write(Latch l, uint32_t val) {
	uint8_t data[3];
	data[0] = (val >> 16) & 0xFF;
	data[1] = (val >> 8) & 0xFF;
	data[2] = ((val) & 0xFC) | ((uint8_t) l & 0x03);
	Delay::us(1);
	HAL_SPI_Transmit(hspi, data, 3, 20);
	LE->BSRR = LEpin;
	Delay::us(1);
	LE->BSRR = LEpin << 16;
}
