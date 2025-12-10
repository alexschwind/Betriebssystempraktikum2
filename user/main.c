#include <user/main.h>
#include <tests/regcheck.h>
#include <config.h>

static void do_prefetch_abort(void);
static void do_data_abort(void);
static void do_svc(void);
static void do_undef(void);

void main(void * args) {
	test_user(args);
	char c = *((char *) args);
	switch (c) {
		case 'a':
			do_data_abort();
			return;
		case 'p':
			do_prefetch_abort();
			return;
		case 'u':
			do_undef();
			return;
		case 's':
			do_svc();
			return;
		case 'c':
			register_checker();
			return;
	}
	
	for(unsigned int n = 0; n < PRINT_COUNT; n++){
		for(volatile unsigned int i = 0; i < BUSY_WAIT_COUNTER; i++){}
		uart_putc(c);
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

static void do_svc(void)
{
	__asm__ volatile("svc #1" ::: "memory");
}

static void do_undef(void)
{
	__asm__ volatile(".word 0xe7f000f0" ::: "memory");
}