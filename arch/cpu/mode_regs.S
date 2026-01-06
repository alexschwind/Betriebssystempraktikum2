.section .text

.global read_mode_specific_registers_hw
.type read_mode_specific_registers_hw, %function
read_mode_specific_registers_hw:
	push {r4-r11, lr}

	mrs r4, cpsr		

	mov r5, r4
	bic r5, r5, #0x1F
	orr r5, r5, #0x1F		
	msr cpsr_c, r5

	mov r2, lr
	str r2, [r0, #0]
	mov r2, sp
	str r2, [r0, #4]
	mrs r2, cpsr
	str r2, [r0, #8]

	mov r5, r4
	bic r5, r5, #0x1F
	orr r5, r5, #0x12	
	msr cpsr_c, r5
	mov r2, lr
	str r2, [r0, #12]
	mov r2, sp
	str r2, [r0, #16]
	mrs r2, spsr
	str r2, [r0, #20]

	mov r5, r4
	bic r5, r5, #0x1F
	orr r5, r5, #0x17		
	msr cpsr_c, r5
	mov r2, lr
	str r2, [r0, #24]
	mov r2, sp
	str r2, [r0, #28]
	mrs r2, spsr
	str r2, [r0, #32]

	mov r5, r4
	bic r5, r5, #0x1F
	orr r5, r5, #0x1B		
	msr cpsr_c, r5
	mov r2, lr
	str r2, [r0, #36]
	mov r2, sp
	str r2, [r0, #40]
	mrs r2, spsr
	str r2, [r0, #44]

	mov r5, r4
	bic r5, r5, #0x1F
	orr r5, r5, #0x13		
	msr cpsr_c, r5
	mov r2, lr
	str r2, [r0, #48]
	mov r2, sp
	str r2, [r0, #52]
	mrs r2, spsr
	str r2, [r0, #56]

	msr cpsr_c, r4			

	pop {r4-r11, pc}
.size read_mode_specific_registers_hw, .-read_mode_specific_registers_hw