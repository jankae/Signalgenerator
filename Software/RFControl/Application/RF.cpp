#include "RF.hpp"

#include "mcp48x2.hpp"
#include "max2871.hpp"
#include "adf4360.hpp"
#include "delay.hpp"
#include "fpga.hpp"
#include <cmath>
#include "log.h"

#define Log_RF		(LevelAll)

#include "main.h"
extern SPI_HandleTypeDef hspi1;

static MCP48X2 PowerDAC = MCP48X2(&hspi1, DBM_CS_GPIO_Port, DBM_CS_Pin);
static ADF4360 HeterodyneLO = ADF4360(&hspi1, ADF_CE_GPIO_Port, ADF_CE_Pin,
		ADF_LE_GPIO_Port, ADF_LE_Pin, ADF_MUX_GPIO_Port, ADF_MUX_Pin);
static MAX2871 Synthesizer = MAX2871(&hspi1, SYNTH_CE_GPIO_Port, SYNTH_CE_Pin,
		SYNTH_LE_GPIO_Port, SYNTH_LE_Pin, SYNTH_MUX_GPIO_Port, SYNTH_MUX_Pin,
		SYNTH_RF_EN_GPIO_Port, SYNTH_RF_EN_Pin, SYNTH_LD_GPIO_Port,
SYNTH_LD_Pin);

extern ADC_HandleTypeDef hadc;
constexpr uint16_t samplelength = 256;
uint16_t ADC_Samples[samplelength];

RF::Status status;

void RF::Init() {
	status.enabled = false;
	status.unlevel = false;
	status.lo_unlocked = false;
	status.synth_unlocked = false;
	status.used_att = Attenuation::DB45;
	HAL_ADC_Start_DMA(&hadc, (uint32_t*) ADC_Samples, samplelength);
	PowerDAC.Shutdown(MCP48X2::Channel::A);
	PowerDAC.Shutdown(MCP48X2::Channel::B);
	HeterodyneLO.Init();
	Synthesizer.Init();
	Configure(0, 0);
	LOG(Log_RF, LevelInfo, "Initialized");
}

void RF::SetAttenuator(Attenuation a) {
	switch (a) {
	case RF::Attenuation::DB0:
		DB15_1_GPIO_Port->BSRR = DB15_1_Pin << 16;
		DB15_2_GPIO_Port->BSRR = DB15_2_Pin << 16;
		DB15_3_GPIO_Port->BSRR = DB15_3_Pin << 16;
		LOG(Log_RF, LevelDebug, "Output attenuation 0db");
		break;
	case RF::Attenuation::DB15:
		DB15_1_GPIO_Port->BSRR = DB15_1_Pin << 16;
		DB15_2_GPIO_Port->BSRR = DB15_2_Pin << 16;
		DB15_3_GPIO_Port->BSRR = DB15_3_Pin;
		LOG(Log_RF, LevelDebug, "Output attenuation 15db");
		break;
	case RF::Attenuation::DB30:
		DB15_1_GPIO_Port->BSRR = DB15_1_Pin << 16;
		DB15_2_GPIO_Port->BSRR = DB15_2_Pin;
		DB15_3_GPIO_Port->BSRR = DB15_3_Pin;
		LOG(Log_RF, LevelDebug, "Output attenuation 30db");
		break;
	case RF::Attenuation::DB45:
		DB15_1_GPIO_Port->BSRR = DB15_1_Pin;
		DB15_2_GPIO_Port->BSRR = DB15_2_Pin;
		DB15_3_GPIO_Port->BSRR = DB15_3_Pin;
		LOG(Log_RF, LevelDebug, "Output attenuation 45db");
		break;
	}
	status.used_att = a;
}

void RF::SetHeterodynePath(bool use) {
	if(use) {
		HeterodyneLO.ChipEnable(true);
		HeterodyneLO.Init();
		HeterodyneLO.SetFrequency(1000000000);
		HeterodyneLO.Update();
		EN_DIRECT_GPIO_Port->BSRR = EN_DIRECT_Pin << 16;
		LOG(Log_RF, LevelDebug, "Heterodyne path enabled");
	} else {
		HeterodyneLO.ChipEnable(false);
		EN_DIRECT_GPIO_Port->BSRR = EN_DIRECT_Pin;
		LOG(Log_RF, LevelDebug, "Heterodyne path disabled");
	}
}

