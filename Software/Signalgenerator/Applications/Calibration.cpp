#include "Calibration.hpp"

#include "common.hpp"
#include "gui.hpp"
#include "SPIProtocol.hpp"
#include "Persistence.hpp"

extern SPI_HandleTypeDef hspi1;

static constexpr int16_t ideal_low = -2000;
static constexpr int16_t ideal_high = 0;
using Point = struct {
	uint32_t freq;
	int16_t low;
	int16_t high;
};

static constexpr uint8_t maxPoints = 10;
static Point points[maxPoints];

static constexpr Point defaultCalibration[maxPoints] = {
		{10000000, -2000, 0},
		{100000000, -2000, 0},
		{200000000, -2000, 0},
		{500000000, -2000, 0},
		{750000000, -2000, 0},
		{1000000000, -2000, 0},
		{1250000000, -2000, 0},
		{1500000000, -2000, 0},
		{1750000000, -2000, 0},
		{2000000000, -2000, 0},
};

void Calibration::Init() {
	memcpy(points, defaultCalibration, sizeof(points));
	Persistence::Add(points, sizeof(points));
}

void Calibration::Run() {
	uint8_t step = 1;
	uint8_t new_step = 0;
	bool done = false;

	Window *w = new Window("Calibration", Font_Big, COORDS(250, 150));
	Container *c = new Container(w->getAvailableArea());
	Label *lStep = new Label(20, Font_Big, Label::Orientation::CENTER);
	Label *lUsage = new Label("Turn knob until output matches", Font_Medium);
	Label *lValue = new Label(20, Font_Big, Label::Orientation::CENTER, COLOR_RED);
	Button *bPrev = new Button("Prev", Font_Big, [](void *ptr, Widget *w) {
		uint8_t *new_step = (uint8_t*) ptr;
		(*new_step)--;
	}, &new_step, COORDS(80, 40));
	Button *bNext = new Button("Next", Font_Big, [](void *ptr, Widget *w) {
		uint8_t *new_step = (uint8_t*) ptr;
		(*new_step)++;
	}, &new_step, COORDS(80, 40));
	Button *bQuit = new Button("Quit", Font_Big, [](void *ptr, Widget *w) {
		bool *done = (bool*) ptr;
		*done = true;
	}, &done, COORDS(80, 40));

	c->attach(lStep, COORDS(4, 5));
	c->attach(lUsage, COORDS(34, 25));
	c->attach(lValue, COORDS(4, 40));

	c->attach(bPrev, COORDS(2, c->getSize().y - bPrev->getSize().y - 2));
	c->attach(bNext,
			COORDS((c->getSize().x - bNext->getSize().x) / 2,
					c->getSize().y - bNext->getSize().y - 2));
	c->attach(bQuit,
			COORDS(c->getSize().x - bQuit->getSize().x - 2,
					c->getSize().y - bQuit->getSize().y - 2));

	EventCatcher *e = new EventCatcher(c, [](GUIEvent_t *const ev) -> bool {
		// only capture encoder movements
		if(ev->type == EVENT_ENCODER_MOVED) {
			return true;
		} else {
			return false;
		}
	}, [](void *ptr, Widget *source, GUIEvent_t *ev){
		uint8_t *step = (uint8_t*) ptr;
		uint8_t pointIndex = *step / 2;
		int16_t *valueToChange = *step & 0x01 ? &points[pointIndex].high : &points[pointIndex].low;
		*valueToChange += ev->movement;
	}, &step);

	w->setMainWidget(e);
	bNext->select();

	constexpr uint8_t last_step = maxPoints * 2 - 1;

	while(!done) {
		if (new_step != step) {
			step = new_step;
			uint8_t pointIndex = step / 2;
			int16_t dbmval = step & 0x01 ? ideal_high : ideal_low;

			// update button states
			if (step == 0) {
				bPrev->setSelectable(false);
			} else {
				bPrev->setSelectable(true);
			}
			if (step >= last_step) {
				bNext->setSelectable(false);
			} else {
				bNext->setSelectable(true);
			}
			// update label text
			char buf[21];
			snprintf(buf, sizeof(buf), "Step %d/%d", step + 1, last_step + 1);
			lStep->setText(buf);
			char freq[10];
			char dbm[10];
			Unit::StringFromValue(freq, 8, points[pointIndex].freq,
					Unit::Frequency);
			Unit::StringFromValue(dbm, 8, dbmval, Unit::dbm);
			snprintf(buf, sizeof(buf), "%s, %s", freq, dbm);
			lValue->setText(buf);
		}
		// send current setting to RFboard
		Protocol::FrontToRF send;
		Protocol::RFToFront recv;
		memset(&send, 0, sizeof(send));
		memset(&recv, 0, sizeof(recv));
		send.Status.UseIntRef = 1;
		uint8_t pointIndex = step / 2;
		int16_t dbmval =
				step & 0x01 ? points[pointIndex].high : points[pointIndex].low;

		send.frequency = points[pointIndex].freq;
		send.dbm = dbmval;
		SPI1_CS_RF_GPIO_Port->BSRR = SPI1_CS_RF_Pin << 16;
		HAL_SPI_TransmitReceive(&hspi1, (uint8_t*) &send, (uint8_t*) &recv,
				sizeof(send), 1000);
		SPI1_CS_RF_GPIO_Port->BSRR = SPI1_CS_RF_Pin;
		vTaskDelay(100);
	}
	delete w;
	if(!Persistence::Save()) {
		Dialog::MessageBox("ERROR", Font_Big, "Failed to save\ndbm calibration",
				Dialog::MsgBox::OK, nullptr, false);
	}
}

int16_t Calibration::Correct(uint32_t freq, int16_t dbm) {
	uint8_t i = 0;
	for (; i < maxPoints; i++) {
		if (freq <= points[i].freq) {
			break;
		}
	}
	int16_t low, high;
	if (i == 0) {
		low = points[0].low;
		high = points[0].low;
	} else {
		// interpolate between points
		low = common_Map(freq, points[i - 1].freq, points[i].freq,
				points[i - 1].low, points[i].low);
		high = common_Map(freq, points[i - 1].freq, points[i].freq,
				points[i - 1].high, points[i].high);
	}
	// interpolate amplitude
	return common_Map(dbm, ideal_low, ideal_high, low, high);
}
