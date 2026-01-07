#include <kernel/handlers.h>
#include <kernel/scheduler.h>
#include <kernel/syscall_dispatch.h>

#include <lib/kprintf.h>

#include <arch/bsp/uart.h>
#include <arch/bsp/irq.h>
#include <arch/bsp/systimer.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <config.h>

#include <syscall.h>

#include <lib/exception_print.h>

#define MODE_MASK 0x1Fu
#define USER_MODE_BITS 0x10u
#define SYSTEM_MODE_BITS 0x1Fu

static void panic(void) __attribute__((noreturn));
static uint32_t read_dfsr(void);
static uint32_t read_dfar(void);
static uint32_t read_ifsr(void);
static uint32_t read_ifar(void);
static bool is_user_thread(const context_frame_t *ctx);

static context_frame_t *current_ctx_storage(void)
{
	return g_current ? &g_current->ctx_storage : NULL;
}

static void save_current_context(context_frame_t *ctx)
{
	context_frame_t *dst = current_ctx_storage();
	if (dst && ctx) {
		memcpy(dst, ctx, sizeof(context_frame_t));
	}
}

static void restore_current_context(context_frame_t *ctx)
{
	context_frame_t *src = current_ctx_storage();
	if (src && ctx) {
		memcpy(ctx, src, sizeof(context_frame_t));
	}
}

static context_frame_t *report_context(context_frame_t *fallback)
{
	context_frame_t *ctx = current_ctx_storage();
	return ctx ? ctx : fallback;
}

void irq_handler(context_frame_t *ctx)
{
	save_current_context(ctx);

	if (irq_get_uart_pending()) {
		if (uart_get_rx_interrupt_status()) {
			while (uart_rx_data_available_and_buffer_not_full()) {
				char c = uart_rx_get_char();
				if (c == 'S') {
					syscall_exit();
					continue;
				}
				if (!uart_buffer_putc(c)) {
					break;
				}
			}
		}

		while (scheduler_has_waiting_input()) {
			char available;
			if (!uart_peekc(&available)) {
				break;
			}

			tcb_t *waiter = scheduler_pop_next_input_waiter();
			if (!waiter) {
				break;
			}

			char delivered;
			if (!uart_getc_nonblocking(&delivered)) {
				break;
			}

			waiter->ctx_storage.r0 = (uint32_t)(uint8_t)delivered;
		}
	}

	if (irq_get_systimer_pending(1)) {
		systimer_clear_match(1);
		systimer_increment_compare(1, TIMER_INTERVAL);
		scheduler_tick();
		scheduler_pick_next();
	}

	restore_current_context(ctx);
}

void svc_handler(context_frame_t *ctx)
{
	__asm__ volatile("cpsid i" ::: "memory");
	save_current_context(ctx);

	context_frame_t *fault_ctx = report_context(ctx);
	if (!is_user_thread(fault_ctx)) {
		struct exception_info info = {
			.exception_name		 = "Kernel Supervisor Call",
			.exception_source_addr = fault_ctx ? fault_ctx->lr_exc : 0U,
		};

		print_exception_infos(fault_ctx, &info);
		panic();
	}

	syscall_result_t result = syscall_dispatch(ctx);
	context_frame_t *stored_ctx = current_ctx_storage();
	if (stored_ctx) {
		if (result.handled) {
			stored_ctx->r0 = result.value;
		}
		if (result.advance_pc) {
			stored_ctx->lr_exc += 4u;
		}
	}

	if (!result.handled) {
		struct exception_info info = {
			.exception_name		 = "Unknown Syscall",
			.exception_source_addr = fault_ctx ? fault_ctx->lr_exc - 4u : 0u,
		};

		print_exception_infos(fault_ctx, &info);
		if (g_current) {
			g_current->state = T_UNUSED;
		}
		result.reschedule = true;
	}

	if (result.reschedule) {
		systimer_increment_compare(1, TIMER_INTERVAL);
		scheduler_pick_next();
	}

	restore_current_context(ctx);
	__asm__ volatile("cpsie i" ::: "memory");
}

void undefined_handler(context_frame_t *ctx)
{
	__asm__ volatile("cpsid i" ::: "memory");
	save_current_context(ctx);

	context_frame_t *fault_ctx = report_context(ctx);
	struct exception_info info = {
		.exception_name		 = "Undefined Instruction",
		.exception_source_addr = fault_ctx ? fault_ctx->lr_exc : 0U,
	};

	print_exception_infos(fault_ctx, &info);

	if (!is_user_thread(fault_ctx)) {
		panic();
	}

	if (g_current) {
		g_current->state = T_UNUSED;
	}
	
	systimer_increment_compare(1, TIMER_INTERVAL);
	scheduler_pick_next();

	restore_current_context(ctx);
	__asm__ volatile("cpsie i" ::: "memory");
}

void prefetch_abort_handler(context_frame_t *ctx)
{
	__asm__ volatile("cpsid i" ::: "memory");
	save_current_context(ctx);

	context_frame_t *fault_ctx = report_context(ctx);
	struct exception_info info = {
		.exception_name		 = "Prefetch Abort",
		.exception_source_addr = fault_ctx ? fault_ctx->lr_exc : 0U,
		.is_prefetch_abort			= true,
		.instruction_fault_status_register	= read_ifsr(),
		.instruction_fault_address_register = read_ifar(),
	};

	print_exception_infos(fault_ctx, &info);

	if (!is_user_thread(fault_ctx)) {
		panic();
	}

	if (g_current) {
		g_current->state = T_UNUSED;
	}
	
	systimer_increment_compare(1, TIMER_INTERVAL);
	scheduler_pick_next();

	restore_current_context(ctx);
	__asm__ volatile("cpsie i" ::: "memory");
}

void data_abort_handler(context_frame_t *ctx)
{
	__asm__ volatile("cpsid i" ::: "memory");
	save_current_context(ctx);

	context_frame_t *fault_ctx = report_context(ctx);
	struct exception_info info = {
		.exception_name		 = "Data Abort",
		.exception_source_addr = fault_ctx ? fault_ctx->lr_exc : 0U,
		.is_data_abort		 = true,
		.data_fault_status_register = read_dfsr(),
		.data_fault_address_register = read_dfar(),
	};

	print_exception_infos(fault_ctx, &info);

	if (!is_user_thread(fault_ctx)) {
		panic();
	}

	if (g_current) {
		g_current->state = T_UNUSED;
	}
	
	systimer_increment_compare(1, TIMER_INTERVAL);
	scheduler_pick_next();

	restore_current_context(ctx);
	__asm__ volatile("cpsie i" ::: "memory");
}

__attribute__((noreturn)) static void panic(void)	
{
	__asm__ volatile("cpsid if" : : : "memory");

	uart_putc('\4');

	for (;;) {
		__asm__ volatile("wfi" ::: "memory");
	}
	__builtin_unreachable();
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

static bool is_user_thread(const context_frame_t *ctx)
{
	if (!ctx) {
		return false;
	}
	uint32_t mode = ctx->cpsr_usr & MODE_MASK;
	return mode == USER_MODE_BITS || mode == SYSTEM_MODE_BITS;
}