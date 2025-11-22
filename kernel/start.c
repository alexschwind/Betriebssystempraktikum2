#include <config.h>

#include <arch/bsp/uart.h>
#include <arch/bsp/systimer.h>
#include <arch/bsp/irq.h>

#include <kernel/handlers.h>

#include <lib/kprintf.h>

#include <tests/regcheck.h>

#include <stdbool.h>

static void subprogram [[noreturn]] (void);

static void do_data_abort(void);
static void do_prefetch_abort(void);
static void do_supervisor_call(void);
static void do_undefined_inst(void);

void start_kernel [[noreturn]] (void);
void start_kernel [[noreturn]] (void)
{
	// UART Initialization
	uart_init();
	uart_enable_rx_interrupt();
	irq_enable_uart();

	// Systimer Initialization
	systimer_increment_compare(1, TIMER_INTERVAL);
	irq_enable_systimer(1);

	__asm__ volatile("cpsie i" ::: "memory"); // Enable irq interrupts

	kprintf("=== Betriebssystem gestartet ===\n");
	test_kernel();
	while (true) {
		char c = uart_getc();
		switch (c) {
		case 'd':
			irq_debug = !irq_debug;
			break;
		case 'a':
			do_data_abort();
			break;
		case 'p':
			do_prefetch_abort();
			break;
		case 's':
			do_supervisor_call();
			break;
		case 'u':
			do_undefined_inst();
			break;
		case 'c':
			register_checker();
			break;
		case 'e':
			subprogram();
		default:
			kprintf("Unknown input: %c\n", c);
			break;
		}
	}
}

static void subprogram [[noreturn]] (void)
{
	while (true) {
		char c = uart_getc();
		for (unsigned int n = 0; n < PRINT_COUNT; n++) {
			uart_putc(c);
			volatile unsigned int i = 0;
			for (; i < BUSY_WAIT_COUNTER; i++) {
			}
		}
	}
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