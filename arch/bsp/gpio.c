#include <arch/bsp/gpio.h>

#define ALT0 0b100
#define ALT1 0b101
#define ALT2 0b110
#define ALT3 0b111
#define ALT4 0b011
#define ALT5 0b010

volatile struct gpio *const gpio = (struct gpio *)GPIO_BASE;

void gpio_set_alt_function(unsigned int pin, unsigned int alt_function)
{
	unsigned int function_encoding;
	switch (alt_function) {
	case 0:
		function_encoding = ALT0;
		break;
	case 1:
		function_encoding = ALT1;
		break;
	case 2:
		function_encoding = ALT2;
		break;
	case 3:
		function_encoding = ALT3;
		break;
	case 4:
		function_encoding = ALT4;
		break;
	case 5:
		function_encoding = ALT5;
		break;
	default:
		return;
	}

	unsigned int reg   = pin / 10u;
	unsigned int shift = (pin % 10u) * 3u;
	unsigned int mask  = 0x7u << shift;

	gpio->func[reg] = (gpio->func[reg] & ~mask) | ((function_encoding & 0x7u) << shift);
}
