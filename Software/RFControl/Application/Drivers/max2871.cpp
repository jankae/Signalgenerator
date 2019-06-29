#include "max2871.hpp"

#include "delay.hpp"
#include "../System/log.h"
#include <cmath>

#define Log_MAX2871 (LevelAll)

bool MAX2871::Init() {
	for (uint8_t i = 0; i < 6; i++) {
		regs[i] = 0;
	}

	ChipEnable(false);
	RFEnable(false);

	SetReference(10000000, false, 5, false);

	// non-inverting loop filter
	regs[2] |= (1UL << 6);
	// select digital lock detect
	regs[5] |= (0x1UL << 22);

	// enable output A (still controlled via RF_EN pin)
	regs[4] |= (1UL << 5);

	regs[4] |= (1UL << 23);

	// enable double buffering for register 4
	regs[2] |= (1UL << 13);

	// automatically switch to integer mode if F = 0
	regs[5] |= (1UL << 24);

	SetPower(Power::n1dbm);
	SetMode(Mode::LowNoise);
	SetCPMode(CPMode::CP20);
	SetCPCurrent(2);
	SetFrequency(1000000000);

	Update();
	Delay::ms(20);
	Update();

	return true;
}

void MAX2871::ChipEnable(bool on) {
	if (on) {
		CE->BSRR = CEpin;
	} else {
		CE->BSRR = CEpin << 16;
	}
}

void MAX2871::RFEnable(bool on) {
	if (on) {
		RF_EN->BSRR = RF_ENpin;
	} else {
		RF_EN->BSRR = RF_ENpin << 16;
	}
}

bool MAX2871::Locked() {
	return LD->IDR & LDpin;
}

void MAX2871::SetPower(Power p) {
	// only set power of port A (B not used)
	regs[4] &= ~0x18;
	regs[4] |= ((uint16_t) p << 3);
}

void MAX2871::SetMode(Mode m) {
	regs[2] &= ~0x60000000;
	regs[2] |= ((uint32_t) m << 29);
}

void MAX2871::SetCPMode(CPMode m) {
	regs[1] &= ~0x60000000;
	regs[1] |= ((uint32_t) m << 29);
}

void MAX2871::SetCPCurrent(uint8_t mA) {
	if(mA > 15) {
		LOG(Log_MAX2871, LevelWarn, "Clipping charge pump current to 15mA");
		mA = 15;
	}
	regs[2] &= ~0x1E00;
	regs[2] |= ((uint16_t) mA << 9);
}

bool MAX2871::SetFrequency(uint64_t f) {
	if (f < 23500000 || f > 6000000000ULL) {
		LOG(Log_MAX2871, LevelError,
				"Frequency must be between 23.5MHz and 6GHz");
		return false;
	}
	LOG(Log_MAX2871, LevelInfo, "Setting frequency to %lu%06luHz...",
			(uint32_t ) (f / 1000000), (uint32_t ) (f % 1000000));
	// select divider
	uint64_t f_vco = f;
	uint8_t div = 0;
	if (f < 46875000) {
		div = 0x07;
		f_vco *= 128;
	} else if (f < 93750000) {
		div = 0x06;
		f_vco *= 64;
	} else if (f < 187500000) {
		div = 0x05;
		f_vco *= 32;
	} else if (f < 375000000) {
		div = 0x04;
		f_vco *= 16;
	} else if (f < 750000000) {
		div = 0x03;
		f_vco *= 8;
	} else if (f < 1500000000) {
		div = 0x02;
		f_vco *= 4;
	} else if (f < 3000000000) {
		div = 0x01;
		f_vco *= 2;
	}
	LOG(Log_MAX2871, LevelDebug, "F_VCO: %lu%06luHz",
			(uint32_t ) (f_vco / 1000000), (uint32_t ) (f_vco % 1000000));
	uint16_t N = f_vco / f_PFD;
	if (N < 19 || N > 4091) {
		LOG(Log_MAX2871, LevelError,
				"Invalid N value, should be between 19 and 4091, got %lu", N);
		return false;
	}
	uint32_t rem_f = f_vco - N * f_PFD;
	LOG(Log_MAX2871, LevelDebug, "Remaining fractional frequency: %lu", rem_f);
	LOG(Log_MAX2871, LevelDebug, "Looking for best fractional match", rem_f);
	uint32_t best_deviation = UINT32_MAX;
	uint16_t best_M = 2, best_F = 0;
	for (uint16_t M = 2; M < 4096; M++) {
		for (uint16_t F = 0; F < M; F++) {
			uint32_t f = (f_PFD * F) / M;
			uint32_t deviation = abs(f - rem_f);
			if (deviation < best_deviation) {
				best_deviation = deviation;
				best_M = M;
				best_F = F;
				if (deviation == 0) {
					break;
				}
			}
		}
		if (best_deviation == 0) {
			break;
		}
	}
	LOG(Log_MAX2871, LevelDebug, "Best match is F=%u/M=%u, deviation of %luHz",
			best_F, best_M, best_deviation);
	uint64_t f_set = (uint64_t) N * f_PFD + (f_PFD * best_F) * f_PFD / best_M;
	f_set /= (1UL << div);

	// write values to registers
	regs[4] &= ~0x00700000;
	regs[4] |= ((uint32_t) div << 20);
	regs[0] &= ~0x7FFFFFF8;
	regs[0] |= ((uint32_t) N << 15) | ((uint32_t) best_F << 3);
	regs[1] &= ~0x00007FF8;
	regs[1] |= ((uint32_t) best_M << 3);

	LOG(Log_MAX2871, LevelInfo, "Set frequency to %lu%06luHz...",
			(uint32_t ) (f_set / 1000000), (uint32_t ) (f_set % 1000000));
	return true;
}

