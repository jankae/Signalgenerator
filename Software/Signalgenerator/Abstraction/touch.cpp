#include "touch.h"

#include "semphr.h"
#include "fatfs.h"
#include "file.h"

#define CS_LOW()			(GPIOB->BSRR = GPIO_PIN_7<<16u)
#define CS_HIGH()			(GPIOB->BSRR = GPIO_PIN_7)
//#define DOUT_LOW()			(GPIOB->BSRR = GPIO_PIN_5<<16u)
//#define DOUT_HIGH()			(GPIOB->BSRR = GPIO_PIN_5)
//#define SCK_LOW()			(GPIOB->BSRR = GPIO_PIN_3<<16u)
//#define SCK_HIGH()			(GPIOB->BSRR = GPIO_PIN_3)
//#define DIN()				(GPIOB->IDR & GPIO_PIN_4)
#define PENIRQ()			(!(GPIOB->IDR & GPIO_PIN_8))

/* A2-A0 bits in control word */
#define CHANNEL_X			(0x10)
#define CHANNEL_Y			(0x50)
#define CHANNEL_3			(0x20)
#define CHANNEL_4			(0x60)

/* SER/DFR bit */
#define SINGLE_ENDED		(0x04)
#define DIFFERENTIAL		(0x00)

/* Resolution */
#define BITS8				(0x80)
#define BITS12				(0x00)

/* Power down mode */
#define PD_PENIRQ			(0x00)
#define PD_NOIRQ			(0x01)
#define PD_ALWAYS_ON		(0x03)

static uint8_t calibrating = 0;

int32_t offsetX = 0, offsetY = 0;
float scaleX = (float) TOUCH_RESOLUTION_X / 4096;
float scaleY = (float) TOUCH_RESOLUTION_Y / 4096;

const fileEntry_t touchCal[4] = {
		{"xfactor", &scaleX, PTR_FLOAT},
		{"xoffset", &offsetX, PTR_INT32},
		{"yfactor", &scaleY, PTR_FLOAT},
		{"yoffset", &offsetY, PTR_INT32},
};

extern SPI_HandleTypeDef hspi3;
extern SemaphoreHandle_t xMutexSPI3;

void touch_Init(void) {
	CS_HIGH();
}

static uint16_t ADS7843_Read(uint8_t control) {
	/* SPI3 is also used for the SD card. Reduce speed now to properly work with
	 * ADS7843 */
	uint16_t cr1 = hspi3.Instance->CR1;
	/* Minimum CLK frequency is 2.5MHz, this could be achieved with a prescaler of 16
	 * (2.25MHz). For reliability, a prescaler of 32 is used */
	hspi3.Instance->CR1 = (hspi3.Instance->CR1 & ~SPI_BAUDRATEPRESCALER_256 )
			| SPI_BAUDRATEPRESCALER_32;
	CS_LOW();
	/* highest bit in control must always be one */
	control |= 0x80;
	uint8_t read[2];
	/* transmit control word */
	HAL_SPI_Transmit(&hspi3, &control, 1, 10);
	/* read ADC result */
	uint16_t dummy = 0;
	HAL_SPI_TransmitReceive(&hspi3, (uint8_t*) &dummy, (uint8_t*) read, 2, 10);
	/* shift and mask 12-bit result */
	uint16_t res = ((uint16_t) read[0] << 8) + read[1];
	res >>= 3;
	res &= 0x0FFF;
	CS_HIGH();
	/* Reset SPI speed to previous value */
	hspi3.Instance->CR1 = cr1;
	return 4095 - res;
}

static void touch_SampleADC(uint16_t *rawX, uint16_t *rawY, uint16_t samples) {
	uint16_t i;
	uint32_t X = 0;
	uint32_t Y = 0;
	for (i = 0; i < samples; i++) {
		X += ADS7843_Read(
		CHANNEL_X | DIFFERENTIAL | BITS12 | PD_PENIRQ);
	}
	for (i = 0; i < samples; i++) {

		Y += ADS7843_Read(
		CHANNEL_Y | DIFFERENTIAL | BITS12 | PD_PENIRQ);
	}
	*rawX = X / samples;
	*rawY = Y / samples;
}

