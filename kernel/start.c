#include <arch/bsp/uart.h>
#include <arch/bsp/systimer.h>
#include <arch/bsp/irq.h>

#include <kernel/scheduler.h>

#include <lib/kprintf.h>

#include <stdbool.h>

#include <config.h>

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

	scheduler_init(); 

	kprintf("=== Betriebssystem gestartet ===\n");
	test_kernel();
	
	scheduler_start();

	__builtin_unreachable();
}