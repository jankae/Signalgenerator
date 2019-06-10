#include "menu.hpp"


Menu::Menu(coords_t size) {
	this->size = size;
	selectable = true;
	nentries = 0;
	firstEntry = nullptr;
	selectedEntry = 0;
	usePages = false;
	entriesPerPage = size.y / EntrySizeY;
	redrawChild = true;
}

bool Menu::AddEntry(MenuEntry* e) {
	if (firstEntry) {
		/* find end of entry list */
		Widget *entry = firstEntry;
		do {
			if (entry == e) {
				/* this widget has already been added, this must never happen */
				CRIT_ERROR("Duplicate entry in menu");
				return false;
			}
			if (entry->next) {
				entry = entry->next;
			} else {
				break;
			}
		} while (1);
		/* add widget to the end */
		entry->next = e;
	} else {
		/* this is the first child */
		firstEntry = e;
	}
	e->parent = this;
	e->size.x = EntrySizeX - 2;
	e->size.y = EntrySizeY - 4;
	e->setPosition(COORDS(size.x - EntrySizeX + 2, 2));
	nentries++;
	entriesPerPage = size.y / EntrySizeY;
	if (nentries > entriesPerPage) {
		usePages = true;
		entriesPerPage--;
	}
	return true;
}

bool Menu::AddWidget(Widget* w) {
	if(firstChild) {
		/* window already has a widget in it */
		CRIT_ERROR("Window already has a widget");
	}
	coords_t maxSize = getAvailableArea();
	if(w->getSize().x > maxSize.x || w->getSize().y > maxSize.y) {
		/* widget doesn't fit in window */
		/* This potentially allows for a memory leak if the application doesn't check for
		 * a return code. The widget (which failed to be attached to the window) won't get freed
		 * when the window is closed.
		 * Workaround: As an empty window is not useful at all, this will only happen during
		 * a software error -> display error message */
		CRIT_ERROR("Widget too big for window");
	}
	firstChild = w;
	redrawChild = 1;
	w->parent = this;
	/* set child offset */
	w->setPosition(COORDS(0, 0));
	return true;
}

void Menu::draw(coords_t offset) {
	if (!nentries) {
		CRIT_ERROR("Menu needs at least one entry");
	}
	/* calculate corners */
	coords_t upperLeft = offset;
	coords_t lowerRight = upperLeft;
	lowerRight.x += size.x - 1;
	lowerRight.y += size.y - 1;
	// trigger possible child redraw preventively here to allow menu entries to draw over menu screen
	if (redrawChild) {
		this->drawChildren(offset);
		redrawChild = false;
	}
	// get offset of first entry on page
	uint8_t pageOffset = (uint8_t) (selectedEntry / entriesPerPage)
			* entriesPerPage;
//	MenuEntry *entry = static_cast<MenuEntry*>(firstEntry->GetNth(pageOffset));
	for (uint8_t i = 0; i < entriesPerPage; i++) {
		if (i + pageOffset >= nentries) {
			break;
		}
		if (i + pageOffset == selectedEntry) {
			display_SetForeground(Selected);
		} else {
			display_SetForeground(Foreground);
		}
		display_HorizontalLine(lowerRight.x - EntrySizeX + 1,
				upperLeft.y + i * EntrySizeY, EntrySizeX);
		display_HorizontalLine(lowerRight.x - EntrySizeX + 1,
				upperLeft.y + i * EntrySizeY + 1, EntrySizeX);
		display_HorizontalLine(lowerRight.x - EntrySizeX + 1,
				upperLeft.y + (i + 1) * EntrySizeY - 1, EntrySizeX);
		display_HorizontalLine(lowerRight.x - EntrySizeX + 1,
				upperLeft.y + (i + 1) * EntrySizeY - 2, EntrySizeX);
		display_VerticalLine(lowerRight.x - EntrySizeX,
				upperLeft.y + i * EntrySizeY + 1, EntrySizeY - 2);
		display_VerticalLine(lowerRight.x - EntrySizeX + 1,
				upperLeft.y + i * EntrySizeY + 1, EntrySizeY - 2);
//		entry->draw(
//				COORDS(offset.x + entry->position.x + 0,
//						offset.y + entry->position.y + i * EntrySizeY));
//		entry = static_cast<MenuEntry*>(entry->next);
	}
	if (usePages) {
		// draw page switcher
		display_SetFont(Font_Big);
		display_SetForeground(Foreground);
		display_SetBackground(Background);
		display_HorizontalLine(lowerRight.x - EntrySizeX + 1,
				upperLeft.y + entriesPerPage * EntrySizeY, EntrySizeX);
		display_HorizontalLine(lowerRight.x - EntrySizeX + 1,
				upperLeft.y + entriesPerPage * EntrySizeY + 1, EntrySizeX);
		display_HorizontalLine(lowerRight.x - EntrySizeX + 1,
				upperLeft.y + (entriesPerPage + 1) * EntrySizeY - 1,
				EntrySizeX);
		display_HorizontalLine(lowerRight.x - EntrySizeX + 1,
				upperLeft.y + (entriesPerPage + 1) * EntrySizeY - 2,
				EntrySizeX);
		display_VerticalLine(lowerRight.x - EntrySizeX,
				upperLeft.y + entriesPerPage * EntrySizeY + 1, EntrySizeY - 2);
		display_VerticalLine(lowerRight.x - EntrySizeX + 1,
				upperLeft.y + entriesPerPage * EntrySizeY + 1, EntrySizeY - 2);
		char page[4];
		page[0] = selectedEntry / entriesPerPage + 1 + '0';
		page[1] = '/';
		page[2] = (nentries - 1) / entriesPerPage + 1 + '0';
		page[3] = 0;
		uint16_t shiftX = (EntrySizeX - strlen(page) * Font_Big.width) / 2;
		uint16_t shiftY = (EntrySizeY - Font_Big.height) / 2;
		display_String(lowerRight.x - EntrySizeX + shiftX,
				upperLeft.y + entriesPerPage * EntrySizeY + shiftY, page);
	}
}

