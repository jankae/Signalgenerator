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
private:
	void View();
	static constexpr uint16_t MaxPoints = 32;
	uint16_t usedPoints;
	int16_t I[MaxPoints];
	int16_t Q[MaxPoints];
	Table<int16_t> *table;
};