void RF::Configure(uint64_t f, int16_t cdbm) {
	if (f == 0) {
		status.enabled = false;
		SetHeterodynePath(false);
		Synthesizer.RFEnable(false);
		Synthesizer.ChipEnable(false);
		FPGA::SetGPIO(FPGA::GPIO::MOD_DISABLE);
		FPGA::UpdateGPIO();
		SetAttenuator(Attenuation::DB45);
		DetectorEnable(false);
		LOG(Log_RF, LevelInfo, "Output disabled");
		return;
	}
	if (f < 250000000) {
		SetHeterodynePath(true);
		f = 1000000000 - f;
	} else {
		SetHeterodynePath(false);
	}
	Synthesizer.ChipEnable(true);
	Synthesizer.SetFrequency(f);
	LPF lpf = LPF::LP2G5;
	if (f < 340000000) {
		lpf = LPF::LP340M;
	} else if (f < 500000000) {
		lpf = LPF::LP500M;
	} else if (f < 750000000) {
		lpf = LPF::LP750M;
	} else if (f < 1100000000) {
		lpf = LPF::LP1G1;
	} else if (f < 1700000000) {
		lpf = LPF::LP1G7;
	}
	SetLPF(lpf);
	SetAttenuator(Attenuation::DB45);

	// power at detector is 9db below output power
	int32_t cdbm_det = cdbm - 900;

	// calculate voltage at detector output
	constexpr float slope = 33.7f; // in mV/dbm
	constexpr float intercept = -69.2; // dbm at 0V output
	constexpr int16_t cdbm_offset = intercept * 100;
	constexpr uint32_t gain = slope * 10;
	uint32_t voltage = ((cdbm_det - cdbm_offset) * gain) / 1000;
	if (voltage < 0) {
		voltage = 0;
	} else if (voltage > 4095) {
		voltage = 4095;
	}
	PowerDAC.Set(MCP48X2::Channel::A, voltage, true);
	LOG(Log_RF, LevelDebug, "Requested output level of %d.%02udbm", cdbm / 100,
			abs(cdbm) % 100);

	// turn on RF path
	Synthesizer.Update();
	FPGA::ResetGPIO(FPGA::GPIO::MOD_DISABLE);
	Synthesizer.RFEnable(true);
	status.enabled = true;
	LOG(Log_RF, LevelInfo, "Output enabled");
}

void RF::DetectorEnable(bool en) {
	if (en) {
		DET_EN_GPIO_Port->BSRR = DET_EN_Pin << 16;
		LOG(Log_RF, LevelDebug, "Detector enabled");
	} else {
		DET_EN_GPIO_Port->BSRR = DET_EN_Pin;
		LOG(Log_RF, LevelDebug, "Detector disabled");
	}
}

