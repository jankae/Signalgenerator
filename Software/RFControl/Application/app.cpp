#include "app.h"

#include "RF.hpp"
#include "SPIProtocol.hpp"
#include <string.h>
#include "stm32f0xx_hal.h"
#include "System/log.h"

static Protocol::RFToFront send;

static volatile bool new_data = false;

extern SPI_HandleTypeDef hspi2;

void app(void) {
	log_init();
	send.MagicConstant = Protocol::MagicConstant;
	Protocol::FrontToRF spi_current;
	memset(&spi_current, 0, sizeof(spi_current));
	HAL_Delay(3000);
	RF::Init(&send);

	while (1) {
		Protocol::FrontToRF spi_new;
		if (HAL_SPI_TransmitReceive(&hspi2, (uint8_t*) &send, (uint8_t*) &spi_new,
				sizeof(spi_new), 1000) == HAL_OK) {
			LOG(Log_System, LevelInfo,
					"SPI data, freq: %lu, dbm: %u, intref: 0x%02x",
					(uint32_t) spi_new.frequency, spi_new.dbm, (uint8_t) spi_new.Status.UseIntRef);
//			while (!new_data)
//				;
//			new_data = false;
			if (spi_new.frequency != spi_current.frequency
					|| spi_new.dbm != spi_current.dbm) {
				RF::Configure(spi_new.frequency, spi_new.dbm);
			}
			if (spi_new.Status.UseIntRef != spi_current.Status.UseIntRef) {
				RF::InternalReference(spi_new.Status.UseIntRef);
			}
			spi_current = spi_new;
		} else {
			LOG(Log_System, LevelInfo, "SPI timeout");
		}
		HAL_Delay(10);
		HAL_SPI_Abort(&hspi2);
	}
}

//void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
//	if (hspi == &hspi2) {
//		// got new data from frontpanel
//		spi_new = recv;
//		new_data = true;
//	}
//}
