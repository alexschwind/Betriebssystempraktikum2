#ifndef UART_H
#define UART_H

#include <stdbool.h>

#define UART_BASE (0x7E201000u - 0x3F000000u)

struct uart {
	unsigned int dr;
	unsigned int rsrecr;
	unsigned int unused0[4];
	unsigned int fr;
	unsigned int unused1;
	unsigned int ilpr;
	unsigned int ibrd;
	unsigned int fbrd;
	unsigned int lcrh;
	unsigned int cr;
	unsigned int ifls;
	unsigned int imsc;
	unsigned int ris;
	unsigned int mis;
	unsigned int icr;
	unsigned int dmacr;
	unsigned int itcr;
	unsigned int itip;
	unsigned int itop;
	unsigned int tdr;
};

extern volatile struct uart *const uart;

void uart_init(void);
char uart_getc(void);
void uart_putc(char c);
void uart_puts(const char *str);
bool uart_getc_nonblocking(char *out);

void	     uart_enable_rx_interrupt(void);
unsigned int uart_get_rx_interrupt_status(void);
void	     uart_clear_interrupt(void);
void	     uart_rx_into_buffer(void);

#endif
