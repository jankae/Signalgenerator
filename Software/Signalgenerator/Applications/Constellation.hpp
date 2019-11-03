#pragma once

#include <stdint.h>
#include <array>
#include "table.hpp"

class Constellation {
public:
	enum class Type : uint8_t {
		BPSK,
		QPSK,
		PSK8,
		QAM16,
		UserDefined,
	};

	Constellation();
	void Edit();
	void SetUsedPoints(uint8_t used) {
		usedPoints = used;
	}
	static void SetFIRinFPGA(uint8_t sps, float beta);
	bool GetScaledPoint(uint16_t point, uint16_t maxVal, int16_t &i,
			int16_t &q);

private:
	void View();
	static constexpr uint16_t MaxPoints = 32;
	uint16_t usedPoints;
	int16_t I[MaxPoints];
	int16_t Q[MaxPoints];
	uint16_t maxAmplitude;
	Table<int16_t> *table;
};
