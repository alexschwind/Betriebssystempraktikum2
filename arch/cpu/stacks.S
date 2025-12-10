#include <kernel/scheduler.h>

.section .stacks, "aw", %nobits

.global __stack_sys_base
.global __stack_sys_top
.global __stack_svc_base
.global __stack_svc_top
.global __stack_irq_base
.global __stack_irq_top
.global __stack_abt_base
.global __stack_abt_top
.global __stack_und_base
.global __stack_und_top

.balign 64
__stack_sys_base:
.space 1024
.balign 8
__stack_sys_top:
.balign 8
__stack_svc_base:
.space 1024
.balign 8
__stack_svc_top:
.balign 8
__stack_irq_base:
.space 1024
.balign 8
__stack_irq_top:
.balign 8
__stack_abt_base:
.space 1024
.balign 8
__stack_abt_top:
.balign 8
__stack_und_base:
.space 1024
.balign 8
__stack_und_top:
.balign 64

.global __thread_stack_pool_base
.global __thread_stack_pool_end

__thread_stack_pool_base:
.space STACK_SIZE * MAX_THREADS
.balign 8
__thread_stack_pool_end: