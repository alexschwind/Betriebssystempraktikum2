.section .vectors, "ax"
.balign 64
.global _vector_table
.extern undefined_handler
.extern svc_handler
.extern prefetch_abort_handler
.extern data_abort_handler
.extern irq_handler

#include <kernel/scheduler.h>

/* Tatsächliche Vector Table */
_vector_table:
	b _start                            /* Restart */
	b undefined_handler                 /* Undefined instruction */
    b svc_handler_asm                   /* Service Call */
	b prefetch_abort_handler            /* Prefetch abort */
	b data_abort_handler                /* data abort */
	nop                                 /* Not used here */
	b irq_handler_asm                   /* IRQ interrupt */
	nop                                 /* Not used here */

irq_handler_asm:
	sub sp, sp, #CTX_FRAME_SIZE
    @ Now SP points to reserved space for context_frame_t

    @ Store general registers
    stmia sp, {r0-r12, lr}     @ r0 at offset 0, lr at offset 13*4

    @ Save CPSR of the interrupted mode
    mrs r1, spsr
    str r1, [sp, #(14*4)]     @ cpsr at offset 14*4

    @ Set up argument to C handler: r0 = sp (context_frame_t *)
    mov r0, sp
    bl irq_handler           @ C: context_frame_t *irq_handler_c(context_frame_t *old);

    @ C returns new SP (= pointer to new context_frame_t)
    mov sp, r0

    @ Restore CPSR into SPSR
    ldr r1, [sp, #(14*4)]
    msr spsr_fsxc, r1

    @ Restore general registers + LR
    ldmia sp, {r0-r12, lr}

    add sp, sp, #CTX_FRAME_SIZE

    @ Exception return, back to user mode
    subs pc, lr, #4

svc_handler_asm:
    sub sp, sp, #CTX_FRAME_SIZE

    stmia sp, {r0-r12, lr}

    mrs r1, spsr
    str r1, [sp, #(14*4)]

    mov r0, sp
    bl svc_handler

    mov sp, r0

    ldr r1, [sp, #(14*4)]
    msr spsr_fsxc, r1

    ldmia sp, {r0-r12, lr}

    add sp, sp, #CTX_FRAME_SIZE

    subs pc, lr, #4