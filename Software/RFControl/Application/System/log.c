#include "log.h"

#include "stm32f0xx.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* Automatically build register and function names based on USART selection */
#define USART_M2(y) 		USART ## y
#define USART_M1(y)  		USART_M2(y)
#define USART_BASE			USART_M1(LOG_USART)

#define HANDLER_M2(x) 		USART ## x ## _IRQHandler
#define HANDLER_M1(x)  		HANDLER_M2(x)
#define HANDLER				HANDLER_M1(LOG_USART)

#define NVIC_ISR_M2(x) 		USART ## x ## _IRQn
#define NVIC_ISR_M1(x)  	NVIC_ISR_M2(x)
#define NVIC_ISR			NVIC_ISR_M1(LOG_USART)

#define CLK_ENABLE_M2(x) 	__HAL_RCC_USART ## x ## _CLK_ENABLE()
#define CLK_ENABLE_M1(x)  	CLK_ENABLE_M2(x)
#define CLK_ENABLE()		CLK_ENABLE_M1(LOG_USART)

#define CLK_DISABLE_M2(x) 	__HAL_RCC_USART ## x ## _CLK_DISABLE()
#define CLK_DISABLE_M1(x)  	CLK_DISABLE_M2(x)
#define CLK_DISABLE()		CLK_DISABLE_M1(LOG_USART)


#define MAX_LINE_LENGTH		256

#ifdef USART_SR_TXE
#define USART_ISR_REG		SR
#define USART_RXNE			USART_SR_RXNE
#define USART_TXE			USART_SR_TXE
#define USART_TC			USART_SR_TC
#define USART_READ			DR
#define USART_WRITE			DR
#else
#define USART_ISR_REG		ISR
#define USART_RXNE			USART_ISR_RXNE
#define USART_TXE			USART_ISR_TXE
#define USART_TC			USART_ISR_TC
#define USART_READ			RDR
#define USART_WRITE			TDR
#endif

static char fifo[LOG_SENDBUF_LENGTH + MAX_LINE_LENGTH];
static uint16_t fifo_write, fifo_read;

#define LOG_ISR_PRIO		0

static const char lvl_strings[][4] = {
	"DBG",
	"INF",
	"WRN",
	"ERR",
	"CRT",
};

#define INC_FIFO_POS(pos, inc) do { pos = (pos + inc) % LOG_SENDBUF_LENGTH; } while(0)

static uint16_t fifo_space() {
	uint16_t used;
	if(fifo_write >= fifo_read) {
		used = fifo_write - fifo_read;
	} else {
		used = fifo_write - fifo_read + LOG_SENDBUF_LENGTH;
	}
	return LOG_SENDBUF_LENGTH - used - 1;
}

void log_init() {
	fifo_write = 0;
	fifo_read = 0;
#ifdef LOG_USE_MUTEXES
	mutex = xSemaphoreCreateMutexStatic(&xMutex);
#endif

	/* USART interrupt Init */
	HAL_NVIC_SetPriority(NVIC_ISR, LOG_ISR_PRIO, 0);
	HAL_NVIC_EnableIRQ(NVIC_ISR);
}

void log_write(const char *module, uint8_t level, const char *fmt, ...) {
	int written = 0;
	va_list args;
	va_start(args, fmt);
	uint8_t lvl = 31 - __builtin_clz(level);
	written = snprintf(&fifo[fifo_write], MAX_LINE_LENGTH, "%05lu [%6.6s,%s]: ",
			HAL_GetTick(), module + 4, lvl_strings[lvl]);
	written += vsnprintf(&fifo[fifo_write + written], MAX_LINE_LENGTH - written,
			fmt, args);
	written += snprintf(&fifo[fifo_write + written], MAX_LINE_LENGTH - written,
			"\r\n");
	// check if line still fits into ring buffer
	if (written > fifo_space()) {
		// unable to fit line, skip
		return;
	}
	int16_t overflow = (fifo_write + written) - LOG_SENDBUF_LENGTH;
	if (overflow > 0) {
		// printf wrote over the end of the ring buffer -> wrap around
		memmove(&fifo[0], &fifo[LOG_SENDBUF_LENGTH], overflow);
	}
	INC_FIFO_POS(fifo_write, written);
	// enable interrupt
	CLK_ENABLE();
	USART_BASE->CR1 |= USART_CR1_TXEIE | USART_CR1_TCIE;
}

void log_flush() {
	while (USART_BASE->CR1 & USART_CR1_TCIE)
		;
}

#ifdef LOG_RECEIVE
static uint8_t *rec_buffer = NULL;
static uint16_t rec_buffer_len = 0;
static rec_cb rec_handler = NULL;
static uint16_t rec_index = 0;
void log_set_receive_handler(rec_cb handler, uint8_t *rec_buf, uint16_t buf_len) {
	rec_handler = NULL;
	rec_buffer = rec_buf;
	rec_buffer_len = buf_len;
	rec_index = 0;
	rec_handler = handler;

	// enable receive interrupt
	USART_BASE->CR1 |= USART_CR1_RXNEIE;
}
#endif

void log_force(const char *fmt, ...) {
	CLK_ENABLE();
	// disable ISR
	USART_BASE->CR1 &= ~(USART_CR1_TXEIE|USART_CR1_TCIE);

	int written = 0;
	va_list args;
	va_start(args, fmt);
	written += vsnprintf(&fifo[written], MAX_LINE_LENGTH - written,
			fmt, args);
	written += snprintf(&fifo[written], MAX_LINE_LENGTH - written,
			"\r\n");

	for (uint16_t i = 0; i < written; i++) {
		while (!(USART_BASE->USART_ISR_REG & USART_TXE));
		USART_BASE->USART_WRITE = fifo[i];
	}
}

/* Implemented directly here for speed reasons. Disable interrupt in CubeMX! */
void HANDLER(void) {
	if ((USART_BASE->CR1 & USART_CR1_TCIE)
			&& (USART_BASE->USART_ISR_REG & USART_TC)) {
		// clear flag
		USART_BASE->USART_ISR_REG &= ~USART_TC;
		if (!(USART_BASE->CR1 & USART_CR1_TXEIE)) {
			USART_BASE->CR1 &= ~USART_CR1_TCIE;
#ifdef LOG_RECEIVE
			if (rec_buffer == 0)
#endif
				CLK_DISABLE();
		}
	}
	if ((USART_BASE->CR1 & USART_CR1_TXEIE)
			&& (USART_BASE->USART_ISR_REG & USART_TXE)) {
		if (fifo_read != fifo_write) {
			USART_BASE->USART_WRITE = fifo[fifo_read];
			INC_FIFO_POS(fifo_read, 1);
		} else {
			// all done, disable interrupt
			USART_BASE->CR1 &= ~USART_CR1_TXEIE;
		}
	}
#ifdef LOG_RECEIVE
	if ((USART_BASE->CR1 & USART_CR1_RXNEIE)
			&& (USART_BASE->USART_ISR_REG & USART_RXNE)) {
		// received something
		if (rec_buffer && rec_index < rec_buffer_len) {
			rec_buffer[rec_index++] = USART_BASE->USART_READ;
			if (rec_buffer[rec_index - 1] == '\n') {
				// got a complete line
				if (rec_handler) {
					rec_handler(rec_buffer, rec_index);
				}
				rec_index = 0;
			}
		} else {
			// receive buffer full, throw away
			rec_index = 0;
		}
	}
#endif
}

