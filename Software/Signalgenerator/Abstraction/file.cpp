#include "file.h"

#include "semphr.h"

SemaphoreHandle_t fileAccess;

static FIL file;
static uint8_t fileOpened = 0;

FATFS fatfs;

int8_t file_Init(void) {
	fileAccess = xSemaphoreCreateMutex();
	if (!fileAccess) {
		return -1;
	}

	/* Check SD card */
	if(f_mount(&fatfs, "0:/", 1)==FR_OK) {
		return 1;
	} else {
		return 0;
	}
}

FRESULT file_open(const char *filename, BYTE mode) {
	if (xSemaphoreTake(fileAccess, 100)) {
		FRESULT res = f_open(&file, filename, mode);
		if (res == FR_OK) {
			fileOpened = 1;
		}
		return res;
	} else {
		return FR_DENIED;
	}
}

FRESULT file_close(void) {
	FRESULT res;
	if (fileOpened) {
		res = f_close(&file);
		fileOpened = 0;
		xSemaphoreGive(fileAccess);
	} else {
		return FR_NO_FILE;
	}
	return res;
}

char* file_ReadLine(char *line, uint16_t maxLength) {
	return f_gets(line, maxLength, &file);
}

int file_WriteLine(const char *line) {
	return f_puts(line, &file);
}

void file_WriteParameters(const fileEntry_t *paramList,
		uint8_t length) {
	if (fileOpened) {
		/* opened file, now write parameters */
		uint8_t i;
		for (i = 0; i < length; i++) {
			f_puts(paramList[i].name, &file);
			f_puts(" = ", &file);
#define		MAX_PARAM_LENGTH		16
			char buf[MAX_PARAM_LENGTH + 2];
			switch (paramList[i].type) {
			case PTR_INT8:
				snprintf(buf, sizeof(buf), "%d\n", *(int8_t*) paramList[i].ptr);
				break;
			case PTR_INT16:
				snprintf(buf, sizeof(buf), "%d\n",
						*(int16_t*) paramList[i].ptr);
				break;
			case PTR_INT32:
				snprintf(buf, sizeof(buf), "%ld\n",
						*(int32_t*) paramList[i].ptr);
				break;
			case PTR_FLOAT:
				snprintf(buf, sizeof(buf), "%f\n", *(float*) paramList[i].ptr);
				break;
			case PTR_STRING:
				snprintf(buf, sizeof(buf), "%s\n", (char*) paramList[i].ptr);
				break;
			default:
				strcpy(buf, "UNKNOWN TYPE\n");
			}
			f_puts(buf, &file);
		}
	}
}

fileResult_t file_ReadParameters(const fileEntry_t *paramList,
		uint8_t length) {
	if (fileOpened) {
		/* opened file, now read parameters */
		char line[50];
		uint8_t valueSet[length];
		memset(valueSet, 0, sizeof(valueSet));
		while (f_gets(line, sizeof(line), &file)) {
			if (line[0] == '#') {
				/* skip comment lines */
				continue;
			}
			/* find matching parameter */
			uint8_t i;
			for (i = 0; i < length; i++) {
				if (!strncmp(line, paramList[i].name,
						strlen(paramList[i].name))) {
					/* found a match */
					char *start = strchr(line, '=');
					if (start) {
						/* Skip leading spaces */
						while (*++start == ' ')
							;
						/* store value in correct format */
						switch (paramList[i].type) {
						case PTR_INT8:
							*(int8_t*) paramList[i].ptr = (int8_t) strtol(start,
									NULL, 0);
							break;
						case PTR_INT16:
							*(int16_t*) paramList[i].ptr = (int16_t) strtol(
									start, NULL, 0);
							break;
						case PTR_INT32:
							*(int32_t*) paramList[i].ptr = (int32_t) strtol(
									start, NULL, 0);
							break;
						case PTR_FLOAT:
							*(float*) paramList[i].ptr = strtof(start,
							NULL);
							break;
						case PTR_STRING:
							strcpy((char*) paramList[i].ptr, start);
							break;
						}
						/* mark parameter as set */
						valueSet[i] = 1;
					}

				}
			}

			/* Check if all parameters have been set */
			for (i = 0; i < length; i++) {
				if (!valueSet[i]) {
					break;
				}
			}
			if(i==length) {
				/* all parameters have been set */
				return FILE_OK;
			}
		}
	}
	/* file ended before all parameters have been set */
	return FILE_PARTIAL;
}