void Menu::drawChildren(coords_t offset) {
	if (firstChild) {
		Widget::draw(firstChild, offset);
	}
	// get offset of first entry on page
	uint8_t pageOffset = (uint8_t) (selectedEntry / entriesPerPage)
			* entriesPerPage;
	MenuEntry *entry = static_cast<MenuEntry*>(firstEntry->GetNth(pageOffset));
	for (uint8_t i = 0; i < entriesPerPage; i++) {
		if (i + pageOffset >= nentries) {
			break;
		}
		Widget::draw(entry, COORDS(offset.x, offset.y + i * EntrySizeY));
		entry = static_cast<MenuEntry*>(entry->next);
	}
}

coords_t Menu::getAvailableArea() {
	return SIZE(size.x - EntrySizeX, size.y);
}

void Menu::input(GUIEvent_t* ev) {
	switch (ev->type) {
	case EVENT_ENCODER_MOVED: {
		uint8_t old_page = selectedEntry / entriesPerPage;
		if (ev->movement > 0) {
			if (selectedEntry < nentries - 1) {
				selectedEntry++;
			} else {
				selectedEntry = 0;
			}
		} else {
			if (selectedEntry > 0) {
				selectedEntry--;
			} else {
				selectedEntry = nentries - 1;
			}
		}
		uint8_t new_page = selectedEntry / entriesPerPage;
		if (new_page == old_page) {
			this->requestRedraw();
		} else {
			PageSwitched();
		}

		ev->type = EVENT_NONE;
	}
		break;
	case EVENT_TOUCH_PRESSED: {
		// select new item
		uint8_t itemOnPage = ev->pos.y / EntrySizeY;
		if (usePages && itemOnPage >= entriesPerPage) {
			// the "next-page" area was touched, select first entry on next page
			uint8_t old_page = selectedEntry / entriesPerPage;
			selectedEntry = (old_page + 1) * entriesPerPage;
			if (selectedEntry >= nentries) {
				selectedEntry = 0;
			}
			PageSwitched();
			ev->type = EVENT_NONE;
		} else {
			// get offset of first entry on page
			uint8_t old_selected = selectedEntry;
			uint8_t pageOffset = (uint8_t) (selectedEntry / entriesPerPage)
					* entriesPerPage;
			selectedEntry = ev->pos.y / EntrySizeY + pageOffset;
			if (old_selected != selectedEntry) {
				this->requestRedraw();
			}
			// do not reset ev->type as the touch event will be passed on to the selected entry
		}
	}
		break;
	}
	if (ev->type != EVENT_NONE) {
		// pass on to selected entry
		if (ev->type == EVENT_TOUCH_DRAGGED || ev->type == EVENT_TOUCH_PRESSED
				|| ev->type == EVENT_TOUCH_HELD
				|| ev->type == EVENT_TOUCH_RELEASED) {
			// position based events, update event position
			ev->pos.x -= size.x - EntrySizeX + 2;
			ev->pos.y -= 2 + (selectedEntry % entriesPerPage) * EntrySizeY;
			;
		}
		Widget::input(firstEntry->GetNth(selectedEntry), ev);
	}
}

void Menu::PageSwitched() {
	this->requestRedrawFull();
	uint8_t pageOffset = (uint8_t) (selectedEntry / entriesPerPage)
			* entriesPerPage;
	MenuEntry *entry = static_cast<MenuEntry*>(firstEntry->GetNth(pageOffset));
	for (uint8_t i = 0; i < entriesPerPage; i++) {
		if (i + pageOffset >= nentries) {
			break;
		}
		entry->requestRedraw();
		entry = static_cast<MenuEntry*>(entry->next);
	}

}
