#include <kernel/handlers.h>
#include <kernel/scheduler.h>

#include <lib/kprintf.h>

#include <arch/bsp/uart.h>
#include <arch/bsp/irq.h>
#include <arch/bsp/systimer.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <config.h>

static void panic(void) __attribute__((noreturn));

void irq_handler(struct exception_frame *frame)
{
	if (irq_get_systimer_pending(1)) {
		systimer_clear_match(1);
		systimer_increment_compare(1, TIMER_INTERVAL);
		kprintf("!");
		schedule(); // TODO implement
	}

	if (irq_get_uart_pending()) {
		if (uart_get_rx_interrupt_status()) {
			uart_rx_into_buffer();
		}
		uart_clear_interrupt();
		char c;
		while (uart_getc_nonblocking(&c)) { // TODO implement uart_getc_nonblocking
			switch (c) {
				case 'x':
					kprintf("Do stuff in kernel"); // This is for later when the kernel should also trigger exceptions.
					break;
				default:
					scheduler_create_thread(); // TODO implement, should take a function pointer and the char as argument
					schedule(); // TODO implement
					break;
			}	
		}
	}
}

void undefined_handler(struct exception_frame *frame)
{
	panic();
	for (;;) {
	}
}

void svc_handler(struct exception_frame *frame)
{
	panic();
	for (;;) {
	}
}

void prefetch_abort_handler(struct exception_frame *frame)
{
	panic();
	for (;;) {
	}
}

void data_abort_handler(struct exception_frame *frame)
{
	panic();
	for (;;) {
	}
}

static void panic(void)
{
	__asm__ volatile("cpsid if" : : : "memory");

	kprintf("\4");

	for (;;) {
		__asm__ volatile("wfi" ::: "memory");
	}
}