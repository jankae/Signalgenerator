#ifndef FILE_H_
#define FILE_H_

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "fatfs.h"

#define FILE_ENTRY_MAX_NAMELENGTH		30

typedef enum {PTR_INT8, PTR_INT16, PTR_INT32, PTR_FLOAT, PTR_STRING} filePointerType_t;

typedef enum {FILE_OK, FILE_PARTIAL, FILE_ERROR} fileResult_t;

typedef struct {
	char name[FILE_ENTRY_MAX_NAMELENGTH];
	void *ptr;
	filePointerType_t type;
} fileEntry_t;

int8_t file_Init(void);
FRESULT file_open(const char *filename, BYTE mode);
FRESULT file_close(void);
char* file_ReadLine(char *line, uint16_t maxLength);
int file_WriteLine(const char *line);
void file_WriteParameters(const fileEntry_t *paramList, uint8_t length);
fileResult_t file_ReadParameters(const fileEntry_t *paramList, uint8_t length);


#endif /* FILE_H_ */
