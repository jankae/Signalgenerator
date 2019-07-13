#include <gui.hpp>
#include "buttons.h"

QueueHandle_t GUIeventQueue = NULL;
Widget *topWidget;
bool isPopup;

static Widget *newTopWidget = nullptr;

TaskHandle_t GUIHandle;

#include "menu.hpp"
#include "MenuBool.hpp"
#include "MenuBack.hpp"
#include "MenuChooser.hpp"
#include "MenuValue.hpp"
#include "Dialog/ItemChooserDialog.hpp"

static void guiThread(void) {
	GUIHandle = xTaskGetCurrentTaskHandle();

	topWidget = NULL;
	GUIEvent_t event;

	while (1) {
		if (xQueueReceive(GUIeventQueue, &event, 20)) {
			if (topWidget) {
				switch (event.type) {
				case EVENT_TOUCH_PRESSED:
				case EVENT_TOUCH_RELEASED:
				case EVENT_TOUCH_HELD:
					/* these are position based events */
					/* check if applicable for top widget
					 * (which, for smaller windows, might not be the case */
					if (topWidget->isInArea(event.pos)) {
						/* send event to top widget */
						Widget::input(topWidget, &event);
					} else if (!isPopup) {
						/* only pass on events to desktop if no popup is present */
//						desktop_Input(&event);
					}
					break;
				case EVENT_BUTTON_CLICKED:
					if(event.button == BUTTON_ONOFF) {
						/* this is a special case button that is always relayed to the
						 * App in control of the output stage */
//						if (pushpull_GetControlHandle()) {
//							xTaskNotify(pushpull_GetControlHandle(),
//									SIGNAL_ONOFF_BUTTON,
//									eSetBits);
//						}
						break;
					}
					/* no break */
				case EVENT_ENCODER_MOVED:
					/* these events are always valid for the selected widget */
					if (Widget::getSelected()) {
						Widget::input(Widget::getSelected(), &event);
					} else {
//						desktop_Input(&event);
					}
					break;
				case EVENT_WINDOW_CLOSE:
					break;
				default:
					break;
				}
			}
		}
		if (newTopWidget && !isPopup) {
			topWidget = newTopWidget;
			newTopWidget = nullptr;
		}
		if (topWidget) {
			Widget::draw(topWidget, COORDS(0, 0));
		}
	}
}

uint8_t gui_Init(void) {
	/* initialize event queue */
	GUIeventQueue = xQueueCreate(10, sizeof(GUIEvent_t));

	if(!GUIeventQueue) {
		return 0;
	}

	/* create GUI thread */
	if(xTaskCreate((TaskFunction_t )guiThread, "GUI", 300, NULL, 3, NULL)!=pdPASS) {
		return 0;
	}
	return 1;
}

void gui_SendEvent(GUIEvent_t *ev) {
	if(!GUIeventQueue || !ev) {
		/* some pointer error */
		return;
	}
	BaseType_t yield;
	xQueueSendFromISR(GUIeventQueue, ev, &yield);
//	portYIELD_FROM_ISR(yield);
}

void gui_SetTopWidget(Widget* w) {
	newTopWidget = w;
}

Widget* gui_GetTopWidget() {
	return topWidget;
}
