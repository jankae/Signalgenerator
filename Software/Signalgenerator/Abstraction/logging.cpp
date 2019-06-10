#include "logging.h"
#include "display.h"

#include <stdarg.h>

void Log_CritError(const char *filename, uint16_t line, const char *fmt, ...) {
	/* Something bad happened */
//	pushpull_AcquireControl();
//	pushpull_SetDefault();

	display_SetForeground(COLOR_RED);
	display_SetBackground(COLOR_BLACK);
	display_Clear();
	display_SetFont(Font_Big);
	display_String(0, 0, "CRITICAL ERROR:");
	display_SetForeground(COLOR_WHITE);
	display_SetFont(Font_Medium);
	display_String(0, 16, filename);
	char buffer[54];
	snprintf(buffer, sizeof(buffer), "Line: %u", line);
	display_String(0, 24, buffer);

	va_list arp;
	va_start(arp, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, arp);
	va_end(arp);

	display_String(0, 40, buffer);

	while(1);
}


/**
 * @see http://www.freertos.org/Debugging-Hard-Faults-On-Cortex-M-Microcontrollers.html
 */
void prvGetRegistersFromStack(uint32_t *pulFaultStackAddress)  __attribute__((no_instrument_function));
void prvGetRegistersFromStack(uint32_t *pulFaultStackAddress)
{
/* These are volatile to try and prevent the compiler/linker optimising them
away as the variables never actually get used. If the debugger won't show the
values of the variables, make them global my moving their declaration outside
of this function. */
//  volatile uint32_t r0;
//  volatile uint32_t r1;
//  volatile uint32_t r2;
//  volatile uint32_t r3;
//  volatile uint32_t r12;
//  volatile uint32_t lr; /* Link register. */
  volatile uint32_t pc; /* Program counter. */
//  volatile uint32_t psr;/* Program status register. */

//  r0 = pulFaultStackAddress[0];
//  r1 = pulFaultStackAddress[1];
//  r2 = pulFaultStackAddress[2];
//  r3 = pulFaultStackAddress[3];
//
//  r12 = pulFaultStackAddress[4];
//  lr = pulFaultStackAddress[5];
  pc = pulFaultStackAddress[6];
//  psr = pulFaultStackAddress[7];

  /* When the following line is hit, the variables contain the register values. */
  CRIT_ERROR("Hardfault at: PC 0x%lx\n", pc);
}

