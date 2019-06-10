#include "dialog.h"

#include "logging.h"

extern SemaphoreHandle_t fileAccess;
extern TaskHandle_t GUIHandle;

namespace Dialog {

#define INPUT_DIALOG_LENGTH		10

struct {
	Window *window;
	union {
		struct {
			SemaphoreHandle_t dialogDone;
			Result res;
			void (*cb)(Result);
		} msgbox;
		struct {
			SemaphoreHandle_t dialogDone;
			uint8_t OKclicked;
		} fileChooser;
		struct {
			SemaphoreHandle_t dialogDone;
			uint8_t OKclicked;
			char *string;
			Label *lString;
			uint8_t pos;
			uint8_t maxLength;
		} StringInput;
		struct {
			SemaphoreHandle_t dialogDone;
		} UnitInput;
	};
} dialog;

static void MessageBoxButton(Widget &w) {
	dialog.msgbox.res = Result::ERR;
	Button *b = (Button*) &w;
	/* find which button has been pressed */
	if(!strcmp(b->getName(), "OK")) {
		dialog.msgbox.res = Result::OK;
	} else if(!strcmp(b->getName(), "ABORT")) {
		dialog.msgbox.res = Result::ABORT;
	}

	delete dialog.window;

	if (dialog.msgbox.cb)
		dialog.msgbox.cb(dialog.msgbox.res);

	if (dialog.msgbox.dialogDone) {
		xSemaphoreGive(dialog.msgbox.dialogDone);
	}
}

Result MessageBox(const char *title, font_t font, const char *msg,
		MsgBox type, void (*cb)(Result), uint8_t block){
	/* check pointers */
	if (!title || !msg) {
		return Result::ERR;
	}

	memset(&dialog, 0, sizeof(dialog));

	if(block && xTaskGetCurrentTaskHandle() == GUIHandle) {
		/* This dialog must never be called by the GUI thread (Deadlock) */
		CRIT_ERROR("Dialog started from GUI thread.");
	}

	if (block) {
		dialog.msgbox.dialogDone = xSemaphoreCreateBinary();
	}

	/* create dialog window and elements */
	Textfield *text = new Textfield(msg, font);
	/* calculate window size */
	coords_t windowSize = text->getSize();
	if (windowSize.x < 132) {
		windowSize.x = 136;
	} else {
		windowSize.x += 4;
	}
	windowSize.y += 50;
	Window *w = new Window(title, Font_Big, windowSize);
	Container *c = new Container(w->getAvailableArea());
	c->attach(text, COORDS(1, 2));
	switch (type) {
	case MsgBox::OK: {
		Button *bOK = new Button("OK", Font_Big, MessageBoxButton, 65);
		c->attach(bOK, COORDS((c->getSize().x - bOK->getSize().x) / 2,
						c->getSize().y - bOK->getSize().y - 1));
		bOK->select();
	}
	break;
	case MsgBox::ABORT_OK: {
		Button *bOK = new Button("OK", Font_Big, MessageBoxButton, 65);
		Button *bAbort = new Button("ABORT", Font_Big, MessageBoxButton, 65);
		c->attach(bAbort,
				COORDS(c->getSize().x / 2 - bAbort->getSize().x - 1,
						c->getSize().y - bAbort->getSize().y - 1));
		c->attach(bOK,
				COORDS(c->getSize().x / 2 + 1,
						c->getSize().y - bOK->getSize().y - 1));
		bAbort->select();
	}
		break;
	}

	dialog.window = w;
	dialog.msgbox.cb = cb;

	w->setMainWidget(c);

	if(block) {
		/* wait for dialog to finish and return result */
		xSemaphoreTake(dialog.msgbox.dialogDone, portMAX_DELAY);
		vPortFree(dialog.msgbox.dialogDone);
		return dialog.msgbox.res;
	} else {
		/* non blocking mode, return immediately */
		return Result::OK;
	}
}

Result FileChooser(const char *title, char *result,
		const char *dir, const char *filetype) {
	if(xTaskGetCurrentTaskHandle() == GUIHandle) {
		/* This dialog must never be called by the GUI thread (Deadlock) */
		CRIT_ERROR("Dialog started from GUI thread.");
	}

	/* check pointers */
	if (!title || !dir) {
		return Result::ERR;
	}

	memset(&dialog, 0, sizeof(dialog));

	dialog.fileChooser.dialogDone = xSemaphoreCreateBinary();
	if(!dialog.fileChooser.dialogDone) {
		/* failed to create semaphore */
		return Result::ERR;
	}

	/* Find applicable files */
	if(!xSemaphoreTake(fileAccess, 1000)) {
		/* failed to allocate fileAccess */
		return Result::ERR;
	}

#define MAX_NUMBER_OF_FILES		50
	char *filenames[MAX_NUMBER_OF_FILES + 1];
	uint8_t foundFiles = 0;
	FRESULT fr; /* Return value */
	DIR *dj = (DIR*) pvPortMalloc(sizeof(DIR)); /* Directory search object */
	FILINFO fno; /* File information */

#if _USE_LFN
	static char lfn[_MAX_LFN + 1]; /* Buffer to store the LFN */
	fno.lfname = lfn;
	fno.lfsize = sizeof lfn;
#endif

	fr = f_opendir(dj, dir);

	if (fr == FR_OK) {
		while (f_readdir(dj, &fno) == FR_OK) {
			char *fn;
#if _USE_LFN
			fn = *fno.lfname ? fno.lfname : fno.fname;
#else
			fn = fno.fname;
#endif
			if(!fn[0] || foundFiles >= MAX_NUMBER_OF_FILES)
				/* no more files */
				break;
			if (filetype) {
				/* check file for filetype */
				char *typestart = strchr(fn, '.');
				if (!typestart) {
					/* file has not type */
					continue;
				}
				typestart++;
				if (strcmp(typestart, filetype)) {
					/* type doesn't match */
					continue;
				}
			}
			/* allocate memory for filename */
			filenames[foundFiles] = (char*) pvPortMalloc(strlen(fn) + 1);
			if (!filenames[foundFiles]) {
				/* malloc failed */
				/* free already allocated names and return with error */
				while (foundFiles > 0) {
					foundFiles--;
					vPortFree(filenames[foundFiles]);
				}
				vPortFree(dj);
				return Result::ERR;
			}
			/* copy filename */
			strcpy(filenames[foundFiles], fn);
			/* switch to next filename */
			foundFiles++;
		}
		f_closedir(dj);
	}
	vPortFree(dj);
	xSemaphoreGive(fileAccess);
	/* Got all matching filenames */
	/* mark end of filename strings */
	filenames[foundFiles] = 0;

	/* Create window */
	Window *w = new Window(title, Font_Big, COORDS(280, 200));
	Container *c = new Container(w->getAvailableArea());

	Button *bAbort = new Button("ABORT", Font_Big, [](Widget &w) {
		dialog.fileChooser.OKclicked = 0;
		xSemaphoreGive(dialog.fileChooser.dialogDone);
	}, 80);

	uint8_t selectedFile = 0;
	if (foundFiles) {
		ItemChooser *i = new ItemChooser((const char**) filenames,
				&selectedFile, Font_Big,
				(c->getSize().y - 30) / Font_Big.height, c->getSize().x);
		c->attach(i, COORDS(0, 0));
		Button *bOK = new Button("OK", Font_Big, [](Widget &w) {
			dialog.fileChooser.OKclicked = 1;
			xSemaphoreGive(dialog.fileChooser.dialogDone);
		}, 80);
		c->attach(bOK,
				COORDS(c->getSize().x - bOK->getSize().x - 5,
						c->getSize().y - bOK->getSize().y - 5));
		i->select();
	} else {
		/* got no files */
		Label *lNoFiles = new Label("No files available", Font_Big);
		c->attach(lNoFiles,
				COORDS((c->getSize().x - lNoFiles->getSize().x) / 2, 40));
		bAbort->select();
	}


	c->attach(bAbort,
			COORDS(5, c->getSize().y - bAbort->getSize().y - 5));

	w->setMainWidget(c);

	/* Wait for button to be clicked */
	xSemaphoreTake(dialog.fileChooser.dialogDone, portMAX_DELAY);
	vPortFree(dialog.fileChooser.dialogDone);

	if(dialog.fileChooser.OKclicked) {
		strcpy(result, filenames[selectedFile]);
	}

	/* free all allocated filenames */
	while (foundFiles > 0) {
		foundFiles--;
		vPortFree(filenames[foundFiles]);
	}

	/* delete window */
	delete w;

	if(dialog.fileChooser.OKclicked) {
		return Result::OK;
	} else {
		return Result::ERR;
	}
}

static void stringInputChar(char c) {
	if(c == 0x08) {
		/* backspace, delete last char */
		if(dialog.StringInput.pos>0) {
			dialog.StringInput.pos--;
			dialog.StringInput.string[dialog.StringInput.pos] = 0;
			dialog.StringInput.lString->setText(dialog.StringInput.string);
		}
	} else {
		/* append character if space is available */
		if(dialog.StringInput.pos<dialog.StringInput.maxLength - 1) {
			dialog.StringInput.string[dialog.StringInput.pos] = c;
			dialog.StringInput.pos++;
			dialog.StringInput.lString->setText(dialog.StringInput.string);
		}
	}
}

Result StringInput(const char *title, char *result, uint8_t maxLength) {
	if(xTaskGetCurrentTaskHandle() == GUIHandle) {
		/* This dialog must never be called by the GUI thread (Deadlock) */
		CRIT_ERROR("Dialog started from GUI thread.");
	}

	/* check pointers */
	if (!title || !result) {
		return Result::ERR;
	}

	memset(&dialog, 0, sizeof(dialog));

	dialog.StringInput.dialogDone = xSemaphoreCreateBinary();
	if(!dialog.StringInput.dialogDone) {
		/* failed to create semaphore */
		return Result::ERR;
	}

	dialog.StringInput.string = result;
	dialog.StringInput.maxLength = maxLength;
	dialog.StringInput.pos = 0;

	memset(result, 0, maxLength);

	/* Create window */
	Window *w = new Window(title, Font_Big, COORDS(313, 233));
	Container *c = new Container(w->getAvailableArea());

	Keyboard *k = new Keyboard(stringInputChar);

	dialog.StringInput.lString = new Label(
			c->getSize().x / Font_Big.width, Font_Big, Label::Orientation::CENTER);

	/* Create buttons */
	Button *bOK = new Button("OK", Font_Big, [](Widget &w) {
		dialog.StringInput.OKclicked = 1;
		xSemaphoreGive(dialog.StringInput.dialogDone);
	}, 80);
	Button *bAbort = new Button("ABORT", Font_Big, [](Widget &w) {
		dialog.StringInput.OKclicked = 0;
		xSemaphoreGive(dialog.StringInput.dialogDone);
	}, 80);

	c->attach(dialog.StringInput.lString, COORDS(0, 8));
	c->attach(k, COORDS(0, 30));
	c->attach(bOK,
			COORDS(c->getSize().x - bOK->getSize().x - 5,
					c->getSize().y - bOK->getSize().y - 5));
	c->attach(bAbort,
			COORDS(5, c->getSize().y - bAbort->getSize().y - 5));

	k->select();
	w->setMainWidget(c);

	/* Wait for button to be clicked */
	xSemaphoreTake(dialog.StringInput.dialogDone, portMAX_DELAY);
	vPortFree(dialog.StringInput.dialogDone);

	/* delete window */
	delete w;

	if(dialog.StringInput.OKclicked) {
		return Result::OK;
	} else {
		return Result::ABORT;
	}
}

Result UnitInput(const char *title, int32_t *result, uint8_t maxLength, const unit_t *unit) {
	if(xTaskGetCurrentTaskHandle() == GUIHandle) {
		/* This dialog must never be called by the GUI thread (Deadlock) */
		CRIT_ERROR("Dialog started from GUI thread.");
	}

	/* check pointers */
	if (!title || !result) {
		return Result::ERR;
	}

	memset(&dialog, 0, sizeof(dialog));

	dialog.UnitInput.dialogDone = xSemaphoreCreateBinary();
	if(!dialog.UnitInput.dialogDone) {
		/* failed to create semaphore */
		return Result::ERR;
	}

	*result = 0;

	/* Create window */
	Window *w = new Window(title, Font_Big, COORDS(313, 233));
	Container *c = new Container(w->getAvailableArea());

	Entry *e = new Entry(result, NULL, NULL, Font_Big, maxLength, unit);

	/* Create buttons */
	Button *bOK = new Button("OK", Font_Big, [](Widget &w) {
		xSemaphoreGive(dialog.UnitInput.dialogDone);
	}, 80);


	c->attach(e, COORDS((c->getSize().x - e->getSize().x) / 2, 5));
	c->attach(bOK,
			COORDS((c->getSize().x - bOK->getSize().x) / 2,
					c->getSize().y - bOK->getSize().y - 5));

	e->select();
	w->setMainWidget(c);

	/* Wait for button to be clicked */
	xSemaphoreTake(dialog.UnitInput.dialogDone, portMAX_DELAY);
	vPortFree(dialog.UnitInput.dialogDone);

	/* delete window */
	delete w;

	return Result::OK;
}

}
