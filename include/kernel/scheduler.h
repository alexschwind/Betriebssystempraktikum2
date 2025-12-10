#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#define MAX_THREADS 32
#define STACK_SIZE  2048
#define CTX_FRAME_SIZE  (17 * 4)

#ifndef __ASSEMBLER__

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef enum {
    T_UNUSED = 0,
    T_RUNNING,
} thread_state_t;

typedef struct context_frame {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t r12;
    uint32_t lr_usr;    // user-mode LR (r14)
    uint32_t sp_usr;    // user-mode SP (r13)
    uint32_t lr_exc;    // exception LR = user PC + 4
    uint32_t cpsr_usr;  // user-mode CPSR
} context_frame_t;

typedef struct tcb {
    context_frame_t    ctx_storage;
    thread_state_t     state;
    uint8_t*           stack_base;
    uint8_t*           stack_top;
} tcb_t;

extern tcb_t* g_current;

extern void scheduler_first_context_restore(context_frame_t *ctx);

void scheduler_pick_next(void);
bool scheduler_thread_create(void(* func)(void *), const void * arg, unsigned int arg_size);
void scheduler_init(void);
__attribute__((noreturn)) void scheduler_start(void);

#endif /* __ASSEMBLER__ */

#endif