#pragma once

#include "widget.hpp"
#include "Unit.hpp"
#include "display.h"
#include "font.h"
#include "buttons.h"
#include "Dialog/ValueInput.hpp"

template<typename T>
class Table : public Widget {
public:
	Table(font_t font, uint8_t visibleLines) {
		columns = nullptr;
		rows = 0;
		cols = 0;
		size.x = 2 + ScrollbarSize;
		size.y = (font.height + PaddingRows) * (visibleLines + 1) + PaddingRows;
		this->font = font;
		selected_row = 0;
		selected_column = 0;
		lines = visibleLines;
		topVisibleEntry = 0;
	}
	~Table() {
		while (columns) {
			Column *next = columns->next;
			delete columns;
			columns = next;
		}
	}

	bool AddColumn(const char *name, T *data, uint8_t len, const Unit::unit *u[],
			uint8_t digits, bool editable) {
		if (rows > 0) {
			if (len != rows) {
				// other length than already added column, aborting
				return false;
			}
		}
		auto *column = new Column;
		// copy members
		strncpy(column->name, name, MaxNameLength);
		column->name[MaxNameLength - 1] = 0;
		column->unit = u;
		column->digits = digits;
		column->editable = editable;
		column->data = data;
		column->next = nullptr;
		if (digits > strlen(name)) {
			column->sizex = font.width * digits + PaddingColumns;
		} else {
			column->sizex = font.width * strlen(name) + PaddingColumns;
		}
		// add to end of linked list
		if (!columns) {
			columns = column;
			rows = len;
		} else {
			auto it = columns;
			while (it->next) {
				it = it->next;
			}
			it->next = column;
		}

		// update overall table size
		size.x += column->sizex;
		cols++;
		return true;
	}
private:
	static constexpr uint8_t MaxNameLength = 10;
	static constexpr uint8_t PaddingRows = 2;
	static constexpr uint8_t PaddingColumns = 2;
	static constexpr uint8_t ScrollbarSize = 20;
	static constexpr color_t ScrollbarColor = COLOR_ORANGE;

	Widget::Type getType() override { return Widget::Type::Table; };

