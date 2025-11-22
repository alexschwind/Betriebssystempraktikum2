#ifndef IRQ_H
#define IRQ_H

#define IRQ_BASE (0x7E00B000u - 0x3F000000u)

// IMPORTANT: FOR THE IRQ CONTROLLER THE OFFSETS ARE WEIRD and dont start at the base address! so I added comments with the real offsets
struct irq_controller {
	unsigned int irq_basic_pending; // offset 0x200
	unsigned int irq_pending_1; // offset 0x204
	unsigned int irq_pending_2; // offset 0x208
	unsigned int fiq_control; // offset 0x20C
	unsigned int enable_irqs_1; // offset 0x210
	unsigned int enable_irqs_2; // offset 0x214
	unsigned int enable_basic_irqs; // offset 0x218
	unsigned int disable_irqs_1; // offset 0x21C
	unsigned int disable_irqs_2; // offset 0x220
	unsigned int disable_basic_irqs; // offset 0x224
};

unsigned int irq_get_uart_pending(void);
unsigned int irq_get_systimer_pending(unsigned int timer_id);

void irq_enable_uart(void);
void irq_enable_systimer(unsigned int timer_id);

void irq_disable_uart(void);
void irq_disable_systimer(unsigned int timer_id);

#endif
