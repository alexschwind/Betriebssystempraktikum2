#ifndef IRQ_H
#define IRQ_H

#define IRQ_BASE (0x7E00B000u - 0x3F000000u)

struct irq_controller {
	unsigned int irq_basic_pending; 
	unsigned int irq_pending_1; 
	unsigned int irq_pending_2; 
	unsigned int fiq_control; 
	unsigned int enable_irqs_1; 
	unsigned int enable_irqs_2; 
	unsigned int enable_basic_irqs; 
	unsigned int disable_irqs_1; 
	unsigned int disable_irqs_2; 
	unsigned int disable_basic_irqs; 
};

unsigned int irq_get_uart_pending(void);
unsigned int irq_get_systimer_pending(unsigned int timer_id);

void irq_enable_uart(void);
void irq_enable_systimer(unsigned int timer_id);

void irq_disable_uart(void);
void irq_disable_systimer(unsigned int timer_id);

#endif