	void draw(coords_t offset) override {
		display_SetForeground(COLOR_BLACK);
		display_SetBackground(COLOR_WHITE);
		display_HorizontalLine(offset.x, offset.y, size.x);
		display_VerticalLine(offset.x, offset.y, size.y);
		// draw columns
		uint16_t offsetX = 1;
		Column *c = columns;
		uint8_t column_cnt = 0;
		while (c) {
			uint16_t offsetY = 1;
			// draw name
			display_AutoCenterString(c->name,
					COORDS(offset.x + offsetX, offset.y + offsetY),
					COORDS(offset.x + offsetX + c->sizex,
							offset.y + offsetY + font.height + 1));
			offsetY += font.height + PaddingRows;
			display_HorizontalLine(offset.x + offsetX, offset.y + offsetY, c->sizex);
			for (uint8_t i = 0; i < lines; i++) {
				char val[c->digits + 1];
				if (i + topVisibleEntry < rows) {
					Unit::StringFromValue(val, c->digits,
							c->data[i + topVisibleEntry], c->unit);
				} else {
					memset(val, ' ', c->digits);
					val[c->digits] = 0;
				}
				if (!c->editable) {
					display_SetForeground(COLOR_GRAY);
				} else if (selected) {
					if (i + topVisibleEntry == selected_row
							&& column_cnt == selected_column) {
						display_SetForeground(COLOR_SELECTED);
					}
				}
				display_String(offset.x + offsetX + 1, offset.y + offsetY + 2, val);
				display_SetForeground(COLOR_BLACK);
				offsetY += font.height + PaddingRows;
				display_HorizontalLine(offset.x + offsetX, offset.y + offsetY, c->sizex);
			}
			offsetX += c->sizex;
			display_VerticalLine(offset.x + offsetX, offset.y, size.y);
			c = c->next;
			column_cnt++;
		}
		// draw scrollbar
		display_Rectangle(offset.x + offsetX, offset.y,
				offset.x + offsetX + ScrollbarSize, offset.y + size.y - 1);
		/* calculate beginning and end of scrollbar */
		uint8_t scrollBegin = common_Map(topVisibleEntry, 0, rows, 0, size.y);
		uint8_t scrollEnd = size.y;
		if (rows > lines) {
			scrollEnd = common_Map(topVisibleEntry + lines, 0, rows, 0, size.y);
		}
		/* display position indicator */
		display_SetForeground(COLOR_BG_DEFAULT);
		display_RectangleFull(offset.x + offsetX + 1, offset.y + 1,
				offset.x + offsetX + ScrollbarSize - 1, offset.y + scrollBegin);
		display_RectangleFull(offset.x + offsetX + 1, offset.y + scrollEnd - 1,
				offset.x + offsetX + ScrollbarSize - 1, offset.y + size.y - 2);

		display_SetForeground(ScrollbarColor);
		display_RectangleFull(offset.x + offsetX + 1,
				offset.y + scrollBegin + 1,
				offset.x + offsetX + ScrollbarSize - 1,
				offset.y + scrollEnd - 2);
	}
	void changeValue() {
		uint8_t column = 0;
		Column *c = columns;
		while (c) {
			if (column == selected_column) {
				if (!c->editable) {
					// this column is not changeable
					return;
				}
				new ValueInput<T>("New cell value:", &c->data[selected_row],
						c->unit, nullptr, nullptr);
			}
			column++;
			c = c->next;
		}
	}
	void input(GUIEvent_t *ev) override {
		if (!selectable) {
			return;
		}
	    switch(ev->type) {
		case EVENT_TOUCH_PRESSED: {
			uint8_t new_row = ev->pos.y / (font.height + PaddingRows);
			if (new_row >= rows) {
				new_row = rows - 1;
			} else if (new_row > 0) {
				// remove offset caused by column name
				new_row--;
			}
			new_row += topVisibleEntry;
			uint8_t new_column = 0;
			Column *c = columns;
			uint16_t x_cnt = 0;
			while (c) {
				x_cnt += c->sizex;
				if (x_cnt >= ev->pos.x) {
					break;
				}
				new_column++;
			}
			if (new_row != selected_row || new_column != selected_column) {
				selected_row = new_row;
				selected_column = new_column;
				requestRedraw();
			} else {
				// same cell was already selected
				changeValue();
			}
	    }
			ev->type = EVENT_NONE;
	    	break;
		case EVENT_BUTTON_CLICKED:
			if ((ev->button & BUTTON_LEFT) && selected_column > 0) {
				selected_column--;
				requestRedraw();
			}
			if ((ev->button & BUTTON_UP) && selected_row > 0) {
				selected_row--;
				if (selected_row < topVisibleEntry) {
					topVisibleEntry = selected_row;
				}
				requestRedraw();
			}
			if ((ev->button & BUTTON_DOWN) && selected_row < rows - 1) {
				selected_row++;
				if (selected_row >= lines) {
					topVisibleEntry = selected_row - lines + 1;
				}
				requestRedraw();
			}
			if ((ev->button & BUTTON_RIGHT) && selected_column < cols - 1) {
				selected_column++;
				requestRedraw();
			}
			if ((ev->button & BUTTON_ENCODER)) {
				changeValue();
			}
			ev->type = EVENT_NONE;
			break;
		case EVENT_ENCODER_MOVED:
			if (ev->movement > 0) {
				if (selected_row < rows - 1) {
					selected_row++;
					if (selected_row >= lines) {
						topVisibleEntry = selected_row - lines + 1;
					}
				} else {
					selected_row = 0;
					topVisibleEntry = 0;
					if (selected_column < cols - 1) {
						selected_column++;
					} else {
						selected_column = 0;
					}
				}
			} else {
				if (selected_row > 0) {
					selected_row--;
					if (selected_row < topVisibleEntry) {
						topVisibleEntry = selected_row;
					}
				} else {
					selected_row = rows - 1;
					if (selected_row >= lines) {
						topVisibleEntry = selected_row - lines + 1;
					}
					if (selected_column > 0) {
						selected_column--;
					} else {
						selected_column = cols - 1;
					}
				}
			}
			requestRedraw();
			ev->type = EVENT_NONE;
			break;
	    default:
	    	break;
	    }
	    return;
	}

	using Column = struct column {
		char name[MaxNameLength];
		const Unit::unit **unit;
		uint8_t digits;
		bool editable;
		uint16_t sizex;
		T *data;
		column *next;
	};
	font_t font;
	Column *columns;
	uint8_t rows;
	uint8_t cols;
	uint8_t selected_row;
	uint8_t selected_column;
    uint8_t lines;
    uint8_t topVisibleEntry;
};
