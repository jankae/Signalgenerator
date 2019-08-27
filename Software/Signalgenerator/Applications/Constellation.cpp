#include "Constellation.hpp"

#include <math.h>

static constexpr Constellation::Point BPSK[] = {
		{INT16_MAX, 0},
		{INT16_MIN, 0},
};
static constexpr Constellation::Point QPSK[] = {
		{-(int16_t) (INT16_MAX*std::sqrt(2)), -(int16_t) (INT16_MAX*std::sqrt(2))},
		{-(int16_t) (INT16_MAX*std::sqrt(2)), (int16_t) (INT16_MAX*std::sqrt(2))},
		{(int16_t) (INT16_MAX*std::sqrt(2)), -(int16_t) (INT16_MAX*std::sqrt(2))},
		{(int16_t) (INT16_MAX*std::sqrt(2)), (int16_t) (INT16_MAX*std::sqrt(2))},
};
static constexpr Constellation::Point PSK8[] = {
		{-(int16_t) (INT16_MAX*std::sqrt(2)), -(int16_t) (INT16_MAX*std::sqrt(2))},
		{-INT16_MAX, 0},
		{0, INT16_MAX},
		{-(int16_t) (INT16_MAX*std::sqrt(2)), (int16_t) (INT16_MAX*std::sqrt(2))},
		{0, -INT16_MAX},
		{(int16_t) (INT16_MAX*std::sqrt(2)), -(int16_t) (INT16_MAX*std::sqrt(2))},
		{(int16_t) (INT16_MAX*std::sqrt(2)), (int16_t) (INT16_MAX*std::sqrt(2))},
		{INT16_MAX, 0},
};
static constexpr Constellation::Point QAM16[] = {
		{-INT16_MAX, INT16_MAX},
		{-INT16_MAX, INT16_MAX/3},
		{-INT16_MAX, -INT16_MAX},
		{-INT16_MAX, -INT16_MAX/3},
		{-INT16_MAX/3, INT16_MAX},
		{-INT16_MAX/3, INT16_MAX/3},
		{-INT16_MAX/3, -INT16_MAX},
		{-INT16_MAX/3, -INT16_MAX/3},
		{INT16_MAX/3, INT16_MAX},
		{INT16_MAX/3, INT16_MAX/3},
		{INT16_MAX/3, -INT16_MAX},
		{INT16_MAX/3, -INT16_MAX/3},
		{INT16_MAX, INT16_MAX},
		{INT16_MAX, INT16_MAX/3},
		{INT16_MAX, -INT16_MAX},
		{INT16_MAX, -INT16_MAX/3},
};

Constellation::Constellation(Type type) {
	switch (type) {
	case Type::BPSK:
		usedPoints = 2;
		points = BPSK;
		break;
	case Type::QPSK:
		usedPoints = 4;
		points = QPSK;
		break;
	case Type::PSK8:
		usedPoints = 8;
		points = PSK8;
		break;
	case Type::QAM16:
		usedPoints = 16;
		points = QAM16;
		break;
	case Type::UserDefined:
		usedPoints = 0;
		points = nullptr;
		break;
	}
}