void RF::SetLPF(LPF l) {
	using namespace FPGA;
	switch(l) {
	case LPF::LP340M:
		SetGPIO(GPIO::SW1_CTL1 | GPIO::SW1_CTL3 | GPIO::SW2_CTL2);
		ResetGPIO(GPIO::SW1_CTL2 | GPIO::SW2_CTL1 | GPIO::SW2_CTL3);
		LOG(Log_RF, LevelDebug, "Selected 340MHz filter");
		break;
	case LPF::LP500M:
		SetGPIO(GPIO::SW1_CTL3 | GPIO::SW2_CTL1 | GPIO::SW2_CTL2);
		ResetGPIO(GPIO::SW1_CTL1 | GPIO::SW1_CTL2 | GPIO::SW2_CTL3);
		LOG(Log_RF, LevelDebug, "Selected 500MHz filter");
		break;
	case LPF::LP750M:
		SetGPIO(GPIO::SW1_CTL1 | GPIO::SW1_CTL2 | GPIO::SW2_CTL3);
		ResetGPIO(GPIO::SW1_CTL3 | GPIO::SW2_CTL1 | GPIO::SW2_CTL2);
		LOG(Log_RF, LevelDebug, "Selected 750MHz filter");
		break;
	case LPF::LP1G1:
		SetGPIO(GPIO::SW1_CTL2 | GPIO::SW2_CTL1 | GPIO::SW2_CTL3);
		ResetGPIO(GPIO::SW1_CTL1 | GPIO::SW1_CTL3 | GPIO::SW2_CTL2);
		LOG(Log_RF, LevelDebug, "Selected 1.1GHz filter");
		break;
	case LPF::LP1G7:
		SetGPIO(GPIO::SW1_CTL1 | GPIO::SW2_CTL1 | GPIO::SW2_CTL2 | GPIO::SW2_CTL3);
		ResetGPIO(GPIO::SW1_CTL2 | GPIO::SW1_CTL3);
		LOG(Log_RF, LevelDebug, "Selected 1.7GHz filter");
		break;
	case LPF::LP2G5:
		SetGPIO(GPIO::SW1_CTL1 | GPIO::SW1_CTL2 | GPIO::SW1_CTL3 | GPIO::SW2_CTL1);
		ResetGPIO(GPIO::SW2_CTL2 | GPIO::SW2_CTL3);
		LOG(Log_RF, LevelDebug, "Selected 2.5GHz filter");
		break;
	}
	UpdateGPIO();
}

static void NewADCSamples(uint16_t *data, uint16_t len) {
	if(!status.enabled) {
		// don't do level adjustments if disabled
		return;
	}
	uint32_t sum = 0;
	for (uint16_t i = 0; i < len; i++) {
		sum += *data++;
	}
	sum /= len;

	/*
	 * voltage controlled attenuator has a usable input range of about
	 * 0.3-2.1V. If the control signal is out of this range, the output
	 * power does not meet the requested value.
	 */
	constexpr float ADC_reference = 3.3f;
	constexpr float voltage_min = 0.3f;
	constexpr float voltage_max = 2.1f;
	constexpr uint16_t ADC_max = 4095;
	constexpr uint16_t low_threshold = voltage_min / ADC_reference * ADC_max;
	constexpr uint16_t high_threshold = voltage_max / ADC_reference * ADC_max;

	static uint32_t last_adjust = HAL_GetTick();
	constexpr uint32_t adjust_delay = 100;

	if (sum < low_threshold) {
		// RF output level is too low
		status.unlevel = true;
		// adjust attenuator if possible
		if (HAL_GetTick() - last_adjust >= adjust_delay) {
			switch (status.used_att) {
			case RF::Attenuation::DB45:
				RF::SetAttenuator(RF::Attenuation::DB30);
				break;
			case RF::Attenuation::DB30:
				RF::SetAttenuator(RF::Attenuation::DB15);
				break;
			case RF::Attenuation::DB15:
				RF::SetAttenuator(RF::Attenuation::DB0);
				break;
			default:
				break;
				last_adjust = HAL_GetTick();
			}
		}
	} else if (sum > high_threshold) {
		// RF output level is too high
		status.unlevel = true;
		// adjust attenuator if possible
		if (HAL_GetTick() - last_adjust >= adjust_delay) {
			switch (status.used_att) {
			case RF::Attenuation::DB0:
				RF::SetAttenuator(RF::Attenuation::DB15);
				break;
			case RF::Attenuation::DB15:
				RF::SetAttenuator(RF::Attenuation::DB30);
				break;
			case RF::Attenuation::DB30:
				RF::SetAttenuator(RF::Attenuation::DB45);
				break;
			default:
				break;
			}
			last_adjust = HAL_GetTick();
		}
	} else {
		status.unlevel = false;
	}

	// also check lock status in ADC interrupt
	status.synth_unlocked = !Synthesizer.Locked();
	status.lo_unlocked = !HeterodyneLO.Locked();
}

extern "C" {
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	NewADCSamples(&ADC_Samples[samplelength / 2], samplelength / 2);
}
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
	NewADCSamples(&ADC_Samples[0], samplelength / 2);
}
}