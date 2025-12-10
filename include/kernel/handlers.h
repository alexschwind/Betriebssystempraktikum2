#ifndef HANDLERS_H_
#define HANDLERS_H_

#include <kernel/scheduler.h>

#include <stdbool.h>
#include <stdint.h>

void irq_handler(context_frame_t *ctx);
void svc_handler(context_frame_t *ctx);
void undefined_handler(context_frame_t *ctx);
void prefetch_abort_handler(context_frame_t *ctx);
void data_abort_handler(context_frame_t *ctx);

#endif
