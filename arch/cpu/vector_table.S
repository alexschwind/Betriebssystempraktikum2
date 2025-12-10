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
    cps #0x1f                  @ switch to system mode (user stack)
    sub sp, sp, #CTX_FRAME_SIZE @ make space for context frame
    stmia sp, {r0-r12}         @ save general registers
    mov r2, sp                 @ r2 points to the context frame on user stack
    cps #0x12                  @ back to IRQ mode
    str lr, [r2, #(13*4)]      @ save exception LR (PC + 4)
    mrs r3, spsr
    str r3, [r2, #(14*4)]      @ save user CPSR

    mov r0, r2                 @ pass frame pointer to C handler
    bl irq_handler

    mov r2, r0                 @ r2 = next context frame pointer

    cps #0x1f
    mov sp, r2                 @ load user stack pointer for next thread
    cps #0x12

    ldr r1, [r2, #(13*4)]      @ load saved exception LR
    ldr r0, [r2, #(14*4)]      @ load saved CPSR
    msr spsr_fsxc, r0
    mov lr, r1

    cps #0x1f
    ldmia sp!, {r0-r12}        @ restore general registers
    add sp, sp, #8             @ drop saved LR/CPSR, pop frame
    cps #0x12

    subs pc, lr, #4

svc_handler_asm:
    cps #0x1f
    sub sp, sp, #CTX_FRAME_SIZE
    stmia sp, {r0-r12}         @ save general registers
    mov r2, sp                 @ r2 points to the context frame on user stack
    cps #0x13
    str lr, [r2, #(13*4)]
    mrs r3, spsr
    str r3, [r2, #(14*4)]

    mov r0, r2
    bl svc_handler

    mov r2, r0

    cps #0x1f
    mov sp, r2
    cps #0x13

    ldr r1, [r2, #(13*4)]
    ldr r0, [r2, #(14*4)]
    msr spsr_fsxc, r0
    mov lr, r1

    cps #0x1f
    ldmia sp!, {r0-r12}
    add sp, sp, #8
    cps #0x13

    subs pc, lr, #4