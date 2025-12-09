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

static void panic(char* msg) __attribute__((noreturn));

context_frame_t *irq_handler(context_frame_t *old)
{
	if (g_current && g_current->state == T_RUNNING) {
        g_current->ctx = old;
    }

	if (irq_get_systimer_pending(1)) {
		systimer_clear_match(1);
		systimer_increment_compare(1, TIMER_INTERVAL);
		kprintf("!");
		scheduler_pick_next();
	}

	if (irq_get_uart_pending()) {
		if (uart_get_rx_interrupt_status()) {
			uart_rx_into_buffer();
		}
		uart_clear_interrupt();
	}

	return g_current->ctx;
}

void undefined_handler()
{
	panic("Undefined instruction");
	for (;;) {
	}
}

void svc_handler()
{
	panic("SVC");
	for (;;) {
	}
}

void prefetch_abort_handler()
{
	panic("Prefetch abort");
	for (;;) {
	}
}

void data_abort_handler()
{
	panic("Data abort");
	for (;;) {
	}
}

static void panic(char* msg)
{
	__asm__ volatile("cpsid if" : : : "memory");

	kprintf("PANIC: %s\n", msg);

	for (;;) {
		__asm__ volatile("wfi" ::: "memory");
	}
}