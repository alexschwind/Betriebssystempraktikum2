.section .init
.extern _vector_table
.extern __stack_svc_top
.extern __stack_irq_top
.extern __stack_abt_top
.extern __stack_und_top
.extern __stack_sys_top

#include <kernel/scheduler.h>

.global _bsprak
_bsprak:
    /* Stacks für alle Modi vorbereiten */
	/* Wir benutzen den FIQ noch nicht */
	cpsid if                  /* Disable IRQ and FIQ */

	cps	#0x13                 /* Switch to supervisor mode */
	ldr sp, =__stack_svc_top
	cps	#0x12                 /* Switch to IRQ mode */
	ldr sp, =__stack_irq_top
	cps	#0x17                 /* Switch to ABT mode */
	ldr sp, =__stack_abt_top
	cps	#0x1b                 /* Switch to UND mode */
	ldr sp, =__stack_und_top
	cps	#0x1f                 /* Switch to system mode */
	ldr sp, =__stack_sys_top
	
	/* Exception Vector Table adresse setzen */
	ldr r0, =_vector_table
	mcr p15, 0, r0, c12, c0, 0

	cps	#0x13        /* Run kernel in supervisor mode */
	bl  start_kernel /* This never returns */

.global scheduler_first_context_restore
@ r0 = ctx (pointer to context_frame_t)
@ must be called from SVC mode
scheduler_first_context_restore:
	mov     r2, r0

	@ Point system-mode SP to the saved user context frame
	cps     #0x1f
	mov     sp, r2
	cps     #0x13

	@ Load return LR and CPSR for the target thread
	ldr     r1, [r2, #(13*4)]
	ldr     r0, [r2, #(14*4)]
	msr     spsr_fsxc, r0
	mov     lr, r1

	@ Restore general-purpose registers from the user stack
	cps     #0x1f
	ldmia   sp!, {r0-r12}
	add     sp, sp, #8 @ drop saved LR/CPSR, pop frame
	cps     #0x13

	@ Return to user mode as if from exception
	subs    pc, lr, #4 