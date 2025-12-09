#include <config.h>

#include <arch/bsp/uart.h>
#include <arch/bsp/systimer.h>
#include <arch/bsp/irq.h>

#include <kernel/handlers.h>
#include <kernel/scheduler.h>

#include <lib/kprintf.h>

#include <tests/regcheck.h>

#include <stdbool.h>

static void do_data_abort(void);
static void do_prefetch_abort(void);
static void do_supervisor_call(void);
static void do_undefined_inst(void);

void start_kernel [[noreturn]] (void);
void start_kernel [[noreturn]] (void)
{
	// called from kernel.S
	// interrupts are disabled
	// in supervisor mode

	uart_init();
	uart_enable_rx_interrupt();
	irq_enable_uart();
	irq_enable_systimer(1);

	kprintf("=== Betriebssystem gestartet ===\n");
	test_kernel();
	scheduler_run(); // never returns
}

static void do_data_abort(void)
{
	volatile unsigned int *ptr = (volatile unsigned int *)0x00000001u;
	*ptr			   = 0xDEADBEEFu;
}

static void do_prefetch_abort(void)
{
	__asm__ volatile("bkpt #0" ::: "memory");
}

static void do_supervisor_call(void)
{
	__asm__ volatile("svc #0" ::: "memory");
}

static void do_undefined_inst(void)
{
	__asm__ volatile(".word 0xe7f000f0" ::: "memory");
}