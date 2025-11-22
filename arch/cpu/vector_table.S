.section .vectors, "ax"

.balign 64

.global _vector_table

.extern undefined_handler
.extern svc_handler
.extern prefetch_abort_handler
.extern data_abort_handler
.extern irq_handler

/* Makros für Prolog und Epilog von Handler */
.macro EXCEPTION_TRAMP label, handler, lr_adjust
.global \label
\label:
.if \lr_adjust
	sub lr, lr, #\lr_adjust
.endif
	stmdb sp!, {r0-r12, lr}
	mrs r1, spsr
	mrs r2, cpsr
	sub sp, sp, #8
	str r1, [sp]
	str r2, [sp, #4]
	mov r0, sp
	bl \handler
1:
	b 1b
.endm

.macro EXCEPTION_TRAMP_RETURN label, handler
.global \label
\label:
	stmdb sp!, {r0-r12, lr}
	mrs r1, spsr
	mrs r2, cpsr
	sub sp, sp, #8
	str r1, [sp]
	str r2, [sp, #4]
	mov r0, sp
	bl \handler
	ldr r1, [sp]
	msr spsr_cxsf, r1
	add sp, sp, #8
	ldmia sp!, {r0-r12, lr}
	subs pc, lr, #4
.endm

/* Tatsächliche Vector Table */
_vector_table:
	b _start                            /* Restart */
	ldr pc, _vector_undefined_ptr       /* Undefined instruction */
	ldr pc, _vector_svc_ptr             /* Service Call */
	ldr pc, _vector_prefetch_abort_ptr  /* Prefetch abort */
	ldr pc, _vector_data_abort_ptr      /* data abort */
	ldr pc, _vector_unused_ptr          /* Not used here */
	ldr pc, _vector_irq_ptr             /* IRQ interrupt */
	ldr pc, _vector_unused_ptr          /* Not used here */

_vector_unused_ptr: .word vector_unused

_vector_undefined_ptr: .word vector_undefined
_vector_svc_ptr: .word vector_svc
_vector_prefetch_abort_ptr: .word vector_prefetch_abort
_vector_data_abort_ptr: .word vector_data_abort
_vector_irq_ptr: .word vector_irq

EXCEPTION_TRAMP vector_undefined, undefined_handler, 4
EXCEPTION_TRAMP vector_svc, svc_handler, 4
EXCEPTION_TRAMP vector_prefetch_abort, prefetch_abort_handler, 4
EXCEPTION_TRAMP vector_data_abort, data_abort_handler, 8
EXCEPTION_TRAMP_RETURN vector_irq, irq_handler

vector_unused:
2:
	b 2b