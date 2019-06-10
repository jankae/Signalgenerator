#ifndef LOGGING_H_
#define LOGGING_H_

#include <stdint.h>

#define CRIT_ERROR(fmt, ...) Log_CritError(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
extern "C"
{
#endif

void Log_CritError(const char *filename, uint16_t line, const char *fmt, ...);

void prvGetRegistersFromStack(uint32_t *pulFaultStackAddress);

#ifdef __cplusplus
}
#endif

#endif
