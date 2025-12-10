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
    @ In exception mode here
    cpsid if                        @ Disable IRQ and FIQ in this mode

    @ ------ IRQ entry: save context of current thread ------

    sub  sp, sp, #CTX_FRAME_SIZE
    stmia sp, {r0-r12}              @ save r0-r12 onto exception stack

    @ Save user/system SP into slot 13
    cps   #0x1f                     @ System mode (shares SP with User)
    mov   r3, sp                    @ r3 = user/system SP
    cps   #\mode                    @ back to exception mode
    cpsid if                        @ Disable IRQ and FIQ in this mode

    str   r3, [sp, #(13*4)]         @ [sp + 52] = user SP
    str   lr, [sp, #(14*4)]         @ [sp + 56] = exception LR (lr_irq)
    mrs   r3, spsr                  @ SPSR_irq = user CPSR
    str   r3, [sp, #(15*4)]         @ [sp + 60] = CPSR of user/system

    @ sp points to full context frame of OLD thread
    mov   r0, sp                    @ r0 = pointer to context frame on IRQ stack
    bl    \handler                  @ C helper: sees context_t *ctx = (context_t *)r0

    @ ------ IRQ exit: restore context of NEW thread ------

    @ irq_sp now points to saved context frame of NEW thread 

    @ Load CPSR for new thread (saved in slot 15) into SPSR_irq
    ldr   r1, [sp, #(15*4)]
    msr   spsr_fsxc, r1

    @ Load exception LR (slot 14) into lr
    ldr   r1, [sp, #(14*4)]
    mov   lr, r1

    @ Restore user/system SP (slot 13)
    ldr   r1, [sp, #(13*4)]
    cps   #0x1f
    mov   sp, r1                    @ new thread's user/system SP
    cps   #\mode                    @ back to IRQ mode, SP = SP_irq again
    cpsid if                        @ Disable IRQ and FIQ in this mode

    @ Now restore r0-r12 from slots 0..12
    ldmia sp, {r0-r12}

    @ Free the frame from IRQ stack
    add   sp, sp, #CTX_FRAME_SIZE 

    @ Return from IRQ into new thread context
    subs  pc, lr, #\ret_adj
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