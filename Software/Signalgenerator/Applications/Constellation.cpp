#include "Constellation.hpp"

#include <math.h>
#include "gui.hpp"
#include "main.h"

extern SPI_HandleTypeDef hspi1;

static constexpr int16_t BPSK_I[] = {
		INT16_MAX,
		INT16_MIN,
};
static constexpr int16_t BPSK_Q[] = {
		0,
		0,
};
static constexpr int16_t QPSK_I[] = {
		-(int16_t) (INT16_MAX/std::sqrt(2)),
		-(int16_t) (INT16_MAX/std::sqrt(2)),
		(int16_t) (INT16_MAX/std::sqrt(2)),
		(int16_t) (INT16_MAX/std::sqrt(2)),
};
static constexpr int16_t QPSK_Q[] = {
		-(int16_t) (INT16_MAX/std::sqrt(2)),
		(int16_t) (INT16_MAX/std::sqrt(2)),
		-(int16_t) (INT16_MAX/std::sqrt(2)),
		(int16_t) (INT16_MAX/std::sqrt(2)),
};
static constexpr int16_t PSK8_I[] = {
		-(int16_t) (INT16_MAX/std::sqrt(2)),
		-INT16_MAX,
		0,
		-(int16_t) (INT16_MAX/std::sqrt(2)),
		0,
		(int16_t) (INT16_MAX/std::sqrt(2)),
		(int16_t) (INT16_MAX/std::sqrt(2)),
		INT16_MAX,
};
static constexpr int16_t PSK8_Q[] = {
		-(int16_t) (INT16_MAX/std::sqrt(2)),
		0,
		INT16_MAX,
		(int16_t) (INT16_MAX/std::sqrt(2)),
		-INT16_MAX,
		-(int16_t) (INT16_MAX/std::sqrt(2)),
		(int16_t) (INT16_MAX/std::sqrt(2)),
		0,
};
static constexpr int16_t QAM16_I[] = {
		-INT16_MAX,
		-INT16_MAX,
		-INT16_MAX,
		-INT16_MAX,
		-INT16_MAX/3,
		-INT16_MAX/3,
		-INT16_MAX/3,
		-INT16_MAX/3,
		INT16_MAX/3,
		INT16_MAX/3,
		INT16_MAX/3,
		INT16_MAX/3,
		INT16_MAX,
		INT16_MAX,
		INT16_MAX,
		INT16_MAX,
};
static constexpr int16_t QAM16_Q[] = {
		INT16_MAX,
		INT16_MAX/3,
		-INT16_MAX,
		-INT16_MAX/3,
		INT16_MAX,
		INT16_MAX/3,
		-INT16_MAX,
		-INT16_MAX/3,
		INT16_MAX,
		INT16_MAX/3,
		-INT16_MAX,
		-INT16_MAX/3,
		INT16_MAX,
		INT16_MAX/3,
		-INT16_MAX,
		-INT16_MAX/3,
};

Constellation::Constellation() {
	usedPoints = 0;
	memset(I, 0, sizeof(I));
	memset(Q, 0, sizeof(Q));
	table = nullptr;
}
void Constellation::Edit() {
	auto w = new Window("Constellation table", Font_Big, COORDS(320, 240));
	auto c = new Container(w->getAvailableArea());
	table = new Table<int16_t>(Font_Big, 11);
	int16_t *symbols = new int16_t[usedPoints];
	for (uint16_t i = 0; i < usedPoints; i++) {
		int16_t val = 0;
		uint32_t spot = 1;
		for (uint32_t j = 0x01; j < 0x8000; j <<= 1) {
			if (i & j) {
				val += spot;
			}
			spot *= 10;
		}
		symbols[i] = val;
	}
	table->AddColumn("Symbol", symbols, (uint8_t) usedPoints, Unit::None, 6, false);
	table->AddColumn("I", I, usedPoints, Unit::None, 6, true);
	table->AddColumn("Q", Q, usedPoints, Unit::None, 6, true);
	c->attach(table, COORDS(1,1));
	auto semphr = xSemaphoreCreateBinary();
	c->attach(new Button("View", Font_Big, [](void* ptr, Widget*){
		Constellation *c = (Constellation*) ptr;
		c->View();
	}, this, COORDS(60, 60)), COORDS(250, 10));
	c->attach(new Button("Default", Font_Medium, [](void* ptr, Widget*){
		Constellation *c = (Constellation*) ptr;
		switch(c->usedPoints) {
		case 2:
			memcpy(c->I, BPSK_I, sizeof(BPSK_I));
			memcpy(c->Q, BPSK_Q, sizeof(BPSK_Q));
			break;
		case 4:
			memcpy(c->I, QPSK_I, sizeof(QPSK_I));
			memcpy(c->Q, QPSK_Q, sizeof(QPSK_Q));
			break;
		case 8:
			memcpy(c->I, PSK8_I, sizeof(PSK8_I));
			memcpy(c->Q, PSK8_Q, sizeof(PSK8_Q));
			break;
		case 16:
			memcpy(c->I, QAM16_I, sizeof(QAM16_I));
			memcpy(c->Q, QAM16_Q, sizeof(QAM16_Q));
			break;
		case 32:
			break;
		}
		c->table->requestRedraw();
	}, this, COORDS(60, 60)), COORDS(250, 80));
	c->attach(new Button("OK", Font_Big, [](void* ptr, Widget*){
		QueueHandle_t s = (QueueHandle_t) ptr;
		xSemaphoreGive(s);
	}, semphr, COORDS(60, 60)), COORDS(250, 150));
	w->setMainWidget(c);
	xSemaphoreTake(semphr, portMAX_DELAY);
	vSemaphoreDelete(semphr);
	delete w;
}

