#ifndef GUI_GUI_H_
#define GUI_GUI_H_

#include <button.hpp>
#include <checkbox.hpp>
#include <container.hpp>
#include <custom.hpp>
#include <dialog.hpp>
#include <entry.hpp>
#include <graph.hpp>
#include <itemChooser.hpp>
#include <keyboard.hpp>
#include <label.hpp>
#include <progressbar.hpp>
#include <sevensegment.hpp>
#include <textfield.hpp>
#include <window.hpp>
#include "menu.hpp"
#include "MenuBack.hpp"
#include "MenuBool.hpp"
#include "MenuChooser.hpp"
#include "MenuValue.hpp"
#include "MenuAction.hpp"
#include "FreeRTOS.h"
#include "queue.h"

uint8_t gui_Init(void);

void gui_SendEvent(GUIEvent_t *ev);

void gui_SetTopWidget(Widget *w);
Widget* gui_GetTopWidget();

#endif
