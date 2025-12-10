#ifndef HANDLERS_H_
#define HANDLERS_H_

#include <kernel/scheduler.h>

#include <stdbool.h>
#include <stdint.h>

context_frame_t *irq_handler(context_frame_t *old);
context_frame_t *svc_handler(context_frame_t *ctx);
context_frame_t *undefined_handler(context_frame_t *ctx);
context_frame_t *prefetch_abort_handler(context_frame_t *ctx);
context_frame_t *data_abort_handler(context_frame_t *ctx);

#endif
