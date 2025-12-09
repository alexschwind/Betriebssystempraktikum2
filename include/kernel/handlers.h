#ifndef HANDLERS_H_
#define HANDLERS_H_

#include <stdbool.h>
#include <stdint.h>

struct exception_frame {
	uint32_t spsr;
	uint32_t cpsr;
	uint32_t r[13];
	uint32_t lr;
};

void undefined_handler [[noreturn]] (struct exception_frame *frame);
void svc_handler [[noreturn]] (struct exception_frame *frame);
void prefetch_abort_handler [[noreturn]] (struct exception_frame *frame);
void data_abort_handler [[noreturn]] (struct exception_frame *frame);
void irq_handler(struct exception_frame *frame);

#endif
