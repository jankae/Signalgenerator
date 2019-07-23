#pragma once

#include "stm32f0xx_hal.h"

class MCP48X2 {
public:
	constexpr MCP48X2(SPI_HandleTypeDef *hspi, GPIO_TypeDef *CS,
			uint16_t CSpin) :
		hspi(hspi),
		CS(CS), CSpin(CSpin)
		{};
	enum class Channel : uint8_t {
		A,
		B,
	};

	void Set(Channel c, uint16_t value, bool gain);
	void Shutdown(Channel c);
	static constexpr uint16_t MaxValue = 4095;
private:
	void Write(uint16_t cmd);
	SPI_HandleTypeDef *hspi;
	GPIO_TypeDef *CS;
	uint16_t CSpin;
};
