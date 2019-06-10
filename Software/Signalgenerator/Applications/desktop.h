#ifndef DESKTOP_H_
#define DESKTOP_H_

#include "gui.h"
#include "task.h"
#include "app.h"

#define DESKTOP_MAX_APPS		6

#define DESKTOP_ICONBAR_WIDTH	40
#define DESKTOP_ICONSPACING_Y	40
#define DESKTOP_ICONOFFSET_Y	4
#define DESKTOP_ICONOFFSET_X	4

void desktop_AddApp(AppInfo_t app);
void desktop_AppStarted(void (*start)(void), Widget *top);
void desktop_AppStopped(void);
void desktop_Draw(void);
void desktop_Input(GUIEvent_t *ev);

#endif
