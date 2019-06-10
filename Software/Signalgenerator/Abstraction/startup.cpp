#include <startup.h>
#include "pushpull.h"

#include "adc.h"
#include "fatfs.h"
#include "file.h"
#include "input.h"

#include "Supply.h"
#include "Charger.h"
#include "Settings.h"
#include "Simulator.h"
#include "Characterisation.h"
#include "Scope.h"
#include "Info.h"
#include "display.h"
#include "buttons.h"

extern uint8_t pushpull_SPI_OK;
extern uint16_t RawADC[SPI_BLOCK_SIZE];

extern QueueHandle_t GUIeventQueue;

static uint8_t line;

typedef enum {TEST_PASSED = 0, TEST_FAILED = 1, TEST_WARNING = 2} TestResult_t;

/* Maximum allowed deviation of voltages in percent that still passes the test */
#define VOLTAGE_MAX_DEV		5
/* Tests whether exp and meas are the same value with some tolerance */
#define TEST_PERCENTDEV(exp, meas)	abs(((int32_t)(exp) - meas)*100/exp) <= VOLTAGE_MAX_DEV ? TEST_PASSED : TEST_FAILED
/* Tests a value for absolute limits */
#define TEST_ABSLIMITS(min, max, meas) (meas>=min && meas<=max) ? TEST_PASSED : TEST_FAILED

static uint16_t get_5VRail(void) {
	uint16_t result[2];
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) result, 2);
	/* wait for ADC to finish */
	HAL_Delay(10);
	/* convert to mV, full scale ADC is 6.6V */
	return (uint32_t) result[0] * 6600 / 4096;
}

static uint16_t get_3V3Rail(void) {
	uint16_t result[2];
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) result, 2);
	/* wait for ADC to finish */
	HAL_Delay(10);
	/* convert to mV, full scale ADC is 1.2V -> 1200mV * 4096 = 4915200 */
	return 4915200UL / result[1];
}

static uint16_t get_3V3Rail_SPI(void) {
	/* convert to mV, full scale ADC is around 1.2V -> 1200mV * 4096 = 4915200 */
	return 4915200UL / RawADC[9];
}

static uint16_t get_5VRail_SPI(void) {
	/* convert to mV, full scale ADC is 6.6V */
	return (uint32_t) RawADC[8] * 6600 / 4096;
}

static uint16_t get_24VRail_SPI(void) {
	/* convert to mV, full scale ADC is 36.3V */
	return (uint32_t) RawADC[7] * 36300 / 4096;
}

static int16_t get_neg8VRail_SPI(void) {
	/* convert to mV, full scale ADC is -15.51V */
	return (int32_t) RawADC[6] * -15510 / 4096;
}

static void display_TestResult(const char * const name, const char * const result,
		TestResult_t res) {
	display_SetForeground(COLOR_WHITE);
	display_String(0, line * 16, name);
	if (res == TEST_PASSED) {
		display_SetForeground(COLOR_GREEN);
		printf("%s  %s PASSED\r\n", name, result);
	} else if (res == TEST_FAILED) {
		display_SetForeground(COLOR_RED);
		printf("%s %s FAILED\r\n", name, result);
	} else if (res == TEST_WARNING) {
		display_SetForeground(COLOR_YELLOW);
		printf("%s %s WARNING\r\n", name, result);
	}
	display_String(200, line * 16, result);
	line++;
}

static void updateResult(TestResult_t *overall, TestResult_t res) {
	if(res == TEST_FAILED) {
		*overall = TEST_FAILED;
	} else if(res == TEST_WARNING) {
		*overall = TEST_WARNING;
	}
}

