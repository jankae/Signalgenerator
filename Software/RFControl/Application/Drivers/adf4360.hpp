#pragma once

#include "stm32f0xx_hal.h"

class ADF4360 {
public:
	constexpr ADF4360(SPI_HandleTypeDef *hspi, GPIO_TypeDef *CE, uint16_t CEpin,
			GPIO_TypeDef *LE, uint16_t LEpin, GPIO_TypeDef *MUX,
			uint16_t MUXpin) :
		LControl(0), LR(0), LN(0),
		hspi(hspi),
		CE(CE), CEpin(CEpin),
		LE(LE), LEpin(LEpin),
		MUX(MUX), MUXpin(MUXpin)
		{};

	bool Init();
	void ChipEnable(bool on);
	bool Locked();
	bool SetFrequency(uint32_t f);

	enum class CorePower : uint8_t {
		mA5 = 0x00,
		mA10 = 0x01,
		mA15 = 0x02,
		mA20 = 0x03,
	};
	void SetCorePower(CorePower p);
	enum class Power : uint8_t {
		n13dbm = 0x00,
		n11dbm = 0x01,
		n8dbm = 0x02,
		n6dbm = 0x03,
	};
	void SetPower(Power p);
	// sets the current in 0.31mA increments
	void SetCurrent(uint8_t inc);
	void Update();
private:
	enum class Latch {
		Control = 0x00,
		R = 0x01,
		N = 0x02,
	};
	void Write(Latch l, uint32_t val);
	uint32_t LControl, LR, LN;
	SPI_HandleTypeDef *hspi;
	GPIO_TypeDef *CE;
	uint16_t CEpin;
	GPIO_TypeDef *LE;
	uint16_t LEpin;
	GPIO_TypeDef *MUX;
	uint16_t MUXpin;
};
