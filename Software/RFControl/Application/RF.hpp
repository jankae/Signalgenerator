#pragma once

#include <stdint.h>
#include "SPIProtocol.hpp"

namespace RF {

enum class LPF : uint8_t {
	LP340M,
	LP500M,
	LP750M,
	LP1G1,
	LP1G7,
	LP2G5
};
enum class Attenuation : uint8_t {
	DB0,
	DB15,
	DB30,
	DB45,
};
using Status = struct {
	bool enabled;
	bool unlevel;
	bool synth_unlocked;
	bool lo_unlocked;
	Attenuation used_att;
};

void Init(Protocol::RFToFront *s);
void Configure(uint64_t f, int16_t mdbm);
void SetAttenuator(Attenuation a);
void SetHeterodynePath(bool use);
void DetectorEnable(bool en);
void SetLPF(LPF l);
void InternalReference(bool enabled);

}
