#pragma once

#include <stdint.h>
#include "SPIProtocol.hpp"

/*
 * Choose either digital or analog amplitude control.
 *
 * Analog control uses the OpAmp inside the HMC1021 (IC27) to control
 * the output amplitude. This does not work well for modulations with
 * changing amplitude levels (control loop works against modulation).
 *
 * To use analog control, use these components:
 * R62, R61: 10k
 * R60: 1u capacitor
 * R59, C155, R65: DNP
 *
 * Digital control uses the HMC1021 only to measure the output amplitude.
 * The variable attenuator is directly controlled via IC28, channel B.
 *
 * To use digital control, use these components:
 * R62, R61, C155: DNP
 * R65: 0 Ohm
 * R59, R60: 4k7
 */
//#define AMP_CTRL_ANALOG
#define AMP_CTRL_DIGITAL

#if (defined(AMP_CTRL_ANALOG) && defined(AMP_CTRL_DIGITAL))
|| (!defined(AMP_CTRL_ANALOG) && !defined(AMP_CTRL_DIGITAL))
#error Define either AMP_CTRL_DIGITAL OR AMP_CTRL_ANALOG
#endif


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
#ifdef AMP_CTRL_DIGITAL
	bool stabilized;
	uint8_t stabilized_cnt;
	int16_t used_variable_att;
	int16_t requestedcdbm;
#endif
};

void Init(Protocol::RFToFront *s);
void Configure(uint64_t f, int16_t mdbm);
void SetAttenuator(Attenuation a);
void SetHeterodynePath(bool use);
void DetectorEnable(bool en);
void SetLPF(LPF l);
void InternalReference(bool enabled);
bool Stabilized();

}
