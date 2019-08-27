#include "RF.hpp"

#include "Drivers/mcp48x2.hpp"
#include "Drivers/max2871.hpp"
#include "Drivers/adf4360.hpp"
#include "Drivers/delay.hpp"
#include "Drivers/fpga.hpp"
#include <cmath>
#include "System/log.h"

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
static constexpr uint16_t samplelength = 256;
static uint16_t ADC_Samples[samplelength];

static RF::Status status;

static Protocol::RFToFront *spi_status;

// HMC1021 detector values
constexpr float slope = 16.85f; // in mV/dbm
constexpr float intercept = -67.5; // dbm at 0V output
constexpr int16_t cdbm_offset = intercept * 100;
constexpr uint32_t gain = slope * 10;

void RF::Init(Protocol::RFToFront *s) {
	spi_status = s;
	status.enabled = false;
	status.unlevel = false;
	status.lo_unlocked = false;
	status.synth_unlocked = false;
	status.used_att = Attenuation::DB45;
	InternalReference(false);
	HAL_ADCEx_Calibration_Start(&hadc);
	HAL_ADC_Start_DMA(&hadc, (uint32_t*) ADC_Samples, samplelength);
	FPGA::SetDAC(0x0FFF, 0x0800);
	PowerDAC.Shutdown(MCP48X2::Channel::A);
	PowerDAC.Shutdown(MCP48X2::Channel::B);
	HeterodyneLO.Init();
	Synthesizer.Init();
	Configure(0, 0);
	LOG(Log_RF, LevelInfo, "Initialized");

	/* BEGIN TESTS */
//	HAL_Delay(2000);
//	InternalReference(true);
//	HeterodyneLO.ChipEnable(true);
//	HeterodyneLO.Init();
//	HeterodyneLO.SetFrequency(1000000000);
//	HeterodyneLO.Update();
//	Configure(500000000, -1000);
//	SetAttenuator(Attenuation::DB30);
//	DetectorEnable(true);
//	PowerDAC.Set(MCP48X2::Channel::A, 900, true);
//	DetectorEnable(true);
//	SetAttenuator(Attenuation::DB0);
//	SetHeterodynePath(false);

//	while(1) {
//		FPGA::SetGPIO(FPGA::GPIO::LED1);
//		FPGA::SetGPIO(FPGA::GPIO::LED2);
//		FPGA::SetGPIO(FPGA::GPIO::LED3);
//		FPGA::SetGPIO(FPGA::GPIO::LED4);
//		FPGA::SetGPIO(FPGA::GPIO::LED5);
//		FPGA::UpdateGPIO();
//		HAL_Delay(1000);
//		FPGA::ResetGPIO(FPGA::GPIO::LED1);
//		FPGA::ResetGPIO(FPGA::GPIO::LED2);
//		FPGA::ResetGPIO(FPGA::GPIO::LED3);
//		FPGA::ResetGPIO(FPGA::GPIO::LED4);
//		FPGA::ResetGPIO(FPGA::GPIO::LED5);
//		FPGA::UpdateGPIO();
//		HAL_Delay(1000);
//	}
	/* END TESTS */
}

void RF::SetAttenuator(Attenuation a) {
	switch (a) {
	case RF::Attenuation::DB0:
		DB15_1_GPIO_Port->BSRR = DB15_1_Pin << 16;
		DB15_2_GPIO_Port->BSRR = DB15_2_Pin << 16;
		DB15_3_GPIO_Port->BSRR = DB15_3_Pin << 16;
		LOG(Log_RF, LevelDebug, "Output attenuation 0db");
		spi_status->Status.n15dbm1 = 0;
		spi_status->Status.n15dbm2 = 0;
		spi_status->Status.n15dbm3 = 0;
		break;
	case RF::Attenuation::DB15:
		DB15_1_GPIO_Port->BSRR = DB15_1_Pin << 16;
		DB15_2_GPIO_Port->BSRR = DB15_2_Pin << 16;
		DB15_3_GPIO_Port->BSRR = DB15_3_Pin;
		LOG(Log_RF, LevelDebug, "Output attenuation 15db");
		spi_status->Status.n15dbm1 = 0;
		spi_status->Status.n15dbm2 = 0;
		spi_status->Status.n15dbm3 = 1;
		break;
	case RF::Attenuation::DB30:
		DB15_1_GPIO_Port->BSRR = DB15_1_Pin << 16;
		DB15_2_GPIO_Port->BSRR = DB15_2_Pin;
		DB15_3_GPIO_Port->BSRR = DB15_3_Pin;
		LOG(Log_RF, LevelDebug, "Output attenuation 30db");
		spi_status->Status.n15dbm1 = 0;
		spi_status->Status.n15dbm2 = 1;
		spi_status->Status.n15dbm3 = 1;
		break;
	case RF::Attenuation::DB45:
		DB15_1_GPIO_Port->BSRR = DB15_1_Pin;
		DB15_2_GPIO_Port->BSRR = DB15_2_Pin;
		DB15_3_GPIO_Port->BSRR = DB15_3_Pin;
		LOG(Log_RF, LevelDebug, "Output attenuation 45db");
		spi_status->Status.n15dbm1 = 1;
		spi_status->Status.n15dbm2 = 1;
		spi_status->Status.n15dbm3 = 1;
		break;
	}
	status.used_att = a;
}

