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
		QAM16,
		UserDefined,
	};

	Constellation(Type type);
private:
	static constexpr uint16_t MaxPoints = 16;
	uint16_t usedPoints;
	Point *points;
};
