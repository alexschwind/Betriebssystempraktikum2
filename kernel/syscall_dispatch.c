#include <kernel/syscall_dispatch.h>

#include <arch/bsp/uart.h>
#include <kernel/scheduler.h>
#include <syscall.h>

#include <config.h>

#include <stdbool.h>
#include <stddef.h>

syscall_result_t make_result(uint32_t value, bool reschedule, bool advance_pc)
{
	syscall_result_t result = {
		.value = value,
		.handled = true,
		.reschedule = reschedule,
		.advance_pc = advance_pc,
	};
	return result;
}

syscall_result_t make_unhandled(void)
{
	syscall_result_t result = {
		.value = 0u,
		.handled = false,
		.reschedule = true,
		.advance_pc = false,
	};
	return result;
}

static syscall_result_t handle_exit(void)
{
	if (g_current) {
		g_current->state = T_UNUSED;
	}
	return make_result(0u, true, false);
}

static syscall_result_t handle_putc(const context_frame_t *ctx)
{
	char c = (char)(ctx->r1 & 0xFFu);
	uart_putc(c);
	return make_result(0u, false, true);
}

static syscall_result_t handle_getc(void)
{
	char c;
	if (uart_getc_nonblocking(&c)) {
		return make_result((uint32_t)(uint8_t)c, false, true);
	}

	if (!scheduler_block_current_on_input()) {
		return make_unhandled();
	}

	return make_result(0u, true, true);
}

static syscall_result_t handle_create_thread(const context_frame_t *ctx)
{
	void (*func)(void *) = (void (*)(void *))ctx->r1;
	void *arg = (void *)ctx->r2;
	unsigned int arg_size = (unsigned int)ctx->r3;
	bool created = scheduler_thread_create(func, arg, arg_size);
	return make_result(created ? 0u : 1u, false, true);
}

static syscall_result_t handle_sleep(const context_frame_t *ctx)
{
	unsigned int cycles = (unsigned int)ctx->r1;
	scheduler_sleep_current(cycles);
	return make_result(0u, true, true);
}

syscall_result_t syscall_dispatch(context_frame_t *ctx)
{
	if (!ctx) {
		return make_unhandled();
	}

	switch ((syscall_id_t)ctx->r0) {
	case SYSCALL_ID_EXIT:
		return handle_exit();
	case SYSCALL_ID_PUTC:
		return handle_putc(ctx);
	case SYSCALL_ID_GETC:
		return handle_getc();
	case SYSCALL_ID_CREATE_THREAD:
		return handle_create_thread(ctx);
	case SYSCALL_ID_SLEEP:
		return handle_sleep(ctx);
	case SYSCALL_ID_UNDEFINED:
	default:
		return make_unhandled();
	}
}
