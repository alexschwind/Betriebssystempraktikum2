#ifndef KERNEL_SYSCALL_H_
#define KERNEL_SYSCALL_H_

#include <stdint.h>

#define SYSCALL_EXIT 1u

static inline __attribute__((noreturn)) void syscall_exit(void)
{
    // Typically: call into kernel via SVC
    // e.g., trigger SVC 0, and in the SVC handler:
    //  - mark g_current->state = T_ZOMBIE
    //  - scheduler_pick_next()
    //  - never return to this thread
	__asm__ volatile ("svc %[imm]" :: [imm] "I" (SYSCALL_EXIT));
	__builtin_unreachable();
}

#endif
