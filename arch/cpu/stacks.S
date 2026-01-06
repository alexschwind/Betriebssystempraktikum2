#include <kernel/scheduler.h>

.section .stacks, "aw", %nobits

.global _stack_sys_base
.global _stack_sys_top
.global _stack_svc_base
.global _stack_svc_top
.global _stack_irq_base
.global _stack_irq_top
.global _stack_abt_base
.global _stack_abt_top
.global _stack_und_base
.global _stack_und_top

.balign 64
_stack_sys_base:
.space 1024
.balign 8
_stack_sys_top:
.balign 8
_stack_svc_base:
.space 1024
.balign 8
_stack_svc_top:
.balign 8
_stack_irq_base:
.space 1024
.balign 8
_stack_irq_top:
.balign 8
_stack_abt_base:
.space 1024
.balign 8
_stack_abt_top:
.balign 8
_stack_und_base:
.space 1024
.balign 8
_stack_und_top:
.balign 64

.global _thread_stack_pool_base
.global _thread_stack_pool_end

_thread_stack_pool_base:
.space STACK_SIZE * MAX_THREADS
.balign 8
_thread_stack_pool_end: