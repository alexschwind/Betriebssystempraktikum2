#ifndef SYSCALL_H_
#define SYSCALL_H_

#include <stdint.h>

enum syscall_id {
    SYSCALL_ID_EXIT = 0u,
    SYSCALL_ID_PUTC = 1u,
    SYSCALL_ID_GETC = 2u,
    SYSCALL_ID_CREATE_THREAD = 3u,
    SYSCALL_ID_SLEEP = 4u,
    SYSCALL_ID_UNDEFINED = 5u,
};

typedef enum syscall_id syscall_id_t;

static inline uint32_t syscall_invoke(syscall_id_t id, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    register uint32_t r0 __asm__("r0") = (uint32_t)id;
    register uint32_t r1 __asm__("r1") = arg1;
    register uint32_t r2 __asm__("r2") = arg2;
    register uint32_t r3 __asm__("r3") = arg3;

    __asm__ volatile ("svc #0"
                      : "+r"(r0)
                      : "r"(r1), "r"(r2), "r"(r3)
                      : "memory");

    return r0;
}

static inline __attribute__((noreturn)) void syscall_exit(void)
{
    (void)syscall_invoke(SYSCALL_ID_EXIT, 0u, 0u, 0u);
    __builtin_unreachable();
}

static inline void syscall_putc(char c)
{
    (void)syscall_invoke(SYSCALL_ID_PUTC, (uint32_t)c, 0u, 0u);
}

static inline char syscall_getc(void)
{
    return (char)syscall_invoke(SYSCALL_ID_GETC, 0u, 0u, 0u);
}

static inline void syscall_create_thread(void (*func)(void *), void *args, unsigned int arg_size)
{
    (void)syscall_invoke(SYSCALL_ID_CREATE_THREAD,
                         (uint32_t)func,
                         (uint32_t)args,
                         (uint32_t)arg_size);
}

static inline void syscall_sleep(unsigned int cycles)
{
    (void)syscall_invoke(SYSCALL_ID_SLEEP, cycles, 0u, 0u);
}

static inline void syscall_undefined(void)
{
    (void)syscall_invoke(SYSCALL_ID_UNDEFINED, 0u, 0u, 0u);
}

#endif
