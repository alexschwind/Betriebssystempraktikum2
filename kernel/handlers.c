#include <kernel/handlers.h>
#include <kernel/scheduler.h>
#include <user/main.h>
#include <kernel/syscall.h>

#include <lib/kprintf.h>

#include <arch/bsp/uart.h>
#include <arch/bsp/irq.h>
#include <arch/bsp/systimer.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <config.h>

#include <lib/exception_print.h>

//#define DEBUG

static void panic(char* msg) __attribute__((noreturn));
static uint32_t read_dfsr(void);
static uint32_t read_dfar(void);
static uint32_t read_ifsr(void);
static uint32_t read_ifar(void);

static inline uint32_t decode_svc_number(context_frame_t *ctx)
{
	uint32_t svc_pc = ctx->lr - 4; // lr points to instruction after SVC
	return (*(uint32_t *)svc_pc) & 0x00FFFFFF;
}

context_frame_t *irq_handler(context_frame_t *old)
{
#ifdef DEBUG
	struct exception_info info = {
		.exception_name	       = "IRQ",
	};
	print_exception_infos(old, &info);
#endif
	
	if (g_current && g_current->state == T_RUNNING) {
	    g_current->ctx = old;
	}
	if (irq_get_uart_pending()) {
		if (uart_get_rx_interrupt_status()) {
			uart_rx_into_buffer();
		}
		uart_clear_interrupt();
		bool new_thread_created = false;
		char c;
		while (uart_getc_nonblocking(&c)) {
			if (scheduler_thread_create(main, &c, sizeof(c))) {
				new_thread_created = true;
			} 
		}
		if (new_thread_created) {
			scheduler_pick_next();
		}
	}

	if (irq_get_systimer_pending(1)) {
		systimer_clear_match(1);
		systimer_increment_compare(1, TIMER_INTERVAL);
		kprintf("!");
		scheduler_pick_next();
	}

	return g_current->ctx;
}

context_frame_t *svc_handler(context_frame_t *ctx)
{
#ifdef DEBUG
	struct exception_info info = {
		.exception_name	       = "Supervisor Call"
	};

	print_exception_infos(ctx, &info);
#endif

	__asm__ volatile("cpsid i" ::: "memory");

	if (g_current) {
		g_current->ctx = ctx;
	}
	context_frame_t *next_ctx = ctx;

	uint32_t svc_no = decode_svc_number(ctx);
	switch (svc_no) {
	case SYSCALL_EXIT:
		if (g_current) {
			g_current->state = T_UNUSED;
		}
		systimer_increment_compare(1, TIMER_INTERVAL);
		scheduler_pick_next();
		kprintf("\n[irq] thread exited\n");
		next_ctx = g_current->ctx;
		break;
	default:
		panic("Unknown SVC");
	}

	__asm__ volatile("cpsie i" ::: "memory");
	return next_ctx;
}

void undefined_handler()
{
	// struct exception_info info = {
	// 	.exception_name	       = "Undefined Instruction",
	// 	.exception_source_addr = frame ? frame->lr : 0U,
	// };

	// print_exception_infos(frame, &info);
	panic("Undefined instruction");
	for (;;) {
	}
}

void prefetch_abort_handler()
{
	// struct exception_info info = {
	// 	.exception_name	       = "Prefetch Abort",
	// 	.exception_source_addr = frame ? frame->lr : 0U,
	// };
	// info.is_prefetch_abort			= true;
	// info.instruction_fault_status_register	= read_ifsr();
	// info.instruction_fault_address_register = read_ifar();

	// print_exception_infos(frame, &info);
	
	panic("Prefetch abort");
	for (;;) {
	}
}

void data_abort_handler()
{
	// struct exception_info info = {
	// 	.exception_name	       = "Data Abort",
	// 	.exception_source_addr = frame ? frame->lr : 0U,
	// };
	// info.is_data_abort		 = true;
	// info.data_fault_status_register	 = read_dfsr();
	// info.data_fault_address_register = read_dfar();

	// print_exception_infos(frame, &info);
	
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

static uint32_t read_dfsr(void)
{
	uint32_t value;
	__asm__ volatile("mrc p15, 0, %0, c5, c0, 0" : "=r"(value));
	return value;
}

static uint32_t read_dfar(void)
{
	uint32_t value;
	__asm__ volatile("mrc p15, 0, %0, c6, c0, 0" : "=r"(value));
	return value;
}

static uint32_t read_ifsr(void)
{
	uint32_t value;
	__asm__ volatile("mrc p15, 0, %0, c5, c0, 1" : "=r"(value));
	return value;
}

static uint32_t read_ifar(void)
{
	uint32_t value;
	__asm__ volatile("mrc p15, 0, %0, c6, c0, 2" : "=r"(value));
	return value;
}