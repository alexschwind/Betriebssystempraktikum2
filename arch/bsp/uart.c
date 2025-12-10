#include <arch/bsp/uart.h>
#include <arch/bsp/gpio.h>

#include <lib/ringbuffer.h>

#include <config.h>

volatile struct uart *const uart = (struct uart *)UART_BASE;

create_ringbuffer(uart_rx_buffer, UART_INPUT_BUFFER_SIZE);

void uart_init(void)
{
	gpio_set_alt_function(14, 0); // Set GPIO14 to ALT0 (UART0 TX)
	gpio_set_alt_function(15, 0); // Set GPIO15 to ALT0 (UART0 RX)

	uart->cr &= ~1u; // disable UART
	while (uart->fr & (1u << 3)) { // wait for end of transmission
		/* wait */
	}
	uart->imsc = 0u; // disable all interrupts
	uart->icr  = 0x7FFu; // clear all interrupts
	uart->lcrh &= ~(1u << 4); // flush and disable FIFOs

	uart->lcrh |= (1u << 4); // Enable fifos
	uart->ifls = (1u << 4); // set RX FIFO interrupt trigger to 1/2 full (bits 5:3 = 0b010)
	uart->cr |= (1u << 9); // enable RX
	uart->cr |= (1u << 8); // enable TX
	uart->cr |= 1u; // enable UART
}

void uart_enable_rx_interrupt(void)
{
	uart->imsc |= (1u << 4);
}

unsigned int uart_get_rx_interrupt_status(void)
{
	return (uart->mis >> 4) & 0x1u;
}

void uart_clear_interrupt(void)
{
	uart->icr = 0x7FFu;
}

void uart_rx_into_buffer(void)
{
	while (!(uart->fr & (1u << 4))) {
		if (buff_is_full(uart_rx_buffer)) {
			break;
		}
		char c = (char)(uart->dr & 0xFF);
		buff_putc(uart_rx_buffer, c);
	}
}

char uart_getc(void)
{
	while (buff_is_empty(uart_rx_buffer)) {
		/* If the ring buffer is empty but the hardware FIFO has data, grab one byte. */
		if (!(uart->fr & (1u << 4))) {
			buff_putc(uart_rx_buffer, (char)(uart->dr & 0xFF));
			break;
		}
	}
	return buff_getc(uart_rx_buffer);
}

void uart_putc(char c)
{
	while (uart->fr & (1u << 5)) {
		/* TX FIFO full, keep waiting */
	}
	uart->dr = (unsigned int)c;
}

void uart_puts(const char *str)
{
	if (!str) {
		return;
	}

	while (*str) {
		uart_putc(*str++);
	}
}

bool uart_getc_nonblocking(char *out)
{
	if (!out) {
		return false;
	}

	if (buff_is_empty(uart_rx_buffer)) {
		return false;
	}

	*out = buff_getc(uart_rx_buffer);
	return true;
}