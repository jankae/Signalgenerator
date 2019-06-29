#include "app.h"

#include "RF.hpp"
#include "SPIProtocol.hpp"
#include <string.h>
#include "stm32f0xx_hal.h"
#include "System/log.h"

static Protocol::RFToFront send;
static Protocol::FrontToRF recv;
static Protocol::FrontToRF spi_new;

static volatile bool new_data = false;

extern SPI_HandleTypeDef hspi2;

void app(void) {
	log_init();
	send.MagicConstant = Protocol::MagicConstant;
	Protocol::FrontToRF spi_current;
	memset(&spi_current, 0, sizeof(spi_current));
	RF::Init(&send);
	HAL_SPI_TransmitReceive_DMA(&hspi2, (uint8_t*) &send, (uint8_t*) &recv,
			sizeof(send));

	while (1) {
		while (!new_data)
			;
		new_data = false;
		HAL_SPI_DMAPause(&hspi2);
		if (spi_new.frequency != spi_current.frequency
				|| spi_new.dbm != spi_current.dbm) {
			RF::Configure(spi_new.frequency, spi_new.dbm);
		}
		if (spi_new.Status.UseIntRef != spi_current.Status.UseIntRef) {
			RF::InternalReference(spi_new.Status.UseIntRef);
		}
		spi_current = spi_new;
		HAL_SPI_DMAResume(&hspi2);
	}
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
	if (hspi == &hspi2) {
		// got new data from frontpanel
		spi_new = recv;
		new_data = true;
	}
}
