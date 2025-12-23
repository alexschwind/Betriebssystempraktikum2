#ifndef KERNEL_SYSCALL_DISPATCH_H_
#define KERNEL_SYSCALL_DISPATCH_H_

#include <stdbool.h>
#include <stdint.h>

#include <kernel/scheduler.h>

typedef struct syscall_result {
	uint32_t value;
	bool handled;
	bool reschedule;
	bool advance_pc;
} syscall_result_t;

syscall_result_t syscall_dispatch(context_frame_t *ctx);

#endif