void Constellation::View() {
	auto w = new Window("Constellation diagram", Font_Big, COORDS(320, 240));
	auto c = new Container(w->getAvailableArea());
	using CustomPtr = struct {
		Constellation *cst;
		Window *w;
	};
	CustomPtr *ptr = new CustomPtr;
	ptr->cst = this;
	ptr->w = w;
	Custom *graph = new Custom(COORDS(c->getSize().y + 20, c->getSize().y),
			[](Custom& c, coords_t pos) {
		Constellation *cst = ((CustomPtr*) c.GetPtr())->cst;
		// find absolute maximum I/Q value
		uint16_t maxVal = 0;
		for(uint16_t i=0;i<cst->usedPoints;i++) {
			uint16_t abs_I = abs(cst->I[i]);
			uint16_t abs_Q = abs(cst->Q[i]);
			if(abs_I > maxVal) {
				maxVal = abs_I;
			}
			if(abs_Q > maxVal) {
				maxVal = abs_Q;
			}
		}
		// draw cross hair
		coords_t center = COORDS(pos.x + c.getSize().y/2, pos.y + c.getSize().y/2);
		display_SetForeground(COLOR_BLACK);
		display_SetBackground(COLOR_BG_DEFAULT);
		display_VerticalLine(pos.x + c.getSize().y / 2, pos.y, c.getSize().y);
		display_HorizontalLine(pos.x, pos.y+ c.getSize().y / 2, c.getSize().y);

		// draw arrows
		display_Line(pos.x + c.getSize().y, pos.y+ c.getSize().y / 2,
				pos.x + c.getSize().y - 5, pos.y+ c.getSize().y / 2 + 5);
		display_Line(pos.x + c.getSize().y, pos.y+ c.getSize().y / 2,
				pos.x + c.getSize().y - 5, pos.y+ c.getSize().y / 2 - 5);
		display_SetFont(Font_Medium);
		display_Char(pos.x + c.getSize().y - Font_Medium.width,
				pos.y+ c.getSize().y / 2 + 7, 'I');

		display_Line(pos.x + c.getSize().y / 2, pos.y,
				pos.x + c.getSize().y / 2 - 5, pos.y + 5);
		display_Line(pos.x + c.getSize().y / 2, pos.y,
				pos.x + c.getSize().y / 2 + 5, pos.y + 5);
		display_Char(pos.x + c.getSize().y / 2 + 7,
				pos.y + 2, 'Q');

		// draw dots
		display_SetForeground(COLOR_RED);
		uint16_t maxPos = c.getSize().y / 2 * 8 / 10;
		for(uint16_t i=0;i<cst->usedPoints;i++) {
			coords_t position = COORDS(common_Map(cst->I[i], 0, maxVal, 0, maxPos),
					common_Map(cst->Q[i], 0, maxVal, 0, -maxPos));
			display_SetForeground(COLOR_RED);
			display_CircleFull(center.x + position.x, center.y + position.y, 4);

			// assemble symbol string
			uint8_t digits_symbol = 31 - __builtin_clz(cst->usedPoints);
			char symbol[digits_symbol + 1];
			symbol[digits_symbol] = 0;
			uint16_t mask = 0x0001;
			for(uint8_t j=0;j<digits_symbol;j++) {
				if(i & mask) {
					symbol[j] = '1';
				} else {
					symbol[j] = '0';
				}
				mask <<= 1;
			}
			display_SetFont(Font_Small);
			display_SetForeground(COLOR_BLACK);
			display_String(center.x + position.x + 6,
					center.y + position.y - 2, symbol);
		}
	}, [](Custom& c, GUIEvent_t *ev) {
		Window *w = ((CustomPtr*) c.GetPtr())->w;
		if(ev->type == EVENT_TOUCH_PRESSED) {
			delete (CustomPtr*) c.GetPtr();
			delete w;
		}
	}, ptr);

	c->attach(graph, COORDS(40,0));
	w->setMainWidget(c);
}

