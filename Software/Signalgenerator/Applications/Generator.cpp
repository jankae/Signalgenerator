#include "Generator.hpp"
#include "gui.hpp"
#include "touch.h"
#include "SPIProtocol.hpp"
#include "Calibration.hpp"
#include "Stream.hpp"
#include "Persistence.hpp"
#include "Constellation.hpp"

// main carrier settings
static bool RFon = false;
static uint32_t frequency = 1000000000;
static int32_t dbm = 0;

// modulation settings
static Protocol::ModulationType modType = Protocol::ModulationType::AM;
static const char *modTypeNames[] { "AM", "FM", "FM USB", "FM LSB", "2QAM",
		"4QAM", "8QAM", "16QAM", "32QAM", nullptr };
static int32_t FMDeviation = 75000;
static int32_t AMDepth = 100000000;

static int32_t QAMbaud = 1000000;
static Constellation QAMconst;

// modulation source settings
static Protocol::SourceType modSourceType = Protocol::SourceType::Disabled;
static const char *modSrcTypeNames[] = { "Disabled", "Fixed", "Sine", "Ramp up",
		"Ramp down", "Triangle", "Square", "PRBS", "Stream", nullptr };
static int32_t modSourceFreq = 1000;
static int32_t modSourceValue = 4095;

static Label *lRFon;
static Label *lCom, *lUnlock, *lUnlevel;

extern SPI_HandleTypeDef hspi1;

static_assert(sizeof(Protocol::RFToFront) == 32);
static_assert(sizeof(Protocol::FrontToRF) == 32);

static bool ModEnabled = false;

static Label *lModulation;
static Label *lFMDeviation;
static Label *lAMDepth;
static Label *lQAMbaud;
static Entry<int32_t> *eFMDeviation;
static Entry<int32_t> *eAMDepth;
static Entry<int32_t> *eQAMbaud;
static Button *bQAMedit;

static Label *lSource;
static ItemChooser *cSource;
static Entry<int32_t> *eSrcValue;
static Entry<int32_t> *eSrcFreq;

static Label *lSrcBufSoft;
static Label *lSrcBufHard;
static ProgressBar *pSrcBufSoft;
static ProgressBar *pSrcBufHard;

static void ModulationChanged(void*, Widget*) {
	if (!ModEnabled) {
		lModulation->setText("CW");
		lFMDeviation->SetVisible(false);
		lAMDepth->SetVisible(false);
		eFMDeviation->SetVisible(false);
		eAMDepth->SetVisible(false);
		lQAMbaud->SetVisible(false);
		eQAMbaud->SetVisible(false);
		bQAMedit->SetVisible(false);
		cSource->SetVisible(false);
		lSource->SetVisible(false);
		eSrcValue->SetVisible(false);
		eSrcFreq->SetVisible(false);
		lSrcBufSoft->SetVisible(false);
		pSrcBufSoft->SetVisible(false);
		lSrcBufHard->SetVisible(false);
		pSrcBufHard->SetVisible(false);
	} else {
		lModulation->setText(modTypeNames[(uint8_t) modType]);
		cSource->SetVisible(true);
		lSource->SetVisible(true);

		lAMDepth->SetVisible(false);
		eAMDepth->SetVisible(false);
		lFMDeviation->SetVisible(false);
		eFMDeviation->SetVisible(false);
		lQAMbaud->SetVisible(false);
		eQAMbaud->SetVisible(false);
		bQAMedit->SetVisible(false);
		switch(modType) {
		case Protocol::ModulationType::AM:
			lAMDepth->SetVisible(true);
			eAMDepth->SetVisible(true);
			break;
		case Protocol::ModulationType::FM:
		case Protocol::ModulationType::FM_LSB:
		case Protocol::ModulationType::FM_USB:
			lFMDeviation->SetVisible(true);
			eFMDeviation->SetVisible(true);
			break;
		case Protocol::ModulationType::QAM2:
		case Protocol::ModulationType::QAM4:
		case Protocol::ModulationType::QAM8:
		case Protocol::ModulationType::QAM16:
		case Protocol::ModulationType::QAM32:
			lQAMbaud->SetVisible(true);
			eQAMbaud->SetVisible(true);
			bQAMedit->SetVisible(true);
			QAMconst.SetUsedPoints(
					strtol(modTypeNames[(int) modType], nullptr, 0));
			break;
		}
		switch(modSourceType) {
		case Protocol::SourceType::Disabled:
			eSrcValue->SetVisible(false);
			eSrcFreq->SetVisible(false);
			break;
		case Protocol::SourceType::FixedValue:
			eSrcValue->SetVisible(true);
			eSrcFreq->SetVisible(false);
			break;
		case Protocol::SourceType::RampDown:
		case Protocol::SourceType::RampUp:
		case Protocol::SourceType::Sine:
		case Protocol::SourceType::Square:
		case Protocol::SourceType::Triangle:
		case Protocol::SourceType::PRBS:
		case Protocol::SourceType::Stream:
			eSrcValue->SetVisible(false);
			eSrcFreq->SetVisible(true);
			break;
		}
		if (modSourceType == Protocol::SourceType::Stream) {
			lSrcBufSoft->SetVisible(true);
			pSrcBufSoft->SetVisible(true);
			lSrcBufHard->SetVisible(true);
			pSrcBufHard->SetVisible(true);
		} else {
			lSrcBufSoft->SetVisible(false);
			pSrcBufSoft->SetVisible(false);
			lSrcBufHard->SetVisible(false);
			pSrcBufHard->SetVisible(false);
		}
	}
}

