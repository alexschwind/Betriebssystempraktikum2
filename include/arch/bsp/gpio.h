#ifndef GPIO_H
#define GPIO_H

#define GPIO_BASE (0x7E200000u - 0x3F000000u)

struct gpio {
	unsigned int func[6];
	unsigned int unused0;
	unsigned int set[2];
	unsigned int unused1;
	unsigned int clr[2];
};

extern volatile struct gpio *const gpio;

void gpio_set_alt_function(unsigned int pin,
			   unsigned int alt_function); // Set the alt function of a GPIO pin

#endif
