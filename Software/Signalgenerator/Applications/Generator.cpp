#include "Generator.hpp"
#include "gui.hpp"
#include "touch.h"

bool RFon = false;
int32_t frequency = 1000000000;
int32_t dbm = 0;

static Label *lRFon;
static Label *lCom, *lUnlock, *lUnlevel;

static void RFonCallback() {
	lRFon->SetVisible(RFon);
}

void Generator::Init() {
	Container *c = new Container(SIZE(DISPLAY_WIDTH, DISPLAY_HEIGHT));

	Menu *mainmenu = new Menu("", SIZE(70, DISPLAY_HEIGHT));
	mainmenu->AddEntry(new MenuBool("Output", &RFon, [](Widget&) {
		RFonCallback();
	}));
	mainmenu->AddEntry(new MenuValue("Frequency", &frequency, Unit::Frequency));
	mainmenu->AddEntry(new MenuValue("Amplitude", &dbm, Unit::dbm));

	Menu *system = new Menu("System", mainmenu->getSize());
	mainmenu->AddEntry(system);
	system->AddEntry(new MenuAction("CalTouch", [](void*, Widget *w) {
		touch_Calibrate();
		gui_GetTopWidget()->requestRedrawFull();
	}, nullptr));
	system->AddEntry(new MenuBack());

	mainmenu->select();
	c->attach(mainmenu, COORDS(DISPLAY_WIDTH - mainmenu->getSize().x, 0));

	auto sFreq = new SevenSegment(&frequency, 12, 3, 11, 6, COLOR_BLUE);
	c->attach(sFreq, COORDS(0,5));
	c->attach(new Label("MHz", Font_Big, COLOR_BLUE), COORDS(200, 20));

	auto sdbm = new SevenSegment(&dbm, 12, 3, 5, 2, COLOR_BLUE);
	c->attach(sdbm, COORDS(108,45));
	c->attach(new Label("dbm", Font_Big, COLOR_BLUE), COORDS(200, 60));

	lRFon = new Label("RF ON", Font_Big, COLOR_DARKGREEN);
	lRFon->SetVisible(RFon);
	c->attach(lRFon, COORDS(30, 50));

	lUnlock = new Label("UNLOCK", Font_Big, COLOR_RED);
	c->attach(lUnlock, COORDS(5, 220));

	lUnlevel = new Label("UNLEVEL", Font_Big, COLOR_RED);
	c->attach(lUnlevel, COORDS(100, 220));

	lCom = new Label("COMMUNICATION ERROR", Font_Big, COLOR_RED);
	c->attach(lCom, COORDS(5, 200));

	c->requestRedrawFull();
	gui_SetTopWidget(c);
}