int8_t touch_GetCoordinates(coords_t *c) {
	if(calibrating) {
		/* don't report coordinates while calibrating */
		return 0;
	}
	if (PENIRQ()) {
		uint16_t rawY, rawX;
		/* screen is being touched */
		/* Acquire SPI resource */
		if (xSemaphoreTake(xMutexSPI3, 10)) {
			touch_SampleADC(&rawX, &rawY, 20);
			/* Release SPI resource */
			xSemaphoreGive(xMutexSPI3);
		} else {
			/* SPI is busy */
			return -1;
		}
		if (!PENIRQ()) {
			/* touch has been released during measurement */
			return 0;
		}
		/* convert to screen resolution */
		c->x = rawX * scaleX + offsetX;
		c->y = rawY * scaleY + offsetY;
		if(c->x < 0)
			c->x = 0;
		else if(c->x >= TOUCH_RESOLUTION_X)
			c->x = TOUCH_RESOLUTION_X - 1;
		if(c->y < 0)
			c->y = 0;
		else if(c->y >= TOUCH_RESOLUTION_Y)
			c->y = TOUCH_RESOLUTION_Y - 1;
		return 1;
	} else {
		return 0;
	}
}

static uint8_t touch_SaveCalibration(void) {
	if (file_open("TOUCH.CAL", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
		return 0;
	}
	file_WriteParameters(touchCal, 4);
	file_close();
	return 1;
}

uint8_t touch_LoadCalibration(void) {
	if (file_open("TOUCH.CAL", FA_OPEN_EXISTING | FA_READ) != FR_OK) {
		return 0;
	}
	if (file_ReadParameters(touchCal, 4) == FILE_OK) {
		file_close();
		return 1;
	} else {
		file_close();
		return 0;
	}
}

void touch_Calibrate(void) {
	if(!xSemaphoreTake(xMutexSPI3, 500)) {
		/* SPI is busy */
		return;
	}
	calibrating = 1;
	uint8_t done = 0;
	/* display first calibration cross */
	display_SetBackground(COLOR_WHITE);
	display_SetForeground(COLOR_BLACK);
	display_Clear();
	display_Line(0, 0, 40, 40);
	display_Line(40, 0, 0, 40);
	coords_t p1;
	do {
		/* wait for touch to be pressed */
		while (!PENIRQ())
			;
		/* get raw data */
		touch_SampleADC((uint16_t*) &p1.x, (uint16_t*) &p1.y, 1000);
		if (p1.x <= 1000 && p1.y <= 1000)
			done = 1;
	} while (!done);
	while (PENIRQ())
		;
	/* display second calibration cross */
	display_Clear();
	display_Line(DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1, DISPLAY_WIDTH - 41,
	DISPLAY_HEIGHT - 41);
	display_Line(DISPLAY_WIDTH - 41, DISPLAY_HEIGHT - 1, DISPLAY_WIDTH - 1,
	DISPLAY_HEIGHT - 41);
	coords_t p2;
	done = 0;
	do {
		/* wait for touch to be pressed */
		while (!PENIRQ())
			;
		/* get raw data */
		touch_SampleADC((uint16_t*) &p2.x, (uint16_t*) &p2.y, 1000);
		if (p2.x >= 3000 && p2.y >= 3000)
			done = 1;
	} while (!done);

	/* calculate new calibration values */
	/* calculate scale */
	scaleX = (float) (DISPLAY_WIDTH - 40) / (p2.x - p1.x);
	scaleY = (float) (DISPLAY_HEIGHT - 40) / (p2.y - p1.y);
	/* calculate offset */
	offsetX = 20 - p1.x * scaleX;
	offsetY = 20 - p1.y * scaleY;

	while(PENIRQ());

	/* Release SPI resource */
	xSemaphoreGive(xMutexSPI3);

	/* Try to write calibration data to file */
	if(touch_SaveCalibration()) {
		printf("Wrote touch calibration file\n");
	} else {
		printf("Failed to create touch calibration file\n");
	}

	calibrating = 0;
}


