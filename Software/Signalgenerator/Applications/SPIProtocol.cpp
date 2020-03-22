#include "SPIProtocol.hpp"
#include <cstring>
#include "common.hpp"
#include "Unit.hpp"
#include "HardwareLimits.hpp"


void Protocol::SetupModulation(FrontToRF &data, Modulation mod) {
	// clear any possible previous settings
	memset(data.modulation_registers, 0, sizeof(data.modulation_registers));
	// Setup the modulation source
	// select the source type
	data.modulation_registers[(int)FPGA::Reg::CTRL] |= (int)mod.source;
	switch(mod.source) {
	case FPGA::ModSrc::Disabled:
		// nothing to do
		break;
	case FPGA::ModSrc::Fixed:
		data.modulation_registers[(int)FPGA::Reg::SourcePINC] |= mod.Fixed.value;
		break;
	case FPGA::ModSrc::ExtI:
	case FPGA::ModSrc::ExtQ: {
		data.Status.ADCCouplingDC = mod.ExtSrc.ACCoupled ? 0 : 1;
		data.Status.ADCImp1M = mod.ExtSrc.Impedance50R ? 0 : 1;
		// Calculate required ADC range by highest/lowest expected value
		uint32_t maxADCVoltage;
		if (abs(mod.ExtSrc.maxVoltage) > abs(mod.ExtSrc.minVoltage)) {
			maxADCVoltage = abs(mod.ExtSrc.maxVoltage);
		} else {
			maxADCVoltage = abs(mod.ExtSrc.minVoltage);
		}
		int32_t ExtADCOffset = (mod.ExtSrc.maxVoltage + mod.ExtSrc.minVoltage)
				/ 2;
		int32_t ExtADCSpan = mod.ExtSrc.maxVoltage - mod.ExtSrc.minVoltage;
		// Two voltage dividers are available:
		// Range1 attenuates the signal to one fourth, Range2 to 3/50
		if (maxADCVoltage > HardwareLimits::MaxExtModADCVoltage * 4) {
			// needs range with maximum attenuation
			data.Status.ADCRange1 = 0;
			data.Status.ADCRange2 = 0;
			ExtADCOffset = ExtADCOffset * 3 / 50;
			ExtADCSpan = ExtADCSpan * 3 / 50;
		} else if (maxADCVoltage
				> HardwareLimits::MaxExtModADCVoltage) {
			// needs middle range
			data.Status.ADCRange1 = 1;
			data.Status.ADCRange2 = 0;
			ExtADCOffset = ExtADCOffset / 4;
			ExtADCSpan = ExtADCSpan / 4;
		} else {
			// can use no attenuation before ADC
			data.Status.ADCRange1 = 0;
			data.Status.ADCRange2 = 1;
			maxADCVoltage = mod.External.maxVoltage;
		}
		// calculate ADC offset value
		int16_t offset = (int32_t) (ExtADCOffset
				* HardwareLimits::MaxExtModADCValue)
				/ HardwareLimits::MaxExtModADCVoltage;
		if (offset >= HardwareLimits::MaxExtModADCValue) {
			offset = HardwareLimits::MaxExtModADCValue - 1;
		} else if (offset < -HardwareLimits::MaxExtModADCValue) {
			offset = -HardwareLimits::MaxExtModADCValue;
		}
		uint16_t scale = FPGA::CalculateExtADCScaling(ExtADCSpan / 2);
		if (mod.source == FPGA::ModSrc::ExtI) {
			data.modulation_registers[(int) FPGA::Reg::ExtIOffset] = 0x8000
					| (offset & 0x03FF);
			data.modulation_registers[(int) FPGA::Reg::ExtIScale] = scale;
		} else {
			data.modulation_registers[(int) FPGA::Reg::ExtQOffset] = 0x8000
					| (offset & 0x03FF);
			data.modulation_registers[(int) FPGA::Reg::ExtQScale] = scale;
		}
	}
		break;
	case FPGA::ModSrc::FIFO:
	case FPGA::ModSrc::PRBS:
	case FPGA::ModSrc::RampDown:
	case FPGA::ModSrc::RampUp:
	case FPGA::ModSrc::Sine:
	case FPGA::ModSrc::Square:
	case FPGA::ModSrc::Triangle:
		// all other cases use a DDS to generate the source signal
		uint32_t pinc = ((uint64_t) mod.Periodic.frequency)
				* (1ULL << HardwareLimits::BitsModSrcPinc)
				/ HardwareLimits::FPGA_CLK;
		// lower 16 bits in register 0, higher 4 bits in highest nibble of register 3
		data.modulation_registers[(int)FPGA::Reg::SourcePINC] |= pinc & 0xFFFF;
		data.modulation_registers[(int)FPGA::Reg::CTRL] |= (pinc & 0xF0000) >> 4;
		break;
	}

	// Setup the modulation scheme
	// set modulation type
	FPGA::ModType type = FPGA::ModType::Disabled;
	switch(mod.type) {
	case ModulationType::AM:
		type = FPGA::ModType::AM;
		// 0: 0% depth, 65535: 100% depth
		data.modulation_registers[(int) FPGA::Reg::Setting1] |= common_Map(
				mod.AM.depth, 0, Unit::maxPercent, 0, UINT16_MAX);
		break;
	case ModulationType::FM:
	case ModulationType::FM_USB:
	case ModulationType::FM_LSB:
		type = FPGA::ModType::FM;
		// 0: 0 deviation, 65535: 6248378Hz deviation
		data.modulation_registers[(int) FPGA::Reg::Setting1] |= common_Map(
				mod.FM.deviation, 0, HardwareLimits::MaxFMDeviation, 0,
				HardwareLimits::MaxFMDeviationSetting);
		data.modulation_registers[(int) FPGA::Reg::Setting2] =
				mod.FM.phase_offset;
		break;
	case ModulationType::QAM2:
	case ModulationType::QAM4:
	case ModulationType::QAM8:
	case ModulationType::QAM16:
	case ModulationType::QAM32: {
		if(mod.QAM.differential) {
			type = FPGA::ModType::QAMDifferential;
		} else {
			type = FPGA::ModType::QAM;
		}
		data.modulation_registers[(int) FPGA::Reg::Setting1] |=
				(uint16_t) mod.QAM.SamplesPerSymbol << 8;
		// calculate PINC
		uint64_t SamplesPerSecond = mod.QAM.SamplesPerSymbol
				* mod.QAM.SymbolsPerSecond;
		uint32_t pinc = SamplesPerSecond
				* (1ULL << HardwareLimits::BitsQAMSampleratePinc)
				/ HardwareLimits::FPGA_CLK;
		data.modulation_registers[(int) FPGA::Reg::Setting2] |= pinc & 0xFFFF;
		data.modulation_registers[(int) FPGA::Reg::Setting3] |= (pinc >> 16)
				& 0xFFFF;
	}
		break;
	case ModulationType::External:
		type = FPGA::ModType::External;
		// Enable external ADC sampling
		data.modulation_registers[(int) FPGA::Reg::ExtIOffset] |= 0x8000;
		data.modulation_registers[(int) FPGA::Reg::ExtQOffset] |= 0x8000;
		data.Status.ADCCouplingDC = mod.External.ACCoupled ? 0 : 1;
		data.Status.ADCImp1M = mod.External.Impedance50R ? 0 : 1;
		uint32_t maxADCVoltage;
		// Two voltage dividers are available:
		// Range1 attenuates the signal to one fourth, Range2 to 3/50
		if (mod.External.maxVoltage > HardwareLimits::MaxExtModADCVoltage * 4) {
			// needs range with maximum attenuation
			data.Status.ADCRange1 = 0;
			data.Status.ADCRange2 = 0;
			maxADCVoltage = mod.External.maxVoltage * 3 / 50;
		} else if (mod.External.maxVoltage
				> HardwareLimits::MaxExtModADCVoltage) {
			// needs middle range
			data.Status.ADCRange1 = 1;
			data.Status.ADCRange2 = 0;
			maxADCVoltage = mod.External.maxVoltage / 4;
		} else {
			// can use no attenuation before ADC
			data.Status.ADCRange1 = 0;
			data.Status.ADCRange2 = 1;
			maxADCVoltage = mod.External.maxVoltage;
		}
		auto scaling = FPGA::CalculateExtADCScaling(maxADCVoltage);
		data.modulation_registers[(int) FPGA::Reg::ExtIScale] = scaling;
		data.modulation_registers[(int) FPGA::Reg::ExtQScale] = scaling;
		break;
	}
	data.modulation_registers[(int) FPGA::Reg::CTRL] |= (int) type;

	switch(mod.type) {
	case ModulationType::AM:
	case ModulationType::FM:
	case ModulationType::FM_USB:
	case ModulationType::FM_LSB:
	case ModulationType::External:
		break;
	// in QAM modulation, settings1 determines bitmask of bits per symbol
	case ModulationType::QAM2:
		data.modulation_registers[(int) FPGA::Reg::Setting1] |= 0x0001;
		break;
	case ModulationType::QAM4:
		data.modulation_registers[(int) FPGA::Reg::Setting1] |= 0x0003;
		break;
	case ModulationType::QAM8:
		data.modulation_registers[(int) FPGA::Reg::Setting1] |= 0x0007;
		break;
	case ModulationType::QAM16:
		data.modulation_registers[(int) FPGA::Reg::Setting1] |= 0x000F;
		break;
	case ModulationType::QAM32:
		data.modulation_registers[(int) FPGA::Reg::Setting1] |= 0x001F;
		break;
	}
}
