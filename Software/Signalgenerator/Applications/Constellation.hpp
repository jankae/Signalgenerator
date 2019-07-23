#pragma once

#include <stdint.h>
#include <array>

class Constellation {
public:
	using Point = struct {
		int16_t I;
		int16_t Q;
	};

	enum class Type : uint8_t {
		BPSK,
		QPSK,
		PSK8,
		QAM4,
		QAM8,
		QAM16,
		QAM32,
		QAM64,
		QAM128,
		QAM256,
		UserDefined,
	};

	Constellation(Type type);
private:
	static constexpr uint16_t MaxPoints = 256;
	uint16_t usedPoints;
	Point *points;
};
