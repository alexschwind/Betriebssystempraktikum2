#include <kernel/scheduler.h>
#include <kernel/syscall.h>

#include <user/main.h>
#include <arch/bsp/systimer.h>

#include <lib/kprintf.h>
#include <config.h>

static tcb_t    g_threads[MAX_THREADS];
static uint8_t  g_stacks[MAX_THREADS][STACK_SIZE];

tcb_t *g_current = NULL;

static tcb_t *g_idle_tcb = NULL;

list_create(g_ready_queue);

static inline tcb_t *ready_node_owner(list_node *node)
{
    return (tcb_t *)((uint8_t *)node - offsetof(tcb_t, ready_node));
}

static void ready_queue_push_back(tcb_t *tcb)
{
    if (!tcb) {
        return;
    }

    list_add_last(g_ready_queue, &tcb->ready_node);
}

static tcb_t *ready_queue_pop_front(void)
{
    list_node *node = list_remove_first(g_ready_queue);
    if (!node) {
        return NULL;
    }

    return ready_node_owner(node);
}

static void idle_thread_fn(void)
{
    for (;;) {
        kprintf(".");
        asm volatile ("wfi");
    }
    __builtin_unreachable();
}

void scheduler_init(void)
{
    for (int i = 0; i < MAX_THREADS; ++i) {
        g_threads[i].state = T_UNUSED;
    }

    // Create idle thread
    g_idle_tcb = &g_threads[0];
    g_idle_tcb->state = T_RUNNING;
    g_idle_tcb->stack_base = g_stacks[0];
    g_idle_tcb->stack_top  = g_stacks[0] + STACK_SIZE;
    g_idle_tcb->ready_node = (list_node){0};

    uintptr_t sp = (uintptr_t)g_idle_tcb->stack_top;
    sp &= ~((uintptr_t)7); // 8-byte align

    sp -= sizeof(context_frame_t);
    context_frame_t *frame = (context_frame_t *)sp;

    for (int i = 0; i < (int)(sizeof(*frame)/sizeof(uint32_t)); ++i)
        ((uint32_t *)frame)[i] = 0;

    frame->lr   = (uint32_t)idle_thread_fn + 4;
    frame->cpsr = 0b10000; // user mode

    g_idle_tcb->ctx = frame;

    static char demo_chars[] = { 'A', 'B', 'C' };
    for (size_t i = 0; i < sizeof(demo_chars); ++i) {
        scheduler_thread_create(main, &demo_chars[i], sizeof(demo_chars[i]));
    }
}

void scheduler_pick_next(void)
{
    tcb_t *prev = g_current;

    // If the previous thread is still runnable, put it at the end of the queue
    if (prev && prev->state == T_RUNNING) {
        prev->state = T_READY;
        ready_queue_push_back(prev);
    }

    tcb_t *next = ready_queue_pop_front();
    if (!next) {
        // No ready threads; run idle
        next = g_idle_tcb;
    }

    next->state = T_RUNNING;
    g_current = next;
}

static tcb_t *alloc_tcb(void)
{
    for (int i = 0; i < MAX_THREADS; ++i) {
        if (g_threads[i].state == T_UNUSED) {
            tcb_t *t = &g_threads[i];
            t->state = T_ALLOCATED;
            t->stack_base = g_stacks[i];
            t->stack_top  = g_stacks[i] + STACK_SIZE;
            t->ready_node = (list_node){0};
            return t;
        }
    }
    return NULL; // no free slots
}

static void thread_trampoline(void (*func)(void *), void *arg)
{
    func(arg);
    syscall_exit();
}

// Called from kernel (SVC mode)
int scheduler_thread_create(void(* func)(void *), const void * arg, unsigned int arg_size)
{
    tcb_t *t = alloc_tcb();
    if (!t)
        return -1;

    // Start from top of stack (ARM stacks grow downward)
    uintptr_t sp = (uintptr_t)t->stack_top;

    // 8-byte align the stack (EABI requirement)
    sp &= ~((uintptr_t)7);

    // Reserve and initialize the initial context frame
    sp -= sizeof(context_frame_t);
    context_frame_t *frame = (context_frame_t *)sp;

    // Zero everything first
    for (int i = 0; i < (int)(sizeof(*frame)/sizeof(uint32_t)); ++i)
        ((uint32_t *)frame)[i] = 0;

    frame->r0   = (uint32_t)func;                 // function to run
    frame->r1   = (uint32_t)arg;                  // argument
    frame->r3   = (uint32_t)arg_size;             // argument size
    frame->lr   = (uint32_t)thread_trampoline + 4;

    // CPSR: user mode, interrupts enabled (bit pattern depends on your config)
    // User mode = 0b10000
    frame->cpsr = 0b10000;                      // + whatever flags you want set/cleared

    t->ctx   = frame;
    t->state = T_READY;

    ready_queue_push_back(t);
    return 0;
}

__attribute__((noreturn)) void scheduler_start(void) 
{
    // Pick first thread
    g_current = ready_queue_pop_front();
    if (!g_current)
        g_current = g_idle_tcb;

    g_current->state = T_RUNNING;

    // Enable interrupts here if needed

    systimer_increment_compare(1, TIMER_INTERVAL);
    scheduler_first_context_restore(g_current->ctx);
    __builtin_unreachable();
}