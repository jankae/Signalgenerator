#include "Generator.hpp"
#include "gui.hpp"
#include "touch.h"
#include "SPIProtocol.hpp"
#include "Calibration.hpp"
#include "Stream.hpp"
#include "Persistence.hpp"
#include "Constellation.hpp"
#include "HardwareLimits.hpp"
#include "FPGA.hpp"
#include "System.hpp"
#include <math.h>

// main carrier settings
static bool RFon = false;
static uint32_t frequency = 1000000000;
static int32_t dbm = 0;

// modulation settings
static Protocol::ModulationType modType = Protocol::ModulationType::AM;
static const char *modTypeNames[] { "AM", "FM", "FM USB", "FM LSB", "2QAM",
		"4QAM", "8QAM", "16QAM", "32QAM", "External", nullptr };
static int32_t FMDeviation = 75000;
static int32_t AMDepth = 100000000;

static uint32_t QAMSymbolrate = 1000000;
static uint8_t QAMSPS = 4;
static int32_t QAMRolloff = 350;
static Constellation QAMconst;
static bool QAMdiff = false;

static bool ExtImpedance50 = true;
static bool ExtCouplingAC = true;
static int32_t ExtMaxLevel = 3000;
static int32_t ExtMaxVoltage = 10000000;
static int32_t ExtCutoff = 5000000;
static int32_t ExtBeta = 8000;

bool updateFIR = false;

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
static Label *lModDescr1;
static Label *lModDescr2;
static Label *lModSrcDescr;

static Label *lSrcBufSoft;
static Label *lSrcBufHard;
static ProgressBar *pSrcBufSoft;
static ProgressBar *pSrcBufHard;

static MenuBool *mRFOn;
static MenuBool *mModOn;

// Changeable modulation menu entries
static Menu *mModulation;
static MenuEntry *mModSrcValue;
static MenuEntry *mModSrcFreq;
static MenuEntry *mAMDepth;
static MenuEntry *mFMDeviation;
static MenuEntry *mQAMConstellation;
static MenuEntry *mQAMSymbolrate;
static MenuEntry *mQAMSPS;
static MenuEntry *mQAMRolloff;
static MenuEntry *mQAMDiff;

static MenuEntry *mExtImpedance;
static MenuEntry *mExtCoupling;
static MenuEntry *mExtMaxVoltage;
static MenuEntry *mExtMaxLevel;
static MenuEntry *mExtCutoff;
static MenuEntry *mExtBeta;

