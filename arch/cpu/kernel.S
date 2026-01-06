.section .init
.extern _vector_table
.extern _stack_svc_top
.extern _stack_irq_top
.extern _stack_abt_top
.extern _stack_und_top
.extern _stack_sys_top

#include <kernel/scheduler.h>

.global _bsprak
_bsprak:
	cpsid if                  

	cps	#0x13                 
	ldr sp, =_stack_svc_top
	cps	#0x12                 
	ldr sp, =_stack_irq_top
	cps	#0x17                 
	ldr sp, =_stack_abt_top
	cps	#0x1b                 
	ldr sp, =_stack_und_top
	cps	#0x1f                
	ldr sp, =_stack_sys_top
	
	ldr r0, =_vector_table
	mcr p15, 0, r0, c12, c0, 0

	cps	#0x13        
	bl  start_kernel 

.global scheduler_first_context_restore
scheduler_first_context_restore:
	cpsid if

	sub     sp, sp, #CTX_FRAME_SIZE
	mov     r1, r0             
    mov     r0, sp             
    mov     r2, #CTX_FRAME_SIZE
    bl      memcpy

	ldr   r1, [sp, #(16*4)]
    msr   spsr_fsxc, r1

	ldr   r1, [sp, #(15*4)]
    mov   lr, r1

	ldr   r1, [sp, #(14*4)]         
	ldr   r2, [sp, #(13*4)]         
	cps   #0x1f
	mov   sp, r1                    
	mov   lr, r2                    
	cps   #0x13                  

    ldmia sp, {r0-r12}

    add   sp, sp, #CTX_FRAME_SIZE 

    subs  pc, lr, #4