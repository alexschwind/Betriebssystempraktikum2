#include <arch/bsp/systimer.h>

volatile struct systimer *const systimer = (struct systimer *)SYSTIMER_BASE;

static inline int systimer_valid_channel(unsigned int timer)
{
	return timer < 4u;
}

void systimer_clear_match(unsigned int timer)
{
	if (!systimer_valid_channel(timer)) {
		return;
	}

	systimer->cs = (1u << timer);
}

void systimer_increment_compare(unsigned int timer, unsigned int interval)
{
	if (!systimer_valid_channel(timer)) {
		return;
	}

	unsigned int now = systimer->clo;
	switch (timer) {
	case 0:
		systimer->c0 = now + interval;
		return;
	case 1:
		systimer->c1 = now + interval;
		return;
	case 2:
		systimer->c2 = now + interval;
		return;
	case 3:
		systimer->c3 = now + interval;
		return;
	default:
		return; 
	}
}
