#include <kernel/handlers.h>
#include <kernel/scheduler.h>
#include <kernel/syscall.h>

#include <lib/kprintf.h>

#include <arch/bsp/uart.h>
#include <arch/bsp/irq.h>
#include <arch/bsp/systimer.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <config.h>

static void panic(char* msg) __attribute__((noreturn));

static inline uint32_t decode_svc_number(context_frame_t *ctx)
{
	uint32_t svc_pc = ctx->lr - 4; // lr points to instruction after SVC
	return (*(uint32_t *)svc_pc) & 0x00FFFFFF;
}

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

context_frame_t *svc_handler(context_frame_t *ctx)
{
	if (g_current) {
		g_current->ctx = ctx;
	}

	uint32_t svc_no = decode_svc_number(ctx);
	switch (svc_no) {
	case SYSCALL_EXIT:
		if (g_current) {
			g_current->state = T_UNUSED;
		}
		scheduler_pick_next();
		kprintf("Thread exited\n");
		return g_current->ctx;
	default:
		panic("Unknown SVC");
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