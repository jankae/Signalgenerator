#include "app.h"

#include "pushpull.h"
#include "desktop.h"

/**
 * @brief Registers a new app with the desktop application
 * @param name Name of the application
 * @param start Pointer to a function creating the application task
 * @param icon Icon of the application
 */
void App_Register(const char *name, const char *descr, void (*start)(void), Image_t icon) {
	/* register app at desktop module */
	AppInfo_t app;
	app.name = name;
	app.descr = descr;
	app.start = start;
	app.icon = icon;
	desktop_AddApp(app);
}

/**
 * @brief Handles common application signals, passes special signals on to
 * the application
 * @param signal Will contain the signal code in case of special signals
 * @param wait Maximum time to wait for signal in ms
 * @return 1 if signal received, 0 otherwise
 */
uint8_t App_Handler(uint32_t *signal, uint32_t wait) {
	if (xTaskNotifyWait(0, ULONG_MAX, signal, pdMS_TO_TICKS(wait))) {
		/* received a notification */
		if(*signal & SIGNAL_TERMINATE) {
			/* Stop control of pushpull stage (will only have an effect if control had been acquired */
			pushpull_ReleaseControl();
			/* Notify desktop of stopped app (will delete app widgets) */
			desktop_AppStopped();
			/* Remove this apps task */
			vTaskDelete(NULL);
			while (1)
				;
		} else {
			/* Pass on signal to user task */
			return 1;
		}
	} else {
		/* Timeout expired, no signal received */
		return 0;
	}
}