static uint16_t htons(uint16_t h) {
	return ((h & 0xFF) << 8) | ((h & 0xFF00) >> 8);
}

static int16_t constrain(int16_t v, int16_t min, int16_t max) {
	if (v > max) {
		v = max;
	} else if (v < min) {
		v = min;
	}
	return v;
}

void Constellation::LoadToFPGA() {
	// find maximum amplitude used in constellation
	uint32_t max_squared = 0;
	for (uint16_t i = 0; i < usedPoints; i++) {
		uint32_t squared = I[i] * I[i] + Q[i] * Q[i];
		if (squared > max_squared) {
			max_squared = squared;
		}
	}
	uint16_t max_amplitude = sqrt(max_squared);
	constexpr uint16_t max_DAC = 2048;

	uint16_t data[2];
	// start loading data to address 0
	data[0] = 0x0000;
	SPI1_CS_FPGA_GPIO_Port->BSRR = SPI1_CS_FPGA_Pin << 16;
	HAL_SPI_Transmit(&hspi1, (uint8_t*) data, 2, 100);
	for (uint16_t i = 0; i < usedPoints; i++) {
		// scale values so that the point with the highest amplitude
		// corresponds to the selected dbm value
		int16_t i_val = (int32_t) I[i] * max_DAC / max_amplitude;
		int16_t q_val = (int32_t) Q[i] * max_DAC / max_amplitude;

		data[0] = htons(constrain(i_val, -max_DAC, max_DAC - 1));
		data[1] = htons(constrain(q_val, -max_DAC, max_DAC - 1));
		HAL_SPI_Transmit(&hspi1, (uint8_t*) data, 4, 100);
	}
	SPI1_CS_FPGA_GPIO_Port->BSRR = SPI1_CS_FPGA_Pin;
}

void Constellation::SetFIRinFPGA(uint8_t sps, float beta) {
	int16_t FIRdata[FIRTaps];
	static constexpr int16_t MaxAmplitude = 1600; // TODO adjust to scale (keep in mind possible overflow in FPGA FIR addition!)
	static constexpr float PI = 3.1415926535f;
	for (uint8_t i = 0; i < FIRTaps; i++) {
		int8_t t = i - FIRTaps / 2;
		float h_t;
		if (t == 0) {
			h_t = 1.0;
		} else if (abs(t) == sps / (2 * beta)) {
			h_t = std::sin(PI / (2 * beta)) / (PI / (2 * beta)) * PI / 4;
		} else {
			h_t = std::sin(PI * t / sps) / (PI * t / sps)
					* std::cos(beta * PI * t / sps)
					/ (1 - (2 * beta * t / sps) * (2 * beta * t / sps));
		}
		printf("FIR tap %d: %f\n", i, h_t);
		FIRdata[i] = htons(h_t * MaxAmplitude);
	}
	// start loading data to address 0
	uint16_t data = htons(0x8000);
	SPI1_CS_FPGA_GPIO_Port->BSRR = SPI1_CS_FPGA_Pin << 16;
	HAL_SPI_Transmit(&hspi1, (uint8_t*) &data, 2, 100);
	HAL_SPI_Transmit(&hspi1, (uint8_t*) FIRdata, sizeof(FIRdata), 100);
	SPI1_CS_FPGA_GPIO_Port->BSRR = SPI1_CS_FPGA_Pin;
}
