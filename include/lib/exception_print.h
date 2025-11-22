#ifndef EXCEPTION_PRINT_H
#define EXCEPTION_PRINT_H

#include <stdbool.h>
#include <stdint.h>

struct exception_frame;

struct mode_regs {
	uint32_t user_lr;
	uint32_t user_sp;
	uint32_t user_cpsr;
	uint32_t irq_lr;
	uint32_t irq_sp;
	uint32_t irq_spsr;
	uint32_t abort_lr;
	uint32_t abort_sp;
	uint32_t abort_spsr;
	uint32_t undefined_lr;
	uint32_t undefined_sp;
	uint32_t undefined_spsr;
	uint32_t supervisor_lr;
	uint32_t supervisor_sp;
	uint32_t supervisor_spsr;
};

struct exception_info {
	const char *exception_name;
	uint32_t    exception_source_addr;
	bool	    is_data_abort;
	bool	    is_prefetch_abort;
	uint32_t    data_fault_status_register;
	uint32_t    data_fault_address_register;
	uint32_t    instruction_fault_status_register;
	uint32_t    instruction_fault_address_register;
};

struct mode_regs read_mode_specific_registers(void);

void print_exception_infos(struct exception_frame *frame, const struct exception_info *info);

#endif