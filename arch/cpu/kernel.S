.section .init

.extern _vector_table
.extern __stack_svc_top
.extern __stack_irq_top
.extern __stack_abt_top
.extern __stack_und_top
.extern __stack_sys_top

.global _bsprak
_bsprak:
    /* Stacks für alle Modi vorbereiten */
	/* Wir benutzen den FIQ und den User mode noch nicht */

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

	cps	#0x13                 /* Run kernel in supervisor mode */
	
	/* Exception Vector Table adresse setzen */
	ldr r0, =_vector_table
	mcr p15, 0, r0, c12, c0, 0

	
	bl  start_kernel /* This never returns */