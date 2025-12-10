#include <lib/exception_print.h>

#include <kernel/handlers.h>

#include <lib/kprintf.h>

#include <stddef.h>

static const char *get_fsr_description(unsigned int fsr);

#define r0 ((unsigned int)(frame->r0))
#define r1 ((unsigned int)(frame->r1))
#define r2 ((unsigned int)(frame->r2))
#define r3 ((unsigned int)(frame->r3))
#define r4 ((unsigned int)(frame->r4))
#define r5 ((unsigned int)(frame->r5))
#define r6 ((unsigned int)(frame->r6))
#define r7 ((unsigned int)(frame->r7))
#define r8 ((unsigned int)(frame->r8))
#define r9 ((unsigned int)(frame->r9))
#define r10 ((unsigned int)(frame->r10))
#define r11 ((unsigned int)(frame->r11))
#define r12 ((unsigned int)(frame->r12))

void print_exception_infos(context_frame_t *frame, const struct exception_info *info)
{
	kprintf("\n############ %s at 0x%08x ############\n", info->exception_name, (unsigned int)info->exception_source_addr);
	if (info->is_data_abort) {
		const char *dfsr_description = get_fsr_description(info->data_fault_status_register);
		kprintf("Data Fault Status Register: 0x%08x -> %s\n",
		    (unsigned int)info->data_fault_status_register, dfsr_description);
		kprintf("Data Fault Adress Register: 0x%08x\n",
		    (unsigned int)info->data_fault_address_register);
	}

	if (info->is_prefetch_abort) {
		const char *ifsr_description = get_fsr_description(info->instruction_fault_status_register);
		kprintf("Instruction Fault Status Register: 0x%08x -> %s\n",
		    (unsigned int)info->instruction_fault_status_register, ifsr_description);
		kprintf("Instruction Fault Adress Register: 0x%08x\n",
		    (unsigned int)info->instruction_fault_address_register);
	}

	kprintf("\n>> Registerschnappschuss <<\n");
	kprintf("R0: 0x%08x  R5: 0x%08x  R10: 0x%08x\n", r0, r5, r10);
	kprintf("R1: 0x%08x  R6: 0x%08x  R11: 0x%08x\n", r1, r6, r11);
	kprintf("R2: 0x%08x  R7: 0x%08x  R12: 0x%08x\n", r2, r7, r12);
	kprintf("R3: 0x%08x  R8: 0x%08x\n", r3, r8);
	kprintf("R4: 0x%08x  R9: 0x%08x\n", r4, r9);
}

#undef r0
#undef r1
#undef r2
#undef r3
#undef r4
#undef r5
#undef r6
#undef r7
#undef r8
#undef r9
#undef r10
#undef r11
#undef r12

static const char *get_fsr_description(unsigned int fsr)
{
	static const char *const fsr_sources[32] = {
		[0b00000] = "No function, reset value",
		[0b00001] = "Alignment fault",
		[0b00010] = "Debug event fault",
		[0b00011] = "Access Flag fault on Section",
		[0b00100] = "Cache maintenance operation fault",
		[0b00101] = "Translation fault on Section",
		[0b00110] = "Access Flag fault on Page",
		[0b00111] = "Translation fault on Page",
		[0b01000] = "Precise External Abort",
		[0b01001] = "Domain fault on Section",
		[0b01011] = "Domain fault on Page",
		[0b01100] = "External abort on Section",
		[0b01101] = "Permission fault on Section",
		[0b01110] = "External abort on Page",
		[0b01111] = "Permission fault on Page",
		[0b10000] = "TLB conflict abort",
		[0b10100] = "Implementation defined fault",
		[0b10110] = "External Abort",
		[0b11000] = "Asynchronous parity error on memory access",
		[0b11001] = "Synchronous parity error on memory access",
		[0b11010] = "Implementation defined fault",
		[0b11100] = "Synchronous parity error on translation table walk on section",
		[0b11110] = "Synchronous parity error on translation table walk on page",
	};

	unsigned int status = (fsr & 0xFU) | ((fsr >> 6U) & 0x10U);
	if (status >= (sizeof(fsr_sources) / sizeof(fsr_sources[0])) || fsr_sources[status] == NULL) {
		return "Invalid fault status register value";
	}

	return fsr_sources[status];
}