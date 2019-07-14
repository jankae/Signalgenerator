#include "Stream.hpp"

#include <math.h>
#include "stm32f1xx.h"
#include "main.h"

extern SPI_HandleTypeDef hspi1;

static uint8_t sine_lookup[1024];
static uint16_t index;

void Stream::Init() {
	for(uint16_t i=0;i<sizeof(sine_lookup);i++) {
		sine_lookup[i] = 128
				+ 127 * sin(i * 2 * 3.1415926526f / sizeof(sine_lookup));
	}
	index = 0;
}

uint8_t* Stream::GetData(uint16_t maxlen, uint16_t* len) {
	uint16_t maxAvailable = sizeof(sine_lookup) - index;
	if(maxlen < maxAvailable) {
		maxAvailable = maxlen;
	}
	*len = maxAvailable;
	uint8_t *ret = &sine_lookup[index];
	index += maxAvailable;
	index %= sizeof(sine_lookup);
	return ret;
}

uint8_t Stream::LoadToFPGA(uint32_t maxTime) {
	uint32_t start = HAL_GetTick();
	uint8_t free;
	uint16_t len;
	uint8_t *data = GetData(1, &len);
	SPI1_CS_FPGA_GPIO_Port->BSRR = SPI1_CS_FPGA_Pin << 16;
	HAL_SPI_TransmitReceive_DMA(&hspi1, data, &free, len);
	while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
	SPI1_CS_FPGA_GPIO_Port->BSRR = SPI1_CS_FPGA_Pin;
	uint8_t initial_free = free;
	uint16_t cnt = 0;
	while(free >= 4 && HAL_GetTick() - start < maxTime) {
		data = GetData(256, &len);
		SPI1_CS_FPGA_GPIO_Port->BSRR = SPI1_CS_FPGA_Pin << 16;
		HAL_SPI_TransmitReceive_DMA(&hspi1, data, &free, len);
		while(HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
		SPI1_CS_FPGA_GPIO_Port->BSRR = SPI1_CS_FPGA_Pin;
		cnt += len;
	}
	if (cnt > 0) {
		printf("Loaded %d bytes to FPGA, filled from %d to %d\n", cnt, initial_free, free);
	}
	return initial_free;
}
