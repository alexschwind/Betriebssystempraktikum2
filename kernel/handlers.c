#include <kernel/handlers.h>

#include <lib/exception_print.h>
#include <lib/kprintf.h>

#include <arch/bsp/uart.h>
#include <arch/bsp/irq.h>
#include <arch/bsp/systimer.h>

#include <config.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

static void panic(void) [[noreturn]];

static uint32_t read_dfsr(void);
static uint32_t read_dfar(void);
static uint32_t read_ifsr(void);
static uint32_t read_ifar(void);

bool irq_debug = false;

void undefined_handler(struct exception_frame *frame)
{
    struct exception_info info = {
        .exception_name = "Undefined Instruction",
        .exception_source_addr = frame ? frame->lr : 0U,
    };

    print_exception_infos(frame, &info);

    panic();
    for (;;) {
    }
}

void svc_handler(struct exception_frame *frame)
{
    struct exception_info info = {
        .exception_name = "Supervisor Call",
        .exception_source_addr = frame ? frame->lr : 0U,
    };

    print_exception_infos(frame, &info);

    panic();
    for (;;) {
    }
}

void prefetch_abort_handler(struct exception_frame *frame)
{
    struct exception_info info = {
        .exception_name = "Prefetch Abort",
        .exception_source_addr = frame ? frame->lr : 0U,
    };
    info.is_prefetch_abort = true;
    info.instruction_fault_status_register = read_ifsr();
    info.instruction_fault_address_register = read_ifar();

    print_exception_infos(frame, &info);

    panic();
    for (;;) {
    }
}

void data_abort_handler(struct exception_frame *frame)
{
    struct exception_info info = {
        .exception_name = "Data Abort",
        .exception_source_addr = frame ? frame->lr : 0U,
    };
    info.is_data_abort = true;
    info.data_fault_status_register = read_dfsr();
    info.data_fault_address_register = read_dfar();

    print_exception_infos(frame, &info);

    panic();
    for (;;) {
    }
}

void irq_handler(struct exception_frame *frame)
{
    if (irq_get_systimer_pending(1)) {
        // Handle Timer IRQ
        systimer_clear_match(1);
        systimer_increment_compare(1, TIMER_INTERVAL);
        kprintf("!\n");
    }

    if (irq_get_uart_pending()) {
        // Handle UART IRQ
        if (uart_get_rx_interrupt_status()) {
            uart_rx_into_buffer();
        }
        uart_clear_interrupt();
    }

    if (irq_debug) {
        struct exception_info info = {
            .exception_name = "IRQ",
            .exception_source_addr = frame ? frame->lr : 0U,
        };
        print_exception_infos(frame, &info);
    }
}

static void panic(void)
{
    __asm__ volatile("cpsid if" : : : "memory");

    kprintf("\4");

    for (;;) {
        __asm__ volatile("wfi" ::: "memory");
    }
}

static uint32_t read_dfsr(void)
{
    uint32_t value;
    __asm__ volatile("mrc p15, 0, %0, c5, c0, 0" : "=r"(value));
    return value;
}

static uint32_t read_dfar(void)
{
    uint32_t value;
    __asm__ volatile("mrc p15, 0, %0, c6, c0, 0" : "=r"(value));
    return value;
}

static uint32_t read_ifsr(void)
{
    uint32_t value;
    __asm__ volatile("mrc p15, 0, %0, c5, c0, 1" : "=r"(value));
    return value;
}

static uint32_t read_ifar(void)
{
    uint32_t value;
    __asm__ volatile("mrc p15, 0, %0, c6, c0, 2" : "=r"(value));
    return value;
}