static void ModulationChanged(void*, Widget*) {
	if (ModEnabled) {
		lModulation->setText(modTypeNames[(uint8_t) modType]);
		lModDescr1->setColor(COLOR_BLACK);
		lModDescr2->setColor(COLOR_BLACK);
		lModSrcDescr->setColor(COLOR_BLACK);
	} else {
		lModulation->setText("CW");
		lModDescr1->setColor(COLOR_LIGHTGRAY);
		lModDescr2->setColor(COLOR_LIGHTGRAY);
		lModSrcDescr->setColor(COLOR_LIGHTGRAY);
	}

	// Check if QAM modulation settings are valid
	if (QAMSPS * QAMSymbolrate > HardwareLimits::MaxFIRRate) {
		Dialog::MessageBox("WARNING", Font_Big, "Symbolrate not achievable\n"
				"with selected samples per\n"
				"symbol. Value has been\n"
				"changed accordingly.", Dialog::MsgBox::OK, nullptr, false);
		QAMSymbolrate = HardwareLimits::MaxFIRRate / QAMSPS;
	}

	lSrcBufSoft->SetVisible(false);
	pSrcBufSoft->SetVisible(false);
	lSrcBufHard->SetVisible(false);
	pSrcBufHard->SetVisible(false);

	// Remove modulation entries from submenu
	mModulation->RemoveEntry(mModSrcValue);
	mModulation->RemoveEntry(mModSrcFreq);
	mModulation->RemoveEntry(mAMDepth);
	mModulation->RemoveEntry(mFMDeviation);
	mModulation->RemoveEntry(mQAMConstellation);
	mModulation->RemoveEntry(mQAMSymbolrate);
	mModulation->RemoveEntry(mQAMSPS);
	mModulation->RemoveEntry(mQAMRolloff);
	mModulation->RemoveEntry(mQAMDiff);
	mModulation->RemoveEntry(mExtCoupling);
	mModulation->RemoveEntry(mExtImpedance);
	mModulation->RemoveEntry(mExtMaxLevel);
	mModulation->RemoveEntry(mExtMaxVoltage);
	mModulation->RemoveEntry(mExtCutoff);
	mModulation->RemoveEntry(mExtBeta);

	char descr[50] = "";
	char value[10];
	switch (modType) {
	case Protocol::ModulationType::AM:
		mModulation->AddEntry(mAMDepth, -1);
		Unit::StringFromValue(value, 7, AMDepth, Unit::Percent);
		snprintf(descr, sizeof(descr), "Max. depth %s", value);
		lModDescr1->setText("AM modulation");
		lModDescr2->setText(descr);
		break;
	case Protocol::ModulationType::FM:
	case Protocol::ModulationType::FM_LSB:
	case Protocol::ModulationType::FM_USB:
		mModulation->AddEntry(mFMDeviation, -1);
		Unit::StringFromValue(value, 9, FMDeviation, Unit::Frequency);
		snprintf(descr, sizeof(descr), "Deviation %s", value);
		if (modType == Protocol::ModulationType::FM_LSB) {
			lModDescr1->setText("FM modulation (LSB)");
		} else if (modType == Protocol::ModulationType::FM_USB) {
			lModDescr1->setText("FM modulation (USB)");
		} else {
			lModDescr1->setText("FM modulation");
		}
		lModDescr2->setText(descr);
		break;
	case Protocol::ModulationType::QAM2:
	case Protocol::ModulationType::QAM4:
	case Protocol::ModulationType::QAM8:
	case Protocol::ModulationType::QAM16:
	case Protocol::ModulationType::QAM32:
		updateFIR = true;
		QAMconst.SetUsedPoints(strtol(modTypeNames[(int) modType], nullptr, 0));
		mModulation->AddEntry(mQAMConstellation, -1);
		mModulation->AddEntry(mQAMSymbolrate, -1);
		mModulation->AddEntry(mQAMSPS, -1);
		mModulation->AddEntry(mQAMRolloff, -1);
		mModulation->AddEntry(mQAMDiff, -1);
		if (QAMdiff) {
			snprintf(descr, sizeof(descr), "%s, differential",
					modTypeNames[(int) modType]);
		} else {
			snprintf(descr, sizeof(descr), "%s modulation",
					modTypeNames[(int) modType]);
		}
		lModDescr1->setText(descr);
		Unit::StringFromValue(descr, 7, QAMSymbolrate, Unit::SampleRate);
		sprintf(&descr[7], " %02dSPS ", QAMSPS);
		Unit::StringFromValue(&descr[14], 5, QAMRolloff, Unit::Fixed3);
		lModDescr2->setText(descr);
		break;
	case Protocol::ModulationType::External:
		updateFIR = true;
		if (modSourceType != Protocol::SourceType::Disabled) {
			Dialog::MessageBox("Warning", Font_Big, "External modulation\n"
					"active. Modulation\n"
					"source has been\n"
					"disabled.", Dialog::MsgBox::OK);
			modSourceType = Protocol::SourceType::Disabled;
		}
		snprintf(descr, sizeof(descr), "External,   ,    ");
		if (ExtCouplingAC) {
			memcpy(&descr[10], "AC", 2);
		} else {
			memcpy(&descr[10], "DC", 2);
		}
		if (ExtImpedance50) {
			memcpy(&descr[14], "50R", 3);
		} else {
			memcpy(&descr[14], "1MR", 3);
		}
		lModDescr1->setText(descr);
		strcpy(descr, "Fullscale: ");
		if (ExtImpedance50) {
			Unit::StringFromValue(&descr[11], 7, ExtMaxLevel, Unit::dbm);
		} else {
			Unit::StringFromValue(&descr[11], 5, ExtMaxVoltage, Unit::Voltage);
		}
		lModDescr2->setText(descr);
		mModulation->AddEntry(mExtCoupling, -1);
		mModulation->AddEntry(mExtImpedance, -1);
		if (ExtImpedance50) {
			mModulation->AddEntry(mExtMaxLevel, -1);
			mExtMaxLevel->requestRedrawFull();
		} else {
			mModulation->AddEntry(mExtMaxVoltage, -1);
			mExtMaxVoltage->requestRedrawFull();
		}
		mModulation->AddEntry(mExtCutoff, -1);
		mModulation->AddEntry(mExtBeta, -1);
		break;
	}
	switch (modSourceType) {
	case Protocol::SourceType::Disabled:
		if (ModEnabled && modType != Protocol::ModulationType::External) {
			lModSrcDescr->setColor(COLOR_RED);
		}
		strncpy(descr, "Source disabled", sizeof(descr));
		break;
	case Protocol::SourceType::FixedValue:
		mModulation->AddEntry(mModSrcValue, 1);
		snprintf(descr, sizeof(descr), "Fixed value(%lu)", modSourceValue);
		break;
	case Protocol::SourceType::RampDown:
	case Protocol::SourceType::RampUp:
	case Protocol::SourceType::Sine:
	case Protocol::SourceType::Square:
	case Protocol::SourceType::Triangle:
		mModulation->AddEntry(mModSrcFreq, 1);
		Unit::StringFromValue(value, 9, modSourceFreq, Unit::Frequency);
		snprintf(descr, sizeof(descr), "%s@%s",
				modSrcTypeNames[(int) modSourceType], value);
		break;
	case Protocol::SourceType::PRBS:
	case Protocol::SourceType::Stream:
		mModulation->AddEntry(mModSrcFreq, 1);
		Unit::StringFromValue(value, 9, modSourceFreq, Unit::SampleRate);
		snprintf(descr, sizeof(descr), "%s@%s",
				modSrcTypeNames[(int) modSourceType], value);
		break;
	}
	lModSrcDescr->setText(descr);
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

void Generator::ToggleRF() {
	RFon = !RFon;
	mRFOn->requestRedrawFull();
}
void Generator::ToggleModulation() {
	ModEnabled = !ModEnabled;
	mModOn->requestRedrawFull();
	ModulationChanged(nullptr, nullptr);
}

void Generator::Init() {
	Stream::Init();

	Container *c = new Container(SIZE(DISPLAY_WIDTH, DISPLAY_HEIGHT));

	auto callback_setTrue = [](void *ptr, Widget*) {
		bool *flag = (bool*) ptr;
		*flag = true;
	};

	Menu *mainmenu = new Menu("", SIZE(70, DISPLAY_HEIGHT));
	mRFOn = new MenuBool("Output", &RFon, nullptr);
	mainmenu->AddEntry(mRFOn);
	mainmenu->AddEntry(
			new MenuValue<uint32_t>("Frequency", &frequency, Unit::Frequency,
					nullptr, nullptr, HardwareLimits::MinFrequency,
					HardwareLimits::MaxFrequency));
	mainmenu->AddEntry(
			new MenuValue<int32_t>("Amplitude", &dbm, Unit::dbm, nullptr,
					nullptr, HardwareLimits::MinOutputLevel,
					HardwareLimits::MaxOutputLevel));
	mModOn = new MenuBool("Modulation", &ModEnabled, ModulationChanged);
	mainmenu->AddEntry(mModOn);

	mModulation = new Menu("Configure\nModulation", mainmenu->getSize());
	mModulation->AddEntry(
			new MenuChooser("Source", modSrcTypeNames,
					(uint8_t*) &modSourceType, ModulationChanged, nullptr));
	mModulation->AddEntry(
			new MenuChooser("Type", modTypeNames, (uint8_t*) &modType,
					ModulationChanged));
	mModSrcFreq = new MenuValue<int32_t>("Src Freq", &modSourceFreq,
			Unit::Frequency, ModulationChanged, nullptr, 0,
			HardwareLimits::MaxModSrcFreq);
	mModSrcValue = new MenuValue<int32_t>("Src Value", &modSourceValue,
			Unit::None, ModulationChanged, nullptr, 0,
			HardwareLimits::MaxSrcValue);
	mAMDepth = new MenuValue<int32_t>("Depth", &AMDepth, Unit::Percent,
			ModulationChanged, nullptr, 0, Unit::maxPercent);
	mFMDeviation = new MenuValue<int32_t>("Deviation", &FMDeviation,
			Unit::Frequency, ModulationChanged, nullptr, 0,
			HardwareLimits::MaxFMDeviation);
	bool editConstellation = false;
	mQAMConstellation = new MenuAction("Edit Con-\nstellation",
			callback_setTrue, &editConstellation);
	mQAMSymbolrate = new MenuValue<uint32_t>("Symbolrate", &QAMSymbolrate,
			Unit::SampleRate, ModulationChanged, nullptr, 0,
			HardwareLimits::MaxFIRRate);
	mQAMSPS = new MenuValue<uint8_t>("Samples\nper symbol", &QAMSPS, Unit::None,
			callback_setTrue, &updateFIR, 1, (HardwareLimits::FIRTaps + 1) / 2);
	mQAMRolloff = new MenuValue<int32_t>("Excess\nbandwidth", &QAMRolloff,
			Unit::Fixed3, callback_setTrue, &updateFIR, 0, 1000);
	mQAMDiff = new MenuBool("Diff.\nencoding", &QAMdiff, ModulationChanged);

	mExtImpedance = new MenuBool("Impedance", &ExtImpedance50,
			ModulationChanged, nullptr, "1MR", "50R");
	mExtCoupling = new MenuBool("Coupling", &ExtCouplingAC, ModulationChanged,
			nullptr, "DC", "AC");
	mExtMaxLevel = new MenuValue<int32_t>("Fullscale", &ExtMaxLevel, Unit::dbm,
			ModulationChanged, nullptr, 0, 3000);
	mExtMaxVoltage = new MenuValue<int32_t>("Fullscale", &ExtMaxVoltage,
			Unit::Voltage, ModulationChanged, nullptr, 100000, 10000000);
	mExtCutoff = new MenuValue<int32_t>("Bandwidth", &ExtCutoff,
			Unit::Frequency, callback_setTrue, &updateFIR, 500000, 10000000);
	mExtBeta = new MenuValue<int32_t>("Beta\n(Kaiser)", &ExtBeta, Unit::Fixed3,
			callback_setTrue, &updateFIR, 1000, 16000);

	mModulation->AddEntry(new MenuBack());
	mainmenu->AddEntry(mModulation);

	bool IntRef = true;
	// No option to switch of internal reference anymore
//	mainmenu->AddEntry(new MenuBool("IntRef", &IntRef, nullptr));

	Menu *system = new Menu("System", mainmenu->getSize());
	mainmenu->AddEntry(system);
	system->AddEntry(new MenuAction("CalTouch", [](void*, Widget *w) {
		touch_Calibrate();
		gui_GetTopWidget()->requestRedrawFull();
	}, nullptr));

	bool calibrate_dbm = false;
	system->AddEntry(
			new MenuAction("Cal dbm", callback_setTrue, &calibrate_dbm));
	bool calibrate_balance = false;
	system->AddEntry(
			new MenuAction("Cal balance", callback_setTrue,
					&calibrate_balance));
	bool calibrate_skew = false;
	system->AddEntry(
			new MenuAction("Cal skew", callback_setTrue,
					&calibrate_skew));
	bool calibrate_reset = false;
	system->AddEntry(
			new MenuAction("Reset Cal", callback_setTrue, &calibrate_reset));
	system->AddEntry(new MenuAction("View Font", [](void *ptr, Widget *w) {
		System::ViewFont();
	}, nullptr));

	system->AddEntry(new MenuBack());

	mainmenu->select();
	c->attach(mainmenu, COORDS(DISPLAY_WIDTH - mainmenu->getSize().x, 0));

	// create and attach frequency and amplitude display
	auto sFreq = new SevenSegment<uint32_t>(&frequency, 12, 3, 11, 6,
	COLOR_BLUE, true, HardwareLimits::MinFrequency,
			HardwareLimits::MaxFrequency);
	c->attach(sFreq, COORDS(0, 5));
	c->attach(new Label("MHz", Font_Big, COLOR_BLUE), COORDS(200, 20));

	auto sdbm = new SevenSegment<int32_t>(&dbm, 12, 3, 5, 2, COLOR_BLUE, true,
			HardwareLimits::MinOutputLevel, HardwareLimits::MaxOutputLevel);
	c->attach(sdbm, COORDS(108, 45));
	c->attach(new Label("dbm", Font_Big, COLOR_BLUE), COORDS(200, 60));

	lRFon = new Label("RF ON", Font_Big, COLOR_DARKGREEN);
	lRFon->SetVisible(RFon);
	c->attach(lRFon, COORDS(30, 44));

	// create and attach modulation/source labels
	lModDescr1 = new Label(20, Font_Big, Label::Orientation::LEFT, COLOR_BLACK);
	c->attach(lModDescr1, COORDS(5, 90));
	lModDescr2 = new Label(20, Font_Big, Label::Orientation::LEFT, COLOR_BLACK);
	c->attach(lModDescr2, COORDS(5, 110));

	lModSrcDescr = new Label(20, Font_Big, Label::Orientation::LEFT,
			COLOR_BLACK);
	c->attach(lModSrcDescr, COORDS(5, 140));

	// create and attach error labels
	lUnlock = new Label("UNLOCK", Font_Big, COLOR_RED);
	lUnlock->SetVisible(false);
	c->attach(lUnlock, COORDS(5, 220));

	lUnlevel = new Label("UNLEVEL", Font_Big, COLOR_RED);
	lUnlevel->SetVisible(false);
	c->attach(lUnlevel, COORDS(100, 220));

	lCom = new Label("COMMUNICATION ERROR", Font_Big, COLOR_RED);
	lCom->SetVisible(false);
	c->attach(lCom, COORDS(5, 200));

	auto lOverload = new Label("ADC OVERLOAD", Font_Big, COLOR_RED);
	lOverload->SetVisible(false);
	c->attach(lOverload, COORDS(5, 180));

	// create and attach modulation widgets
	lModulation = new Label(8, Font_Big, Label::Orientation::CENTER,
			COLOR_DARKGREEN);
	c->attach(lModulation, COORDS(12, 61));

	QAMconst = Constellation();

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

	while (1) {
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
			FPGA::SetConstellation(QAMconst);
			editConstellation = false;
			continue;
		}
		if (updateFIR) {
			ModulationChanged(nullptr, nullptr);
			if (modType == Protocol::ModulationType::External) {
				FPGA::SetFIRLowpass(ExtCutoff, HardwareLimits::ExtSamplerate,
						(float) ExtBeta / 1000);
			} else {
				float beta = (float) QAMRolloff / 1000;
				FPGA::SetFIRRaisedCosine(QAMSPS, beta);
			}
			updateFIR = false;
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

			// balance calibration procedure changes QAM constellation
			// and FIR in FPGA, reload
			FPGA::SetConstellation(QAMconst);
			updateFIR = true;
			continue;
		}
		if (calibrate_skew) {
			Calibration::RunSkew();
			calibrate_skew = false;
			continue;
		}
		if (calibrate_reset) {
			Calibration::ResetPopup();
			calibrate_reset = false;
			continue;
		}
		Protocol::FrontToRF send;
		Protocol::RFToFront recv;
		memset(&send, 0, sizeof(send));
		memset(&recv, 0, sizeof(recv));
		send.Status.UseIntRef = IntRef ? 1 : 0;
		send.Status.ADCMax = 511;
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
			static auto lastType = Protocol::ModulationType::AM;
			mod.type = modType;
			switch (modType) {
			case Protocol::ModulationType::AM:
				mod.AM.depth = AMDepth;
				break;
			case Protocol::ModulationType::FM:
				mod.FM.deviation = FMDeviation;
				mod.FM.phase_offset = Calibration::CorrectSkew(frequency);
				break;
			case Protocol::ModulationType::FM_USB:
				mod.FM.deviation = FMDeviation;
				mod.FM.phase_offset = 49152 + Calibration::CorrectSkew(frequency);
				break;
			case Protocol::ModulationType::FM_LSB:
				mod.FM.deviation = FMDeviation;
				mod.FM.phase_offset = 16384 + Calibration::CorrectSkew(frequency);
				break;
			case Protocol::ModulationType::QAM2:
			case Protocol::ModulationType::QAM4:
			case Protocol::ModulationType::QAM8:
			case Protocol::ModulationType::QAM16:
			case Protocol::ModulationType::QAM32:
				mod.QAM.SamplesPerSymbol = QAMSPS;
				mod.QAM.SymbolsPerSecond = QAMSymbolrate;
				mod.QAM.differential = QAMdiff;
				break;
			case Protocol::ModulationType::External:
				if (lastType != Protocol::ModulationType::External) {
					updateFIR = true;
				}
				mod.External.ACCoupled = ExtCouplingAC;
				mod.External.Impedance50R = ExtImpedance50;
				if (!ExtImpedance50) {
					mod.External.maxVoltage = ExtMaxVoltage;
				} else {
					// convert from dbm to peak voltage
					float power_mW = pow(10.0f, (float) ExtMaxLevel / 1000.0f);
					float Veff = sqrt(power_mW / 1000 * 50);
					float Vp = sqrt(2) * Veff;
					mod.External.maxVoltage = Vp * 1000000;
				}
				break;
			}
			mod.source = modSourceType;
			if (modSourceType == Protocol::SourceType::FixedValue) {
				mod.Fixed.value = modSourceValue;
			} else {
				mod.Periodic.frequency = modSourceFreq;
			}
			Protocol::SetupModulation(send, mod);

			lastType = modType;
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
		if (recv.Status.AmplitudeUnlevel) {
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
		if (recv.Status.IADCOverload || recv.Status.QADCOverload) {
			lOverload->SetVisible(true);
		} else {
			lOverload->SetVisible(false);
		}
	}
}