void RF::SetHeterodynePath(bool use) {
	if(use) {
		// Set PLL to generate 1GHz tone
		HeterodyneLO.ChipEnable(true);
		HeterodyneLO.Init();
		HeterodyneLO.SetFrequency(1000000000);
		HeterodyneLO.Update();
		// enable mixer
		MIX_EN_GPIO_Port->BSRR = MIX_EN_Pin;
		// set switches to heterodyne path
		EN_DIRECT_GPIO_Port->BSRR = EN_DIRECT_Pin << 16;
		LOG(Log_RF, LevelDebug, "Heterodyne path enabled");
		spi_status->Status.HeterodynePLLON = 1;
	} else {
		// disable PLL
		HeterodyneLO.ChipEnable(false);
		// disable mixer
		MIX_EN_GPIO_Port->BSRR = MIX_EN_Pin << 16;
		// set switches to direct path
		EN_DIRECT_GPIO_Port->BSRR = EN_DIRECT_Pin;
		LOG(Log_RF, LevelDebug, "Heterodyne path disabled");
		spi_status->Status.HeterodynePLLON = 0;
	}
}

void RF::Configure(uint64_t f, int16_t cdbm) {
	if (f == 0) {
		status.enabled = false;
		SetHeterodynePath(false);
		Synthesizer.RFEnable(false);
		Synthesizer.ChipEnable(false);
		spi_status->Status.MainPLLON = 0;
		FPGA::SetGPIO(FPGA::GPIO::MOD_DISABLE);
		spi_status->Status.IQModEnabled = 0;
		FPGA::UpdateGPIO();
		SetAttenuator(Attenuation::DB45);
		DetectorEnable(false);
		LOG(Log_RF, LevelInfo, "Output disabled");
		spi_status->Status.AmplitudeUnlevel = 0;
		spi_status->Status.MainPLLUnlocked = 0;
		spi_status->Status.HeterodynePLLUnlock = 0;
		return;
	}
	if (f < 250000000) {
		SetHeterodynePath(true);
		f = 1000000000 + f;
	} else {
		SetHeterodynePath(false);
	}
	Synthesizer.ChipEnable(true);
	Synthesizer.SetFrequency(f);
	/* Measured LPF corner frequencies (all quite low, probably due to extra layout capacitances)
	 * 340: 270
	 * 500: 410
	 * 750: 570
	 * 1G1: 850
	 * 1G7: 1140
	 * 2G5: 2100
	 */
	LPF lpf = LPF::LP2G5;
	if (f < 270000000) {
		lpf = LPF::LP340M;
	} else if (f < 410000000) {
		lpf = LPF::LP500M;
	} else if (f < 570000000) {
		lpf = LPF::LP750M;
	} else if (f < 850000000) {
		lpf = LPF::LP1G1;
	} else if (f < 1100000000) {
		lpf = LPF::LP1G7;
	}
	SetLPF(lpf);
	SetAttenuator(Attenuation::DB45);

#ifdef AMP_CTRL_ANALOG
	// power at detector is 9db below output power
	int32_t cdbm_det = cdbm - 900;

	// calculate voltage at detector output
	uint32_t voltage = ((cdbm_det - cdbm_offset) * gain) / 1000;
	if (voltage < 0) {
		voltage = 0;
	} else if (voltage > 4095) {
		voltage = 4095;
	}
	PowerDAC.Set(MCP48X2::Channel::A, voltage, true);
#endif
#ifdef AMP_CTRL_DIGITAL
	status.requestedcdbm = cdbm;
	// set highest possible attenuation at startup
	status.used_variable_att = MCP48X2::MaxValue / 2;
	PowerDAC.Set(MCP48X2::Channel::B, status.used_variable_att * 2, false);
	status.unlevel = true;
	spi_status->Status.AmplitudeUnlevel = 0;
#endif
	DetectorEnable(true);
	LOG(Log_RF, LevelDebug, "Requested output level of %d.%02udbm", cdbm / 100,
			abs(cdbm) % 100);

	// turn on RF path
	Synthesizer.Update();
	FPGA::ResetGPIO(FPGA::GPIO::MOD_DISABLE);
	FPGA::UpdateGPIO();
	spi_status->Status.IQModEnabled = 1;
	Synthesizer.RFEnable(true);
	spi_status->Status.MainPLLON = 1;
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
	spi_status->Status.Filter = (uint16_t) l;
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

	constexpr float ADC_reference = 3.3f;
	constexpr uint16_t ADC_max = 4095;
#ifdef AMP_CTRL_ANALOG

	/*
	 * voltage controlled attenuator has a usable input range of about
	 * 0.3-2.1V. If the control signal is out of this range, the output
	 * power does not meet the requested value.
	 */
	constexpr float voltage_min = 0.3f;
	constexpr float voltage_max = 2.1f;
	constexpr uint16_t low_threshold = voltage_min / ADC_reference * ADC_max;
	constexpr uint16_t high_threshold = voltage_max / ADC_reference * ADC_max;

	static uint32_t last_adjust = HAL_GetTick();
	constexpr uint32_t adjust_delay = 10;

	if (sum < low_threshold) {
		// RF output level is too low
		status.unlevel = true;
		spi_status->Status.AmplitudeUnlevel = 1;
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
			}
			last_adjust = HAL_GetTick();
		}
	} else if (sum > high_threshold) {
		// RF output level is too high
		status.unlevel = true;
		spi_status->Status.AmplitudeUnlevel = 1;
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
		spi_status->Status.AmplitudeUnlevel = 0;
	}
