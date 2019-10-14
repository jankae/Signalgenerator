#include "SPIProtocol.hpp"
#include <cstring>
#include "common.hpp"
#include "Unit.hpp"

static constexpr uint32_t FPGA_CLK = 100000000;

void Protocol::SetupModulation(FrontToRF &data, Modulation mod) {
	// clear any possible previous settings
	memset(data.modulation_registers, 0, sizeof(data.modulation_registers));
	// Setup the modulation source
	// select the source type
	data.modulation_registers[3] |= (((uint8_t) mod.source) << 8);
	switch(mod.source) {
	case SourceType::FixedValue:
		data.modulation_registers[0] = mod.Fixed.value;
		break;
	default:
		// all other cases use a DDS to generate the source signal
		data.modulation_registers[0] = ((uint64_t) mod.Periodic.frequency)
								* (1ULL << 27) / FPGA_CLK;
	}

	// Setup the modulation scheme
	// set modulation type
	uint8_t type = 0x00;
	switch(mod.type) {
	case ModulationType::AM:
		type = 0x08;
		break;
	case ModulationType::FM:
		type = 0x04;
		break;
	case ModulationType::FM_USB:
		type = 0x06;
		break;
	case ModulationType::FM_LSB:
		type = 0x05;
		break;
	case ModulationType::QAM2:
	case ModulationType::QAM4:
	case ModulationType::QAM8:
	case ModulationType::QAM16:
	case ModulationType::QAM32: {
		if(mod.QAM.differential) {
			type = 0x0D;
		} else {
			type = 0x0C;
		}
		data.modulation_registers[1] |= (uint16_t) mod.QAM.SamplesPerSymbol << 8;
		// calculate PINC
		uint64_t SamplesPerSecond = mod.QAM.SamplesPerSymbol
				* mod.QAM.SymbolsPerSecond;
		uint32_t pinc = SamplesPerSecond * (1ULL << 32) / FPGA_CLK;
		data.modulation_registers[2] = pinc & 0xFFFF;
		data.modulation_registers[4] = (pinc >> 16) & 0xFFFF;
	}
		break;
	}
	data.modulation_registers[3] |= type;

	switch(mod.type) {
	case ModulationType::AM:
		// 0: 0% depth, 65535: 100% depth
		data.modulation_registers[1] = common_Map(mod.AM.depth, 0, Unit::maxPercent, 0, UINT16_MAX);
		break;
	case ModulationType::FM:
	case ModulationType::FM_USB:
	case ModulationType::FM_LSB:
		// 0: 0 deviation, 65535: 6248378Hz deviation
		data.modulation_registers[1] = common_Map(mod.FM.deviation, 0, 6248378, 0, UINT16_MAX);
		break;
	// in QAM modulation, settings1 determines bitmask of bits per symbol
	case ModulationType::QAM2:
		data.modulation_registers[1] |= 0x0001;
		break;
	case ModulationType::QAM4:
		data.modulation_registers[1] |= 0x0003;
		break;
	case ModulationType::QAM8:
		data.modulation_registers[1] |= 0x0007;
		break;
	case ModulationType::QAM16:
		data.modulation_registers[1] |= 0x000F;
		break;
	case ModulationType::QAM32:
		data.modulation_registers[1] |= 0x001F;
		break;
	}
}