void Generator::Init() {
	Stream::Init();

	Container *c = new Container(SIZE(DISPLAY_WIDTH, DISPLAY_HEIGHT));

	Menu *mainmenu = new Menu("", SIZE(70, DISPLAY_HEIGHT));
	mainmenu->AddEntry(new MenuBool("Output", &RFon, nullptr));
	mainmenu->AddEntry(new MenuValue<uint32_t>("Frequency", &frequency, Unit::Frequency));
	mainmenu->AddEntry(new MenuValue<int32_t>("Amplitude", &dbm, Unit::dbm));
	mainmenu->AddEntry(new MenuBool("Modulation", &ModEnabled, ModulationChanged));

	mainmenu->AddEntry(
			new MenuChooser("Mod Type", modTypeNames, (uint8_t*)&modType, ModulationChanged));

//	Menu *Source = new Menu("ModSrc", mainmenu->getSize());
//	Source->AddEntry(new MenuChooser("ModSrc", modSrcTypeNames, (uint8_t*) &modSourceType, nullptr, nullptr));
//	Source->AddEntry(new MenuValue("Frequency", &modSourceFreq, Unit::Frequency, nullptr, nullptr));
//	Source->AddEntry(new MenuValue("Value", &modSourceValue, Unit::None, nullptr, nullptr));
//	Source->AddEntry(new MenuBack());
//
//	mainmenu->AddEntry(Source);

	bool IntRef = true;
	mainmenu->AddEntry(new MenuBool("IntRef", &IntRef, nullptr));

	Menu *system = new Menu("System", mainmenu->getSize());
	mainmenu->AddEntry(system);
	system->AddEntry(new MenuAction("CalTouch", [](void*, Widget *w) {
		touch_Calibrate();
		gui_GetTopWidget()->requestRedrawFull();
	}, nullptr));

	bool calibrate_dbm = false;
	system->AddEntry(new MenuAction("Cal dbm", [](void* ptr, Widget *w) {
		bool *calibrate = (bool*) ptr;
		*calibrate = true;
	}, &calibrate_dbm));
	bool calibrate_balance = false;
	system->AddEntry(new MenuAction("Cal balance", [](void* ptr, Widget *w) {
		bool *calibrate = (bool*) ptr;
		*calibrate = true;
	}, &calibrate_balance));
	system->AddEntry(new MenuAction("Reset Cal", [](void *ptr, Widget *w) {
		Dialog::MessageBox("Confirm reset", Font_Big,
				"Really reset\nall calibration\nvalues?",
				Dialog::MsgBox::ABORT_OK, [](Dialog::Result res) {
					if(res == Dialog::Result::OK) {
						Persistence::Init();
						touch_Init();
						Calibration::Init();
						Persistence::Save();
					}
				}, false);
	}, nullptr));

	system->AddEntry(new MenuBack());

	mainmenu->select();
	c->attach(mainmenu, COORDS(DISPLAY_WIDTH - mainmenu->getSize().x, 0));

	// create and attach frequency and amplitude display
	auto sFreq = new SevenSegment<uint32_t>(&frequency, 12, 3, 11, 6, COLOR_BLUE, true, 5000000, 2200000000);
	c->attach(sFreq, COORDS(0,5));
	c->attach(new Label("MHz", Font_Big, COLOR_BLUE), COORDS(200, 20));

	auto sdbm = new SevenSegment<int32_t>(&dbm, 12, 3, 5, 2, COLOR_BLUE, true, -6000, 2000);
	c->attach(sdbm, COORDS(108,45));
	c->attach(new Label("dbm", Font_Big, COLOR_BLUE), COORDS(200, 60));

	lRFon = new Label("RF ON", Font_Big, COLOR_DARKGREEN);
	lRFon->SetVisible(RFon);
	c->attach(lRFon, COORDS(30, 44));

	// create and attach error labels
	lUnlock = new Label("UNLOCK", Font_Big, COLOR_RED);
	c->attach(lUnlock, COORDS(5, 220));

	lUnlevel = new Label("UNLEVEL", Font_Big, COLOR_RED);
	c->attach(lUnlevel, COORDS(100, 220));

	lCom = new Label("COMMUNICATION ERROR", Font_Big, COLOR_RED);
	c->attach(lCom, COORDS(5, 200));

	// create and attach modulation widgets
	lModulation = new Label(6, Font_Big, Label::Orientation::CENTER, COLOR_DARKGREEN);
	c->attach(lModulation, COORDS(24, 61));

	lFMDeviation = new Label("Deviation:", Font_Big);
	c->attach(lFMDeviation, COORDS(10, 90));
	// maximum deviation depends on internal FPGA bus widths
	eFMDeviation = new Entry<int32_t>(&FMDeviation, 6248378, 0, Font_Big, 8, Unit::Frequency);
	c->attach(eFMDeviation, COORDS(150, 90));
	lAMDepth = new Label("Depth:", Font_Big);
	c->attach(lAMDepth, COORDS(10, 90));
	eAMDepth = new Entry<int32_t>(&AMDepth, Unit::maxPercent, 0, Font_Big, 8, Unit::Percent);
	c->attach(eAMDepth, COORDS(150, 90));

	bool editConstellation = false;
	bQAMedit = new Button("Constellation", Font_Medium, [](void*ptr, Widget*) {
		bool *edit = (bool*) ptr;
		*edit = true;
	}, &editConstellation, COORDS(0, 22));
	c->attach(bQAMedit, COORDS(10, 89));
	lQAMbaud = new Label("Baud:", Font_Big);
	c->attach(lQAMbaud, COORDS(85, 91));
	eQAMbaud = new Entry<int32_t>(&QAMbaud, 2000000, 0, Font_Big, 8, Unit::SampleRate);
	c->attach(eQAMbaud, COORDS(150, 90));

	QAMconst = Constellation();

	// create and attach modulation source widgets
	lSource = new Label("Modulation source:", Font_Big);
	c->attach(lSource, COORDS(10, 115));
	cSource = new ItemChooser(modSrcTypeNames, (uint8_t*) &modSourceType, Font_Big, 1, 80);
	cSource->setCallback(ModulationChanged, nullptr);
	c->attach(cSource, COORDS(10, 130));
	eSrcValue = new Entry<int32_t>(&modSourceValue, 4095, 0, Font_Big, 8, Unit::None);
	c->attach(eSrcValue, COORDS(150, 130));
	eSrcFreq = new Entry<int32_t>(&modSourceFreq, 48000, 0, Font_Big, 8, Unit::Frequency);
	c->attach(eSrcFreq, COORDS(150, 130));

	// Buffer indicators for streaming source
	lSrcBufSoft = new Label("Software buffer:", Font_Medium);
	c->attach(lSrcBufSoft, COORDS(10, 155));
	pSrcBufSoft = new ProgressBar(COORDS(110, 20));
	c->attach(pSrcBufSoft, COORDS(10, 165));

	lSrcBufHard = new Label("Hardware buffer:", Font_Medium);
	c->attach(lSrcBufHard, COORDS(130, 155));
	pSrcBufHard = new ProgressBar(COORDS(110, 20));
	c->attach(pSrcBufHard, COORDS(130, 165));

	// TODO
	pSrcBufSoft->setState(100);

	ModulationChanged(nullptr, nullptr);

	c->requestRedrawFull();
	gui_SetTopWidget(c);

	vTaskDelay(1000);

//	auto cst = Constellation(Constellation::Type::BPSK);
//	cst.Edit();

	while(1) {
		uint32_t start = HAL_GetTick();
		constexpr uint32_t delay = 100;
		if (ModEnabled && modSourceType == Protocol::SourceType::Stream) {
			uint8_t FPGAfree = Stream::LoadToFPGA(delay);
			uint8_t fillState = (255 - FPGAfree) * 100 / 255;
			pSrcBufHard->setState(fillState);
		}
		uint32_t now = HAL_GetTick();
		if (now - start < delay) {
			vTaskDelay(delay - (now - start));
		}

		if (editConstellation) {
			QAMconst.Edit();
			QAMconst.LoadToFPGA();
			// TODO move FIR tap generation to proper place
			Constellation::SetFIRinFPGA(4, 0.5);
			editConstellation = false;
			continue;
		}
		if (calibrate_dbm) {
			Calibration::RunAmplitude();
			calibrate_dbm = false;
			continue;
		}
		if (calibrate_balance) {
			Calibration::RunBalance();
			calibrate_balance = false;
			continue;
		}
		Protocol::FrontToRF send;
		Protocol::RFToFront recv;
		memset(&send, 0, sizeof(send));
		memset(&recv, 0, sizeof(recv));
		send.Status.UseIntRef = IntRef ? 1 : 0;
		if (RFon) {
			send.frequency = frequency;
			send.dbm = Calibration::CorrectAmplitude(frequency, dbm);
		} else {
			send.frequency = 0;
			send.dbm = 0;
		}
		auto balance = Calibration::CorrectBalance(frequency);
		send.offset_I = balance.I;
		send.offset_Q = balance.Q;
		if (ModEnabled) {
			Protocol::Modulation mod;
			mod.type = modType;
			switch(modType) {
			case Protocol::ModulationType::AM:
				mod.AM.depth = AMDepth;
				break;
			case Protocol::ModulationType::FM:
			case Protocol::ModulationType::FM_USB:
			case Protocol::ModulationType::FM_LSB:
				mod.FM.deviation = FMDeviation;
				break;
			case Protocol::ModulationType::QAM2:
			case Protocol::ModulationType::QAM4:
			case Protocol::ModulationType::QAM8:
			case Protocol::ModulationType::QAM16:
			case Protocol::ModulationType::QAM32:
				// TODO replace dummy values
				mod.QAM.SamplesPerSymbol = 4;
				mod.QAM.SymbolsPerSecond = 1000;
				break;
			}
			mod.source = modSourceType;
			if(modSourceType == Protocol::SourceType::FixedValue) {
				mod.Fixed.value = modSourceValue;
			} else {
				mod.Periodic.frequency = modSourceFreq;
			}
			Protocol::SetupModulation(send, mod);
		}
		SPI1_CS_RF_GPIO_Port->BSRR = SPI1_CS_RF_Pin << 16;
		HAL_SPI_TransmitReceive(&hspi1, (uint8_t*) &send, (uint8_t*) &recv,
				sizeof(send), 1000);
		SPI1_CS_RF_GPIO_Port->BSRR = SPI1_CS_RF_Pin;
		// evaluate received data
		if (recv.MagicConstant == Protocol::MagicConstant) {
			lCom->SetVisible(false);
		} else {
			lCom->SetVisible(true);
		}
		if (recv.Status.MainPLLON && recv.Status.IQModEnabled) {
			lRFon->SetVisible(true);
		} else {
			lRFon->SetVisible(false);
		}
		if(recv.Status.AmplitudeUnlevel) {
			lUnlevel->SetVisible(true);
		} else {
			lUnlevel->SetVisible(false);
		}
		if ((recv.Status.MainPLLUnlocked && recv.Status.MainPLLON)
				|| (recv.Status.HeterodynePLLUnlock
						&& recv.Status.HeterodynePLLON)) {
			lUnlock->SetVisible(true);
		} else {
			lUnlock->SetVisible(false);
		}
	}
}
