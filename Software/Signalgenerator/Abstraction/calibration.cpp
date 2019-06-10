#include "../Abstraction/calibration.h"

#include <math.h>

typedef struct {
	int32_t offset;
	float scale;
} calEntryData_t;

const calEntryData_t defaultEntries[CAL_NUMBER_OF_ENTRIES] = {
		/* voltage to DAC */
		{0, (float) DAC_MAX / MAX_VOLTAGE},
		/* source current */
		{0, (float) DAC_MAX / MAX_SOURCE_CURRENT},
		/* sink current */
		{0, (float) DAC_MAX / MAX_SINK_CURRENT},
		/* ADC to current low */
		{ADC_MAX_DIFF, (float) -MAX_CURRENT_LOW_ADC / ADC_MAX_DIFF},
		/* ADC to current high */
		{ADC_MAX_DIFF, (float) -MAX_CURRENT_HIGH_ADC / ADC_MAX_DIFF},
		/* ADC to push/pull out */
		{0, (float) MAX_PUSHPULL_OUT / ADC_MAX_SINGLE},
		/* ADC to battery voltage */
		{0, (float) MAX_BATTERY / ADC_MAX_SINGLE},
};

calEntryData_t entries[CAL_NUMBER_OF_ENTRIES];

const fileEntry_t CalFileEntries[CAL_NUMBER_OF_ENTRIES * 2] = {
		{"DACvoltageOffset", &entries[CAL_VOLTAGE_DAC].offset, PTR_INT32},
		{"DACvoltageScale", &entries[CAL_VOLTAGE_DAC].scale, PTR_FLOAT},

		{"DACSourceOffset", &entries[CAL_MAX_CURRENT_DAC].offset, PTR_INT32},
		{"DACSourceScale", &entries[CAL_MAX_CURRENT_DAC].scale, PTR_FLOAT},

		{"DACSinkOffset", &entries[CAL_MIN_CURRENT_DAC].offset, PTR_INT32},
		{"DACSinkScale", &entries[CAL_MIN_CURRENT_DAC].scale, PTR_FLOAT},

		{"ADCcurrentlOffset", &entries[CAL_ADC_CURRENT_LOW].offset, PTR_INT32},
		{"ADCcurrentlScale", &entries[CAL_ADC_CURRENT_LOW].scale, PTR_FLOAT},

		{"ADCcurrenthOffset", &entries[CAL_ADC_CURRENT_HIGH].offset, PTR_INT32},
		{"ADCcurrenthScale", &entries[CAL_ADC_CURRENT_HIGH].scale, PTR_FLOAT},

		{"ADCPushpullOffset", &entries[CAL_ADC_PUSHPULL_OUT].offset, PTR_INT32},
		{"ADCPushpullScale", &entries[CAL_ADC_PUSHPULL_OUT].scale, PTR_FLOAT},

		{"ADCoutputOffset", &entries[CAL_ADC_BATTERY].offset, PTR_INT32},
		{"ADCoutputScale", &entries[CAL_ADC_BATTERY].scale, PTR_FLOAT},
};

void cal_Init(void){
	/* start with default entries */
	memcpy(entries, defaultEntries, sizeof(entries));
}

uint8_t cal_Save(void) {
	if (file_open("OUTPUT.CAL", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
		return 0;
	}
	file_WriteParameters(CalFileEntries, CAL_NUMBER_OF_ENTRIES * 2);
	file_close();
	return 1;
}

uint8_t cal_Load(void) {
	if (file_open("OUTPUT.CAL", FA_OPEN_EXISTING | FA_READ) != FR_OK) {
		return 0;
	}
	if (file_ReadParameters(CalFileEntries, CAL_NUMBER_OF_ENTRIES * 2)
			== FILE_OK) {
		file_close();
		return 1;
	} else {
		file_close();
		return 0;
	}
}

#define CAL_MAX_DEV			15
#define CAL_PERCENTDEV(exp, meas)	abs(((exp) - (meas))*100/(exp)) <= CAL_MAX_DEV ? 0 : 1
#define CAL_MAX_LSB_DEV_ADC	(ADC_MAX_SINGLE/40)
#define CAL_MAX_LSB_DEV_DAC	(DAC_MAX/40)
#define CAL_ABSLIMITS(min, max, meas) ((meas)>=(min) && (meas)<=(max)) ? 0 : 1

uint8_t cal_Valid(void) {
	uint8_t i;
	for (i = 0; i < CAL_NUMBER_OF_ENTRIES; i++) {
		/* Check calibration against default entry */
		if (CAL_PERCENTDEV(defaultEntries[i].scale, entries[i].scale)) {
			/* scale factor deviates too much */
			printf(
					"Calibration entry scale %d deviates too much: is %f, should be around %f\n",
					i, entries[i].scale, defaultEntries[i].scale);
			return 0;
		}
		int32_t maxDev;
		/* calculate limits for offset deviation */
		if (fabs(defaultEntries[i].scale) > 1) {
			/* probably an ADC conversion */
			maxDev = CAL_MAX_LSB_DEV_ADC;
		} else {
			/* probably a DAC conversion, take scale into account */
			maxDev = (float) CAL_MAX_LSB_DEV_DAC
					/ fabs(defaultEntries[i].scale);
		}

		if (CAL_ABSLIMITS(defaultEntries[i].offset - maxDev,
				defaultEntries[i].offset + maxDev, entries[i].offset)) {
			/* offset deviates too much */
			printf(
					"Calibration entry offset %d deviates too much: is %ld, should be around %ld\n",
					i, entries[i].offset, defaultEntries[i].offset);
			return 0;
		}
	}
	return 1;
}

int32_t cal_GetCalibratedValue(calEntryNum_t entry, int32_t rawValue) {
	rawValue -= entries[entry].offset;
	return entries[entry].scale * rawValue;
}

void cal_UpdateEntry(calEntryNum_t entry, int32_t raw1, int32_t cal1,
		int32_t raw2, int32_t cal2){
	/* calculate scale */
	entries[entry].scale = (float) (cal2 - cal1) / (raw2 - raw1);
	/* calculate offset */
	entries[entry].offset = raw1 - cal1 / entries[entry].scale;
}
