#include <kernel/scheduler.h>
#include <kernel/syscall.h>

#include <user/main.h>
#include <arch/bsp/systimer.h>

#include <lib/kprintf.h>
#include <config.h>
#include <string.h>

static tcb_t g_threads[MAX_THREADS];
extern uint8_t __thread_stack_pool_base[];
static unsigned int g_rr_cursor = 1;
tcb_t *g_current = NULL;
static tcb_t *g_idle_tcb = NULL;

static inline uint8_t *thread_stack_base(unsigned int idx)
{
    if (idx >= MAX_THREADS) {
        return NULL;
    }

    return __thread_stack_pool_base + ((size_t)idx * STACK_SIZE);
}

static inline uint8_t *thread_stack_top(unsigned int idx)
{
    uint8_t *base = thread_stack_base(idx);
    return base ? base + STACK_SIZE : NULL;
}





static void idle_thread_fn(void)
{
    for (;;) {
        asm volatile ("wfi");
    }
    __builtin_unreachable();
}

static void thread_trampoline(void (*func)(void *), void *arg)
{
    func(arg);
    syscall_exit();
}




void scheduler_init(void)
{
    memset(__thread_stack_pool_base, 0, STACK_SIZE * MAX_THREADS);

    for (int i = 0; i < MAX_THREADS; ++i) {
        g_threads[i].state = T_UNUSED;
        g_threads[i].stack_base = thread_stack_base(i);
        g_threads[i].stack_top  = thread_stack_top(i);
        memset(&g_threads[i].ctx_storage, 0, sizeof(context_frame_t));
    }

    // Create idle thread
    g_idle_tcb = &g_threads[0];
    g_idle_tcb->state = T_RUNNING;
    memset(g_idle_tcb->stack_top, 0, STACK_SIZE);

    g_idle_tcb->ctx_storage.sp_usr   = (uint32_t)g_idle_tcb->stack_top;
    g_idle_tcb->ctx_storage.lr_exc   = (uint32_t)idle_thread_fn + 4;
    g_idle_tcb->ctx_storage.cpsr_usr = 0b10000       // user mode
                                 | (1u << 6);    // FIQ disable
}

void scheduler_pick_next(void)
{
    tcb_t* prev = g_current;
    for (unsigned int scanned = 0; scanned < MAX_THREADS; ++scanned) {
        unsigned int idx = g_rr_cursor;
        g_rr_cursor = (g_rr_cursor + 1u) % MAX_THREADS;

        tcb_t *candidate = &g_threads[idx];
        if (candidate == g_idle_tcb) {
            continue;
        }

        if (candidate->state == T_RUNNING) {
            g_current = candidate;
            return;
        }
    }

    g_current = g_idle_tcb;
    if (prev != NULL && prev != g_current && g_current != g_idle_tcb) {
        uart_putc('\n');
    }
}

// Called from kernel (SVC mode)
bool scheduler_thread_create(void(* func)(void *), const void * arg, unsigned int arg_size)
{
    tcb_t* t = NULL;
    unsigned int temp_rr_cursor = g_rr_cursor;
    for (unsigned int scanned = 0; scanned < MAX_THREADS; ++scanned) {
        unsigned int idx = temp_rr_cursor;
        temp_rr_cursor = (temp_rr_cursor + 1u) % MAX_THREADS;

        tcb_t *candidate = &g_threads[idx];
        if (candidate == g_idle_tcb) {
            continue;
        }

        if (candidate->state == T_UNUSED) {
            t = candidate;
            break;
        }
    }
    if (!t) {
        kprintf("Could not create thread.");
        return false; // No free slots
    }

    memset(t->stack_top, 0, STACK_SIZE);

    // Start from top of stack (ARM stacks grow downward)
    uintptr_t sp = (uintptr_t)t->stack_top;

    uintptr_t arg_ptr = 0;
    if (arg && arg_size) {
        sp -= arg_size;
        sp &= ~((uintptr_t)3);
        arg_ptr = sp;
        memcpy((void *)arg_ptr, arg, arg_size);
    }
    
    memset(&t->ctx_storage, 0, sizeof(context_frame_t));
    t->ctx_storage.r0   = (uint32_t)func;                 // function to run
    t->ctx_storage.r1   = (uint32_t)arg_ptr;              // argument pointer (copied onto stack)
    t->ctx_storage.sp_usr   = (uint32_t)sp;
    t->ctx_storage.lr_exc   = (uint32_t)thread_trampoline + 4;
    t->ctx_storage.cpsr_usr = 0b10000       // user mode
                            | (1u << 6);    // FIQ disable
    t->state = T_RUNNING;

    return true;
}

__attribute__((noreturn)) void scheduler_start(void) 
{
    scheduler_pick_next();
    systimer_increment_compare(1, TIMER_INTERVAL);
    scheduler_first_context_restore(&g_current->ctx_storage);
    __builtin_unreachable();
}