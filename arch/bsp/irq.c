#include <arch/bsp/irq.h>

#define IRQ_REG_BASE (IRQ_BASE + 0x200u)

volatile struct irq_controller *irq_regs(void)
{
	return (volatile struct irq_controller *)IRQ_REG_BASE;
}

unsigned int irq_get_uart_pending(void)
{
	return (irq_regs()->irq_pending_2 >> (57 - 32)) & 0x1u;
}

unsigned int irq_get_systimer_pending(unsigned int timer_id)
{
	return irq_regs()->irq_pending_1 & (1u << timer_id);
}

void irq_enable_uart(void)
{
	irq_regs()->enable_irqs_2 = 1u << (57 - 32);
}

void irq_enable_systimer(unsigned int timer_id)
{
	irq_regs()->enable_irqs_1 = 1u << timer_id;
}

void irq_disable_uart(void)
{
	irq_regs()->disable_irqs_2 = 1u << (57 - 32);
}

void irq_disable_systimer(unsigned int timer_id)
{
	irq_regs()->disable_irqs_1 = 1u << timer_id;
}