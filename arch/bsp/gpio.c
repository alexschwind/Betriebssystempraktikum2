#include <arch/bsp/gpio.h>

volatile struct gpio *const gpio = (struct gpio *)GPIO_BASE;

void gpio_set_alt_function(unsigned int pin, unsigned int alt_function)
{
	unsigned int function_encoding;
	switch (alt_function) {
	case 0:
		function_encoding = 0b100; // ALT0
		break;
	case 1:
		function_encoding = 0b101; // ALT1
		break;
	case 2:
		function_encoding = 0b110; // ALT2
		break;
	case 3:
		function_encoding = 0b111; // ALT3
		break;
	case 4:
		function_encoding = 0b011; // ALT4
		break;
	case 5:
		function_encoding = 0b010; // ALT5
		break;
	default:
		return; // Invalid alt function
	}

	unsigned int reg   = pin / 10u;
	unsigned int shift = (pin % 10u) * 3u;
	unsigned int mask  = 0x7u << shift; // 3-bit field per pin

	gpio->func[reg] = (gpio->func[reg] & ~mask) | ((function_encoding & 0x7u) << shift);
}
