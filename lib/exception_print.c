#include <lib/exception_print.h>

#include <kernel/handlers.h>

#include <lib/kprintf.h>

#include <stddef.h>

static void print_psr(unsigned int psr);
static const char *get_fsr_description(unsigned int fsr);
static const char *psr_mode_name(unsigned int psr);
extern void read_mode_specific_registers_hw(struct mode_regs *out);

struct mode_regs read_mode_specific_registers(void)
{
	struct mode_regs regs = {0};
	read_mode_specific_registers_hw(&regs);
	return regs;
}


#define r0 ((unsigned int)(frame->r[0]))
#define r1 ((unsigned int)(frame->r[1]))
#define r2 ((unsigned int)(frame->r[2]))
#define r3 ((unsigned int)(frame->r[3]))
#define r4 ((unsigned int)(frame->r[4]))
#define r5 ((unsigned int)(frame->r[5]))
#define r6 ((unsigned int)(frame->r[6]))
#define r7 ((unsigned int)(frame->r[7]))
#define r8 ((unsigned int)(frame->r[8]))
#define r9 ((unsigned int)(frame->r[9]))
#define r10 ((unsigned int)(frame->r[10]))
#define r11 ((unsigned int)(frame->r[11]))
#define r12 ((unsigned int)(frame->r[12]))

void print_exception_infos(struct exception_frame *frame,
		const struct exception_info *info)
{
	kprintf("############ EXCEPTION ############\n");
	kprintf("%s an Adresse: 0x%08x\n", info->exception_name,
	    (unsigned int)info->exception_source_addr);
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

	struct mode_regs mode_regs = read_mode_specific_registers();

	kprintf("\n>> Modusspezifische Register <<\n");
	kprintf("User/System | LR: 0x%08x | SP: 0x%08x | CPSR: ",
	    (unsigned int)mode_regs.user_lr, (unsigned int)mode_regs.user_sp);
	print_psr((unsigned int)frame->cpsr);
	kprintf("\nIRQ         | LR: 0x%08x | SP: 0x%08x | SPSR: ",
	    (unsigned int)mode_regs.irq_lr, (unsigned int)mode_regs.irq_sp);
	print_psr((unsigned int)mode_regs.irq_spsr);
	kprintf("\nAbort       | LR: 0x%08x | SP: 0x%08x | SPSR: ",
	    (unsigned int)mode_regs.abort_lr, (unsigned int)mode_regs.abort_sp);
	print_psr((unsigned int)mode_regs.abort_spsr);
	kprintf("\nUndefined   | LR: 0x%08x | SP: 0x%08x | SPSR: ",
	    (unsigned int)mode_regs.undefined_lr, (unsigned int)mode_regs.undefined_sp);
	print_psr((unsigned int)mode_regs.undefined_spsr);
	kprintf("\nSupervisor  | LR: 0x%08x | SP: 0x%08x | SPSR: ",
	    (unsigned int)mode_regs.supervisor_lr, (unsigned int)mode_regs.supervisor_sp);
	print_psr((unsigned int)mode_regs.supervisor_spsr);
	kprintf("\n");
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

#define PSR_FLAG(psr_value, bit) (((psr_value) >> (bit)) & 1U)

static void print_psr(unsigned int psr)
{
	kprintf("%c%c%c%c %c %c%c%c",
		PSR_FLAG(psr, 31) ? 'N' : '_',
		PSR_FLAG(psr, 30) ? 'Z' : '_',
		PSR_FLAG(psr, 29) ? 'C' : '_',
		PSR_FLAG(psr, 28) ? 'V' : '_',
		PSR_FLAG(psr, 9) ? 'E' : '_',
		PSR_FLAG(psr, 7) ? 'I' : '_',
		PSR_FLAG(psr, 6) ? 'F' : '_',
		PSR_FLAG(psr, 5) ? 'T' : '_');

	kprintf(" %s", psr_mode_name(psr));
	kprintf(" 0x%08x", psr);
}

#undef PSR_FLAG

static const char *psr_mode_name(unsigned int psr)
{
	static char padded[12];
	static const char *const mode_names[32] = {
		[0b10000] = "      User",
		[0b10001] = "       FIQ",
		[0b10010] = "       IRQ",
		[0b10011] = "Supervisor",
		[0b10111] = "     Abort",
		[0b11011] = " Undefined",
		[0b11111] = "    System",
	};

	const char *name = mode_names[psr & 0x1FU];
	if (!name) {
		name = "   Invalid";
	}

	unsigned int width = 8U;
	unsigned int len = 0U;
	while (name[len] != '\0') {
		++len;
	}

	unsigned int pad = width > len ? width - len : 0U;
	unsigned int pos = 0U;
	while (pos < pad && pos < (unsigned int)(sizeof(padded) - 1U)) {
		padded[pos++] = ' ';
	}

	for (unsigned int i = 0U; name[i] != '\0' && pos < (unsigned int)(sizeof(padded) - 1U); ++i) {
		padded[pos++] = name[i];
	}

	padded[pos] = '\0';
	return padded;
}

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