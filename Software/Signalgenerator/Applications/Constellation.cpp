#include "Constellation.hpp"

#include <math.h>

static constexpr Constellation::Point BPSK[] = {
		{INT16_MAX, 0},
		{INT16_MIN, 0},
};
static constexpr Constellation::Point QPSK[] = {
		{(int16_t) (INT16_MAX*std::sqrt(2)), 0},
};

static constexpr Constellation::Point firstQuadrantQAM4[] = {
		{INT16_MAX, INT16_MAX},
};

template<uint16_t size>
struct QAM {
	constexpr QAM() :
			points() {
		for (uint16_t i = 0; i < size; i++) {
			uint16_t quadrant_index = i % (size / 4);
			uint8_t quadrant = 0;
			if(i & (size >> 1)) {
				if(i & (size >> 2)) {
					quadrant = 2;
				} else {
					quadrant = 1;
				}
			} else {
				if(i & (size >> 2)) {
					quadrant = 3;
				} else {
					quadrant = 0;
				}
			}
			switch(size) {
			case 4:
				points[i].I = INT16_MAX;
				points[i].Q = INT16_MAX;
				break;
			}
			switch(quadrant) {
			case 0:
				break;
			case 1: {
				int16_t buf = points[i].I;
				points[i].I = -points[i].Q;
				points[i].Q = buf;
			}
				break;
			case 2:
				points[i].I = -points[i].I;
				points[i].Q = -points[i].Q;
				break;
			case 3: {
				int16_t buf = points[i].I;
				points[i].I = points[i].Q;
				points[i].Q = -buf;
			}
				break;
			}
		}
	}
	Constellation::Point points[size];
};

constexpr auto QAM4 = QAM<4>();

Constellation::Constellation(Type type) {
}
