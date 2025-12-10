#include <user/main.h>

void main(void *args)
{
	char c = *((char *)args);
	for(unsigned int n = 0; n < PRINT_COUNT; n++){
		for(volatile unsigned int i = 0; i < BUSY_WAIT_COUNTER; i++){}
		uart_putc(c);
	}
}