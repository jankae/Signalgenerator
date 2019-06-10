#ifndef GUI_EVENTS_H_
#define GUI_EVENTS_H_

#include "common.h"
#include "widget.h"

typedef enum  {GUI_OK, GUI_ERROR, GUI_UNABLE} GUIResult_t;

typedef struct event GUIEvent_t;

typedef enum {
	EVENT_NONE,
	EVENT_TOUCH_PRESSED,
	EVENT_TOUCH_RELEASED,
	EVENT_TOUCH_DRAGGED,
	EVENT_TOUCH_HELD,
	EVENT_WINDOW_CLOSE,
	EVENT_BUTTON_CLICKED,
	EVENT_ENCODER_MOVED,
	EVENT_WIDGET_DELETE,
} GUIEventType_t;

struct event {
	/* indicates the type of event */
	GUIEventType_t type;
	/* data corresponding to the event type */
	union {
		/* coordinates for position based events */
		coords_t pos;
		/* pointer to widget for widget based events */
		class Widget *w;
		/* Button mask for button related events */
		uint32_t button;
		/* Encoder movement */
		int32_t movement;
	};
};

#endif