#endif
#ifdef AMP_CTRL_DIGITAL

	static uint32_t last_adjust = HAL_GetTick();
	constexpr uint32_t adjust_delay = 10;
	if (HAL_GetTick() - last_adjust >= adjust_delay) {
		last_adjust = HAL_GetTick();
		// calculate measured dbm

		// convert ADC value into voltage
		constexpr uint16_t ADCRefmV = ADC_reference * 1000;
		uint16_t voltage = ADCRefmV * sum / ADC_max;
		int16_t cdbm_det = (voltage * 1000UL) / gain + cdbm_offset;
		// actual output level is 9db higher than level at detector
		int16_t cdbm = cdbm_det + 900;

		int16_t cdbm_diff = status.requestedcdbm - cdbm;

		// Variable attenuator has a slope of approximately -20db/V
		int16_t voltage_change_mV = -cdbm_diff / 2;
		if (status.used_variable_att + voltage_change_mV < 400) {
			// variable attenuator at low attenuation limit, reduce
			// fixed attenuator if possible
			switch (status.used_att) {
			case RF::Attenuation::DB45:
				RF::SetAttenuator(RF::Attenuation::DB30);
				cdbm_diff -= 1500;
				break;
			case RF::Attenuation::DB30:
				RF::SetAttenuator(RF::Attenuation::DB15);
				cdbm_diff -= 1500;
				break;
			case RF::Attenuation::DB15:
				RF::SetAttenuator(RF::Attenuation::DB0);
				cdbm_diff -= 1500;
				break;
			default:
				break;
			}
			voltage_change_mV = -cdbm_diff / 2;
		} else if (status.used_variable_att + voltage_change_mV > 2000) {
			// variable attenuator at high attenuation limit, increase
			// fixed attenuator if possible
			switch (status.used_att) {
			case RF::Attenuation::DB0:
				RF::SetAttenuator(RF::Attenuation::DB15);
				cdbm_diff += 1500;
				break;
			case RF::Attenuation::DB15:
				RF::SetAttenuator(RF::Attenuation::DB30);
				cdbm_diff += 1500;
				break;
			case RF::Attenuation::DB30:
				RF::SetAttenuator(RF::Attenuation::DB45);
				cdbm_diff += 1500;
				break;
			default:
				break;
			}
			voltage_change_mV = -cdbm_diff / 2;
		}
		// adjust variable attenuator
		uint16_t buf = status.used_variable_att;
		status.used_variable_att += (voltage_change_mV * 8) / 10;
		if (status.used_variable_att > MCP48X2::MaxValue) {
			status.used_variable_att = MCP48X2::MaxValue;
			status.unlevel = true;
			spi_status->Status.AmplitudeUnlevel = 1;
		} else if (status.used_variable_att < 0) {
			status.used_variable_att = 0;
			status.unlevel = true;
			spi_status->Status.AmplitudeUnlevel = 1;
		} else {
			status.unlevel = false;
			spi_status->Status.AmplitudeUnlevel = 0;
		}
//		LOG(Log_RF, LevelDebug, "Measured: %d, requested: %d, diff: %d, V_change: %d, old DAC: %u, new DAC: %u",
//				cdbm, status.requestedcdbm, cdbm_diff, voltage_change_mV, buf, status.used_variable_att);
		PowerDAC.Set(MCP48X2::Channel::B, status.used_variable_att * 2, false);
	}
#endif

	// also check lock status in ADC interrupt
	status.synth_unlocked = !Synthesizer.Locked();
	status.lo_unlocked = !HeterodyneLO.Locked();
	spi_status->Status.MainPLLUnlocked = status.synth_unlocked ? 1 : 0;
	spi_status->Status.HeterodynePLLUnlock = status.lo_unlocked ? 1 : 0;
}

extern "C" {
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	NewADCSamples(&ADC_Samples[samplelength / 2], samplelength / 2);
}
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
	NewADCSamples(&ADC_Samples[0], samplelength / 2);
}
}

void RF::InternalReference(bool enabled) {
	if (enabled) {
		EN_INTREF_GPIO_Port->BSRR = EN_INTREF_Pin << 16;
		spi_status->Status.IntRefON = 1;
	} else {
		EN_INTREF_GPIO_Port->BSRR = EN_INTREF_Pin;
		spi_status->Status.IntRefON = 0;
	}
}
