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

static void panic(char* msg) __attribute__((noreturn));
static uint32_t read_dfsr(void);
static uint32_t read_dfar(void);
static uint32_t read_ifsr(void);
static uint32_t read_ifar(void);
static bool context_from_user_thread(const context_frame_t *ctx);
static void trigger_kernel_data_abort(void) __attribute__((noreturn));
static void trigger_kernel_prefetch_abort(void) __attribute__((noreturn));
static void trigger_kernel_svc(void) __attribute__((noreturn));
static void trigger_kernel_undef(void) __attribute__((noreturn));

static inline uint32_t decode_svc_number(context_frame_t *ctx)
{
	uint32_t svc_pc = ctx->lr - 4; // lr points to instruction after SVC
	return (*(uint32_t *)svc_pc) & 0x00FFFFFF;
}

context_frame_t *irq_handler(context_frame_t *old)
{
	bool should_do_context_switch = false;

	if (g_current && g_current->state == T_RUNNING) {
	    g_current->ctx = old;
	}

	if (irq_get_uart_pending()) {
		if (uart_get_rx_interrupt_status()) {
			uart_rx_into_buffer();
		}
		uart_clear_interrupt();
		char c;
		while (uart_getc_nonblocking(&c)) {
			switch (c) {
			case 'S':
				trigger_kernel_svc();
				break;
			case 'P':
				trigger_kernel_prefetch_abort();
				break;
			case 'A':
				trigger_kernel_data_abort();
				break;
			case 'U':
				trigger_kernel_undef();
				break;
			default:
				if (scheduler_thread_create(main, &c, sizeof(c))) {
					should_do_context_switch = true;
				}
				break;
			}
		}
	}

	if (irq_get_systimer_pending(1)) {
		systimer_clear_match(1);
		systimer_increment_compare(1, TIMER_INTERVAL);
		uart_putc('!');
		should_do_context_switch = true;
	}
	
	if (should_do_context_switch) {
		scheduler_pick_next();
	}

	return g_current ? g_current->ctx : old;
}

context_frame_t *svc_handler(context_frame_t *ctx)
{
	__asm__ volatile("cpsid i" ::: "memory");

	if (!context_from_user_thread(ctx)) {
		struct exception_info info = {
			.exception_name		 = "Kernel Supervisor Call",
			.exception_source_addr = ctx ? ctx->lr : 0U,
		};

		print_exception_infos(ctx, &info);
		panic("Kernel attempted SVC");
	}

	if (g_current) {
		g_current->ctx = ctx;
		g_current->state = T_UNUSED;
	}

	/* Later we can differentiate SVC numbers here
	uint32_t svc_no = decode_svc_number(ctx);
	switch (svc_no) {
		case SYSCALL_EXIT:
		...
	*/

	systimer_increment_compare(1, TIMER_INTERVAL);
	scheduler_pick_next();

	__asm__ volatile("cpsie i" ::: "memory");
	return g_current ? g_current->ctx : ctx;
}

context_frame_t *undefined_handler(context_frame_t *ctx)
{
	__asm__ volatile("cpsid i" ::: "memory");

	struct exception_info info = {
		.exception_name		 = "Undefined Instruction",
		.exception_source_addr = ctx ? ctx->lr : 0U,
	};

	print_exception_infos(ctx, &info);

	if (!context_from_user_thread(ctx)) {
		panic("Kernel undefined instruction");
	}

	if (g_current) {
		g_current->ctx = ctx;
		g_current->state = T_UNUSED;
	}
	
	systimer_increment_compare(1, TIMER_INTERVAL);
	scheduler_pick_next();

	__asm__ volatile("cpsie i" ::: "memory");
	return g_current ? g_current->ctx : ctx;
}

context_frame_t *prefetch_abort_handler(context_frame_t *ctx)
{
	__asm__ volatile("cpsid i" ::: "memory");

	struct exception_info info = {
		.exception_name		 = "Prefetch Abort",
		.exception_source_addr = ctx ? ctx->lr : 0U,
		.is_prefetch_abort			= true,
		.instruction_fault_status_register	= read_ifsr(),
		.instruction_fault_address_register = read_ifar(),
	};

	print_exception_infos(ctx, &info);

	if (!context_from_user_thread(ctx)) {
		panic("Kernel prefetch abort");
	}

	if (g_current) {
		g_current->ctx = ctx;
		g_current->state = T_UNUSED;
	}
	
	systimer_increment_compare(1, TIMER_INTERVAL);
	scheduler_pick_next();

	__asm__ volatile("cpsie i" ::: "memory");
	return g_current ? g_current->ctx : ctx;
}

context_frame_t *data_abort_handler(context_frame_t *ctx)
{
	__asm__ volatile("cpsid i" ::: "memory");

	struct exception_info info = {
		.exception_name		 = "Data Abort",
		.exception_source_addr = ctx ? ctx->lr : 0U,
		.is_data_abort		 = true,
		.data_fault_status_register = read_dfsr(),
		.data_fault_address_register = read_dfar(),
	};

	print_exception_infos(ctx, &info);

	if (!context_from_user_thread(ctx)) {
		panic("Kernel data abort");
	}

	if (g_current) {
		g_current->ctx = ctx;
		g_current->state = T_UNUSED;
	}
	
	systimer_increment_compare(1, TIMER_INTERVAL);
	scheduler_pick_next();

	__asm__ volatile("cpsie i" ::: "memory");
	return g_current ? g_current->ctx : ctx;
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

static bool context_from_user_thread(const context_frame_t *ctx)
{
	if (!ctx) {
		return false;
	}
	uint32_t mode = ctx->cpsr & 0x1Fu;
	return mode == 0x10u || mode == 0x1Fu; // TODO maybe only user mode
}

static void trigger_kernel_data_abort(void)
{
	volatile unsigned int *ptr = (volatile unsigned int *)0x00000001u;
	*ptr = 0xDEADBEEFu;
	__builtin_unreachable();
}

static void trigger_kernel_prefetch_abort(void)
{
	__asm__ volatile("bkpt #0" ::: "memory");
	__builtin_unreachable();
}

static void trigger_kernel_svc(void)
{
	__asm__ volatile("svc #1" ::: "memory");
	__builtin_unreachable();
}

static void trigger_kernel_undef(void)
{
	__asm__ volatile(".word 0xe7f000f0" ::: "memory");
	__builtin_unreachable();
}