void startup_Hardware(void) {
	HAL_ADCEx_Calibration_Start(&hadc1);
	display_SetBackground(COLOR_BLACK);
	display_SetForeground(COLOR_WHITE);
	display_SetFont(Font_Big);
	display_Clear();
	display_String(0, 0, "Running selftest...");
	printf("Running selftest...\n");

	line = 1;

	char buffer[32];
	TestResult_t res;
	TestResult_t overallRes = TEST_PASSED;
	int32_t meas;

	/* Check frontpanel board voltages */
	meas = get_3V3Rail();
	res = TEST_PERCENTDEV(3300, meas);
	updateResult(&overallRes, res);
	common_StringFromValue(buffer, 6, meas * 1000, &Unit_Voltage);
	display_TestResult("3.3V(1) rail:", buffer, res);

	meas = get_5VRail();
	res = TEST_PERCENTDEV(5000, meas);
	updateResult(&overallRes, res);
	common_StringFromValue(buffer, 6, meas * 1000, &Unit_Voltage);
	display_TestResult("5V(1) rail:", buffer, res);

	if(overallRes != TEST_PASSED) {
		goto error;
	}

	/* Check connection to analog board */
//	if(pushpull_SPI_OK) {
//		display_TestResult("SPI Connection:", "OK", TEST_PASSED);
//	} else {
//		display_TestResult("SPI Connection:", "ERROR", TEST_FAILED);
//		goto error;
//	}

	/* Check rails on analog board */
	meas = get_3V3Rail_SPI();
	res = TEST_PERCENTDEV(3300, meas);
	updateResult(&overallRes, res);
	common_StringFromValue(buffer, 6, meas * 1000, &Unit_Voltage);
	display_TestResult("3.3V(2) rail:", buffer, res);

	meas = get_5VRail_SPI();
	res = TEST_PERCENTDEV(5000, meas);
	updateResult(&overallRes, res);
	common_StringFromValue(buffer, 6, meas * 1000, &Unit_Voltage);
	display_TestResult("5V(2) rail:", buffer, res);

	meas = get_24VRail_SPI();
	res = TEST_PERCENTDEV(24000, meas);
	updateResult(&overallRes, res);
	common_StringFromValue(buffer, 6, meas * 1000, &Unit_Voltage);
	display_TestResult("24V rail:", buffer, res);

	meas = get_neg8VRail_SPI();
	res = TEST_PERCENTDEV(-8000, meas);
	updateResult(&overallRes, res);
	common_StringFromValue(buffer, 6, meas * 1000, &Unit_Voltage);
	display_TestResult("-8V rail:", buffer, res);

	meas = pushpull_GetTemperature();
	res = TEST_ABSLIMITS(-10, 60, meas);
	updateResult(&overallRes, res);
	common_StringFromValue(buffer, 5, meas, &Unit_Temperature);
	display_TestResult("Temperature:", buffer, res);

	if(overallRes != TEST_PASSED) {
		goto error;
	}

	/* Sanity check some output stage behavior */
	/* Bias current should be close to zero at this point */
	meas = pushpull_GetBiasCurrent();
	res = TEST_ABSLIMITS(0, 15000, meas);
	updateResult(&overallRes, res);
	common_StringFromValue(buffer, 6, meas, &Unit_Current);
	display_TestResult("Bias current:", buffer, res);

	/* Output current should be close to zero at this point */
	meas = pushpull_GetCurrent();
	res = TEST_ABSLIMITS(-10000, 10000, meas);
	updateResult(&overallRes, res);
	common_StringFromValue(buffer, 6, meas, &Unit_Current);
	display_TestResult("Output current:", buffer, res);

	pushpull_AcquireControl();
	pushpull_SetDriveCurrent(200);
	pushpull_SetSinkCurrent(100000);
	pushpull_SetSourceCurrent(100000);
	pushpull_SetVoltage(1000000);
	/* Wait for changes to take effect */
	HAL_Delay(100);

	/* Output voltage should be close to 1V at this point */
	meas = pushpull_GetOutputVoltage();
	/* Allow up to 250mV (Output does not reach GND) */
	res = TEST_PERCENTDEV(1000000, meas);
	updateResult(&overallRes, res);
	common_StringFromValue(buffer, 6, meas, &Unit_Voltage);
	display_TestResult("Output low:", buffer, res);

	/* Set high output voltage */
	pushpull_SetVoltage(18000000);
	/* Wait for changes to take effect */
	HAL_Delay(100);

	/* Output voltage should be around 18V */
	meas = pushpull_GetOutputVoltage();
	res = TEST_PERCENTDEV(18000000, meas);
	updateResult(&overallRes, res);
	common_StringFromValue(buffer, 6, meas, &Unit_Voltage);
	display_TestResult("Output high:", buffer, res);

	pushpull_SetVoltage(0);
	HAL_Delay(10);
	pushpull_SetDriveCurrent(0);
	pushpull_SetSinkCurrent(0);
	pushpull_SetSourceCurrent(0);
	pushpull_ReleaseControl();

	if(overallRes != TEST_PASSED) {
error:
		display_SetForeground(COLOR_RED);
		display_String(0, 224, "HARDWARE ERROR. STOPPING.");
		// TODO Add key to continue anyway
		HAL_Delay(2000);
	} else {
		display_SetForeground(COLOR_GREEN);
		display_String(0, 224, "HARDWARE TESTS PASSED");
		HAL_Delay(500);
	}
}

