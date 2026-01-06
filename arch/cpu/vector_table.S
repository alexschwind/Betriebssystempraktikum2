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

    cpsid if                       

    sub  sp, sp, #CTX_FRAME_SIZE
    stmia sp, {r0-r12}              

    cps   #0x1f                     
    mov   r3, sp                    
    mov   r4, lr                    
    cps   #\mode                    
    cpsid if                        

    str   r4, [sp, #(13*4)]         
    str   r3, [sp, #(14*4)]         
    str   lr, [sp, #(15*4)]        
    mrs   r3, spsr                 
    str   r3, [sp, #(16*4)]         

    
    mov   r0, sp                    
    bl    \handler                  

    ldr   r1, [sp, #(16*4)]
    msr   spsr_fsxc, r1

    
    ldr   r1, [sp, #(15*4)]
    mov   lr, r1

    ldr   r1, [sp, #(14*4)]         
    ldr   r2, [sp, #(13*4)]         
    cps   #0x1f
    mov   sp, r1                    
    mov   lr, r2                    
    cps   #\mode                    
    cpsid if                        

    
    ldmia sp, {r0-r12}

    
    add   sp, sp, #CTX_FRAME_SIZE 

    
    subs  pc, lr, #\ret_adj
.endm


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