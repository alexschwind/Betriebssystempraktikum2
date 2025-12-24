#include <kernel/scheduler.h>

#include <syscall.h>

#include <arch/bsp/systimer.h>
#include <arch/bsp/uart.h>

#include <lib/kprintf.h>
#include <config.h>
#include <string.h>
#include <stddef.h>

#include <lib/list.h>

extern void main(void) __attribute__((weak));

static tcb_t g_threads[MAX_THREADS];
extern uint8_t __thread_stack_pool_base[];
static unsigned int g_rr_cursor = 1;
tcb_t *g_current = NULL;
static tcb_t *g_idle_tcb = NULL;
static list_node g_getc_wait_list_head = { &g_getc_wait_list_head, &g_getc_wait_list_head };

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
static inline void list_node_init(list_node *node)
{
    node->next = node;
    node->prev = node;
}

static inline tcb_t *tcb_from_wait_node(list_node *node)
{
    return (tcb_t *)((char *)node - offsetof(tcb_t, wait_node));
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

static void user_main_entry(void *unused)
{
    (void)unused;
    if (main) {
        main();
    }
}

static void create_initial_user_thread(void)
{
    if (!main) {
        kprintf("Failed to create initial user thread because main is not defined.\n");
        return;
    }

    if (!scheduler_thread_create(user_main_entry, NULL, 0u)) {
        kprintf("Failed to create initial user thread.\n");
    }
}




void scheduler_init(void)
{
    memset(__thread_stack_pool_base, 0, STACK_SIZE * MAX_THREADS);

    for (int i = 0; i < MAX_THREADS; ++i) {
        g_threads[i].state = T_UNUSED;
        g_threads[i].stack_base = thread_stack_base(i);
        g_threads[i].stack_top  = thread_stack_top(i);
        memset(&g_threads[i].ctx_storage, 0, sizeof(context_frame_t));
        g_threads[i].sleep_ticks = 0u;
        list_node_init(&g_threads[i].wait_node);
    }

    // Create idle thread
    g_idle_tcb = &g_threads[0];
    g_idle_tcb->state = T_RUNNING;
    memset(g_idle_tcb->stack_top, 0, STACK_SIZE);
    g_idle_tcb->sleep_ticks = 0u;
    list_node_init(&g_idle_tcb->wait_node);

    g_idle_tcb->ctx_storage.sp_usr   = (uint32_t)g_idle_tcb->stack_top;
    g_idle_tcb->ctx_storage.lr_exc   = (uint32_t)idle_thread_fn + 4;
    g_idle_tcb->ctx_storage.cpsr_usr = 0b10000       // user mode
                                 | (1u << 6);    // FIQ disable

    create_initial_user_thread();
}

void scheduler_pick_next(void)
{
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

    if (arg_size > STACK_SIZE) {
        kprintf("Thread argument block too large.\n");
        return false;
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
    t->sleep_ticks = 0u;
    list_node_init(&t->wait_node);

    return true;
}

void scheduler_sleep_current(uint32_t ticks)
{
    if (!g_current || g_current == g_idle_tcb) {
        return;
    }

    if (ticks == 0u) {
        ticks = 1u;
    }

    g_current->sleep_ticks = ticks;
    g_current->state = T_SLEEPING;
}

void scheduler_tick(void)
{
    for (unsigned int i = 0; i < MAX_THREADS; ++i) {
        tcb_t *thread = &g_threads[i];
        if (thread == g_idle_tcb) {
            continue;
        }

        if (thread->state == T_SLEEPING && thread->sleep_ticks > 0u) {
            thread->sleep_ticks--;
            if (thread->sleep_ticks == 0u) {
                thread->state = T_RUNNING;
            }
        }
    }
}

bool scheduler_block_current_on_input(void)
{
    if (!g_current || g_current == g_idle_tcb) {
        return false;
    }

    if (g_current->state != T_WAITING_IO) {
        g_current->state = T_WAITING_IO;
        list_add_last(&g_getc_wait_list_head, &g_current->wait_node);
    }

    return true;
}

bool scheduler_has_waiting_input(void)
{
    return !list_is_empty(&g_getc_wait_list_head);
}

tcb_t *scheduler_pop_next_input_waiter(void)
{
    list_node *node = list_remove_first(&g_getc_wait_list_head);
    if (!node) {
        return NULL;
    }

    tcb_t *thread = tcb_from_wait_node(node);
    list_node_init(node);
    thread->sleep_ticks = 0u;
    thread->state = T_RUNNING;
    return thread;
}

__attribute__((noreturn)) void scheduler_start(void) 
{
    scheduler_pick_next();
    systimer_increment_compare(1, TIMER_INTERVAL);
    scheduler_first_context_restore(&g_current->ctx_storage);
    __builtin_unreachable();
}