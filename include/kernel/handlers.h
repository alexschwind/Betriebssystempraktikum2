#ifndef HANDLERS_H_
#define HANDLERS_H_

#include <kernel/scheduler.h>

#include <stdbool.h>
#include <stdint.h>

void undefined_handler [[noreturn]] ();
void svc_handler [[noreturn]] ();
void prefetch_abort_handler [[noreturn]] ();
void data_abort_handler [[noreturn]] ();
context_frame_t *irq_handler(context_frame_t *old);

#endif
