#include "Calibration.hpp"

#include "common.hpp"
#include "gui.hpp"
#include "SPIProtocol.hpp"
#include "Persistence.hpp"

extern SPI_HandleTypeDef hspi1;

static constexpr int16_t ideal_low = -2000;
static constexpr int16_t ideal_high = 0;
using AmplitudePoint = struct {
	uint32_t freq;
	int16_t low;
	int16_t high;
};

using BalancePoint = struct {
	uint32_t freq;
	int16_t I;
	int16_t Q;
};

static constexpr uint8_t maxPoints = 10;
static AmplitudePoint AmplitudePoints[maxPoints];

static BalancePoint BalancePoints[maxPoints];

static constexpr AmplitudePoint defaultAmplitudeCalibration[maxPoints] = {
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

static constexpr BalancePoint defaultBalanceCalibration[maxPoints] = {
		{10000000, 0, 0},
		{100000000, 0, 0},
		{200000000, 0, 0},
		{500000000, 0, 0},
		{750000000, 0, 0},
		{1000000000, 0, 0},
		{1250000000, 0, 0},
		{1500000000, 0, 0},
		{1750000000, 0, 0},
		{2000000000, 0, 0},
};

void Calibration::Init() {
	memcpy(AmplitudePoints, defaultAmplitudeCalibration, sizeof(AmplitudePoints));
	memcpy(BalancePoints, defaultBalanceCalibration, sizeof(BalancePoints));
	Persistence::Add(AmplitudePoints, sizeof(AmplitudePoints));
	Persistence::Add(BalancePoints, sizeof(BalancePoints));
}

void Calibration::RunAmplitude() {
	uint8_t step = 1;
	uint8_t new_step = 0;
	bool done = false;

	Window *w = new Window("dbm Calibration", Font_Big, COORDS(250, 150));
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
		int16_t *valueToChange = *step & 0x01 ? &AmplitudePoints[pointIndex].high : &AmplitudePoints[pointIndex].low;
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
			Unit::StringFromValue(freq, 8, AmplitudePoints[pointIndex].freq,
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
				step & 0x01 ? AmplitudePoints[pointIndex].high : AmplitudePoints[pointIndex].low;

		send.frequency = AmplitudePoints[pointIndex].freq;
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

int16_t Calibration::CorrectAmplitude(uint32_t freq, int16_t dbm) {
	uint8_t i = 0;
	for (; i < maxPoints; i++) {
		if (freq <= AmplitudePoints[i].freq) {
			break;
		}
	}
	int16_t low, high;
	if (i == 0) {
		low = AmplitudePoints[0].low;
		high = AmplitudePoints[0].high;
	} else {
		// interpolate between AmplitudePoints
		low = common_Map(freq, AmplitudePoints[i - 1].freq, AmplitudePoints[i].freq,
				AmplitudePoints[i - 1].low, AmplitudePoints[i].low);
		high = common_Map(freq, AmplitudePoints[i - 1].freq, AmplitudePoints[i].freq,
				AmplitudePoints[i - 1].high, AmplitudePoints[i].high);
	}
	// interpolate amplitude
	return common_Map(dbm, ideal_low, ideal_high, low, high);
}

void Calibration::RunBalance() {
	uint8_t step = 1;
	uint8_t new_step = 0;
	bool done = false;

	Window *w = new Window("Offset Calibration", Font_Big, COORDS(250, 150));
	Container *c = new Container(w->getAvailableArea());
	Label *lStep = new Label(20, Font_Big, Label::Orientation::CENTER);
	Label *lUsage = new Label("Adjust knob to suppress carrier", Font_Medium);
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
		int16_t *valueToChange = *step & 0x01 ? &BalancePoints[pointIndex].Q : &BalancePoints[pointIndex].I;
		*valueToChange += ev->movement * 16;
	}, &step);

	w->setMainWidget(e);
	bNext->select();

	constexpr uint8_t last_step = maxPoints * 2 - 1;

	while(!done) {
		if (new_step != step) {
			step = new_step;
			uint8_t pointIndex = step / 2;

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
			Unit::StringFromValue(freq, 8, BalancePoints[pointIndex].freq,
					Unit::Frequency);
			char *offset = step & 0x01 ? "Q Offset" : "I Offset";
			snprintf(buf, sizeof(buf), "%s, %s", freq, offset);
			lValue->setText(buf);
		}
		// send current setting to RFboard
		Protocol::FrontToRF send;
		Protocol::RFToFront recv;
		memset(&send, 0, sizeof(send));
		memset(&recv, 0, sizeof(recv));
		send.Status.UseIntRef = 1;
		uint8_t pointIndex = step / 2;
		send.frequency = BalancePoints[pointIndex].freq;
		send.dbm = CorrectAmplitude(BalancePoints[pointIndex].freq, 0);

		send.offset_I = BalancePoints[pointIndex].I;
		send.offset_Q = BalancePoints[pointIndex].Q;
		// FM modulation with constant modulation source
		send.modulation_registers[0] = 4095; // fixed value
		send.modulation_registers[1] = UINT16_MAX; // maximum frequency deviation
		send.modulation_registers[3] |= 0x04; // FM modulation
		send.modulation_registers[3] |= (((uint8_t) 0x01) << 8); // fixed source
		SPI1_CS_RF_GPIO_Port->BSRR = SPI1_CS_RF_Pin << 16;
		HAL_SPI_TransmitReceive(&hspi1, (uint8_t*) &send, (uint8_t*) &recv,
				sizeof(send), 1000);
		SPI1_CS_RF_GPIO_Port->BSRR = SPI1_CS_RF_Pin;
		vTaskDelay(100);
	}
	delete w;
	if(!Persistence::Save()) {
		Dialog::MessageBox("ERROR", Font_Big, "Failed to save\balance calibration",
				Dialog::MsgBox::OK, nullptr, false);
	}
}

Calibration::IQOffset Calibration::CorrectBalance(uint32_t freq) {
	uint8_t i = 0;
	for (; i < maxPoints; i++) {
		if (freq <= BalancePoints[i].freq) {
			break;
		}
	}
	IQOffset ret;
	if (i == 0) {
		ret.I = BalancePoints[0].I;
		ret.Q = BalancePoints[0].Q;
	} else {
		// interpolate between AmplitudePoints
		ret.I = common_Map(freq, BalancePoints[i - 1].freq, BalancePoints[i].freq,
				BalancePoints[i - 1].I, BalancePoints[i].I);
		ret.Q = common_Map(freq, BalancePoints[i - 1].freq, BalancePoints[i].freq,
				BalancePoints[i - 1].Q, BalancePoints[i].Q);
	}

	return ret;
}
