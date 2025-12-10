#ifndef KERNEL_SYSCALL_H_
#define KERNEL_SYSCALL_H_

#include <stdint.h>

#define SYSCALL_EXIT 1u

static inline __attribute__((noreturn)) void syscall_exit(void)
{
	__asm__ volatile ("svc %[imm]" :: [imm] "I" (SYSCALL_EXIT));
	__builtin_unreachable();
}

#endif
