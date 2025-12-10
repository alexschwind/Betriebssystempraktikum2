.section .vectors, "ax"
.balign 64
.global _vector_table
.extern undefined_handler
.extern svc_handler
.extern prefetch_abort_handler
.extern data_abort_handler
.extern irq_handler

#include <kernel/scheduler.h>

.macro DEFINE_CONTEXT_HANDLER name, mode, handler, ret_adj
\name:
    cps #0x1f                     @ switch to system mode (user stack)
    sub sp, sp, #CTX_FRAME_SIZE   @ make space for context frame
    stmia sp, {r0-r12}            @ save r0-r12 on user stack
    mov r2, sp                    @ r2 points to the context frame on user stack
    cps #\mode                    @ switch to target mode
    str lr, [r2, #(13*4)]         @ save lr on context frame
    mrs r3, spsr
    str r3, [r2, #(14*4)]         @ save spsr on context frame

    mov r0, r2
    bl \handler

    mov r2, r0

    cps #0x1f
    mov sp, r2                    @ restore user stack pointer
    cps #\mode

    ldr r1, [r2, #(13*4)]         @ load lr from context frame
    ldr r0, [r2, #(14*4)]         @ load spsr from context frame
    msr spsr_fsxc, r0
    mov lr, r1

    cps #0x1f
    ldmia sp!, {r0-r12}
    add sp, sp, #8
    cps #\mode

    subs pc, lr, #\ret_adj
.endm

/* Tatsächliche Vector Table */
_vector_table:
	b _start                            /* Restart */
	b undefined_handler_asm             /* Undefined instruction */
    b svc_handler_asm                   /* Service Call */
	b prefetch_abort_handler_asm        /* Prefetch abort */
    b data_abort_handler_asm            /* data abort */
	nop                                 /* Not used here */
	b irq_handler_asm                   /* IRQ interrupt */
	nop                                 /* Not used here */

DEFINE_CONTEXT_HANDLER irq_handler_asm, 0x12, irq_handler, 4
DEFINE_CONTEXT_HANDLER svc_handler_asm, 0x13, svc_handler, 4
DEFINE_CONTEXT_HANDLER data_abort_handler_asm, 0x17, data_abort_handler, 4
DEFINE_CONTEXT_HANDLER prefetch_abort_handler_asm, 0x17, prefetch_abort_handler, 4
DEFINE_CONTEXT_HANDLER undefined_handler_asm, 0x1b, undefined_handler, 4