void startup_Software(void) {
	display_SetBackground(COLOR_BLACK);
	display_SetForeground(COLOR_WHITE);
	display_SetFont(Font_Big);
	display_Clear();
	display_String(0, 0, "Booting...");

	line = 1;

	uint8_t error = 0;
	uint8_t warning = 0;

	/* Start sampling of frontpanel controls */
	buttons_Init();

	switch(file_Init()) {
	case -1:
		display_TestResult("SD card:", "INT ERR", TEST_FAILED);
		error = 1;
		break;
	case 0:
		display_TestResult("SD card:", "MISSING", TEST_WARNING);
		warning = 1;
		break;
	case 1:
		display_TestResult("SD card:", "OK", TEST_PASSED);
		break;
	}

	/* Initialize sampling of touch display */
	if(input_Init()==pdPASS) {
		display_TestResult("Touch thread:", "STARTED", TEST_PASSED);
	} else {
		display_TestResult("Touch thread:", "ERROR", TEST_FAILED);
		error = 1;
	}

	/* Load touch calibration */
	if(touch_LoadCalibration()) {
		display_TestResult("Touch calib:", "LOADED", TEST_PASSED);
	} else {
		display_TestResult("Touch calib:", "MISSING", TEST_WARNING);
		warning = 1;
	}

	/* Load output calibration */
	if(cal_Load()) {
		display_TestResult("Output calib:", "LOADED", TEST_PASSED);
	} else {
		display_TestResult("Output calib:", "MISSING", TEST_WARNING);
		warning = 1;
	}

	if(cal_Valid()) {
		display_TestResult("Calibration:", "OK", TEST_PASSED);
	} else {
		display_TestResult("Calibration:", "DUBIOUS", TEST_WARNING);
		warning = 1;
	}

//	/* Setup Apps */
	Supply_Init();
	Charger_Init();
	Simulator_Init();
//	Characterisation_Init();
	Scope_Init();
	settings_Init();
	Info_Init();

	/* Start GUI thread */
	if(gui_Init()) {
		display_TestResult("GUI thread:", "STARTED", TEST_PASSED);
	} else {
		display_TestResult("GUI thread:", "ERROR", TEST_FAILED);
		error = 1;
	}

	if (error) {
		display_SetForeground(COLOR_RED);
		display_String(0, 224, "SOFTWARE ERROR. STOPPING.");
		/* keep showing screen until there is some user input */
		GUIEvent_t event;
		while (xQueuePeek(GUIeventQueue, &event, 0) == pdFALSE) {
		}
	} else {
		display_SetForeground(COLOR_WHITE);
		display_String(0, 208, "Boot process completed.");
		char buffer[27];
		snprintf(buffer, sizeof(buffer), "Remaining HEAP: %lu",
				(uint32_t) xPortGetFreeHeapSize());
		display_String(0, 224, buffer);

		HAL_Delay(1000);
	}

	if(warning) {
		/* keep showing screen until there is some user input */
		GUIEvent_t event;
		while (xQueuePeek(GUIeventQueue, &event, 0) == pdFALSE) {
		}
	}
}