void MAX2871::Write(uint8_t reg, uint32_t val) {
	uint8_t data[4];
	data[0] = (val >> 24) & 0xFF;
	data[1] = (val >> 16) & 0xFF;
	data[2] = (val >> 8) & 0xFF;
	data[3] = ((val) & 0xF8) | (reg & 0x07);
	Delay::us(1);
	HAL_SPI_Transmit(hspi, data, 4, 20);
	LE->BSRR = LEpin;
	Delay::us(1);
	LE->BSRR = LEpin << 16;
	LOG(Log_MAX2871, LevelDebug, "Wrote 0x%08x to register %d", val, reg);
	Delay::ms(10);
}

bool MAX2871::SetReference(uint32_t f_ref, bool doubler, uint16_t r,
		bool div2) {
	if (f_ref < 10000000) {
		LOG(Log_MAX2871, LevelError,
				"Reference frequency must be >=10MHz, is %lu", f_ref);
		return false;
	} else if (f_ref > 105000000 && doubler) {
		LOG(Log_MAX2871, LevelError,
				"Reference frequency must be <=105MHz when used with doubler, is %lu",
				f_ref);
		return false;
	} else if (f_ref > 210000000) {
		LOG(Log_MAX2871, LevelError,
				"Reference frequency must be <=210MHz, is %lu", f_ref);
		return false;
	}
	if (r < 1 || r > 1023) {
		LOG(Log_MAX2871, LevelError,
				"Reference divider must be between 1 and 1023, is %d", r);
		return false;
	}
	// calculate PFD frequency
	uint32_t pfd = f_ref;
	if (doubler) {
		pfd *= 2;
	}
	pfd /= r;
	if (div2) {
		pfd /= 2;
	}
	if (pfd > 125000000) {
		LOG(Log_MAX2871, LevelError, "PFD frequency must be <=125MHz, is %d",
				pfd);
		return false;
	}
	// input values are valid, adjust registers
	regs[2] &= ~0x03FFC000;
	if (doubler) {
		regs[2] |= (1UL << 25);
	}
	if (div2) {
		regs[2] |= (1UL << 24);
	}
	regs[2] |= (r << 14);
	f_PFD = pfd;
	LOG(Log_MAX2871, LevelInfo, "Set PFD frequency to %lu", f_PFD);

	// updating VAS state machine clock
	uint16_t BS = f_PFD / 50000;
	if (BS > 1023) {
		BS = 1023;
	} else if (BS < 1) {
		BS = 1;
	}
	LOG(Log_MAX2871, LevelDebug, "BS set to %lu", BS);
	regs[4] &= ~0x030FF000;
	regs[4] |= ((BS & 0xFF) << 12);
	regs[4] |= (((BS >> 8) & 0x03) << 24);
	return true;
}

void MAX2871::Update() {
	for(int8_t i=5;i>=0;i--) {
		Write(i, regs[i]);
	}
}
