#ifndef APP_H_
#define APP_H_

#include <stdint.h>
#include <limits.h>
#include "FreeRTOS.h"
#include "widget.h"

/* Stop this app */
#define SIGNAL_TERMINATE		0x80000000
/* No specific action, just trigger the app task */
#define SIGNAL_WAKEUP	0x40000000

#define SIGNAL_ONOFF_BUTTON		0x00000001
#define SIGNAL_PUSHPULL_UPDATE	0x00000002

#define APP_MAX_NAMELENGTH		20


typedef enum {
	APP_STOPPED, APP_STARTSEND, APP_RUNNING, APP_KILLSEND
} AppState_t;

typedef struct {
	const char *name;
	const char *descr;
	TaskHandle_t handle;
	void (*start)(void);
	Widget *topWidget;
	AppState_t state;
	Image_t icon;
} AppInfo_t;

/**
 * @brief Registers a new app with the desktop application
 * @param name Name of the application
 * @param start Pointer to a function creating the application task
 * @param icon Icon of the application
 */
void App_Register(const char *name, const char *descr, void (*start)(void), Image_t icon);

/**
 * @brief Handles common application signals, passes special signals on to
 * the application
 * @param signal Will contain the signal code in case of special signals
 * @param wait Maximum time to wait for signal in ms
 * @return 1 if signal received, 0 otherwise
 */
uint8_t App_Handler(uint32_t *signal, uint32_t wait);

#endif
