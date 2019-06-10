/*
 * dialog.h
 *
 *  Created on: Mar 16, 2017
 *      Author: jan
 */

#ifndef DIALOG_H_
#define DIALOG_H_

#include "gui.h"

namespace Dialog {

enum class MsgBox {
	ABORT_OK, OK
};

enum class Result {
	OK, ABORT, ERR
};

Result MessageBox(const char *title, font_t font, const char *msg,
		MsgBox type, void (*cb)(Result), uint8_t block);

Result FileChooser(const char *title, char *result,
		const char *dir, const char *filetype);

Result StringInput(const char *title, char *result, uint8_t maxLength);

Result UnitInput(const char *title, int32_t *result, uint8_t maxLength, const unit_t *unit);

}

#endif /* DIALOG_H_ */
