
.section .init

.extern _vector_table
.extern __stack_svc_top
.extern __stack_irq_top
.extern __stack_abt_top
.extern __stack_und_top
.extern __stack_sys_top


.global _start
_start:
	/* Erkennen ob Hypervisor Mode aktiv */
	mrs r0, cpsr
	and r0, r0, #0x1F
	mov r1, #0x1A	//Modebits = 0x1A = Hypervisor Mode
	cmp r0, r1		//Vergleichen und Statusbits setzen für bedingte Sprünge

	/* Wenn Hypemode erkannt: abschalten (sprung nach _exitHyper)
	 * wenn nicht erkannt: dann weitermachen (weiterlaufen nach _bsprak)
	 */
	beq _exitHyper

	/* Qemu startet immer alle 4 Kerne
	 * Wir lassen alle anderen Kerne endless loopen
	 */
_checkCores:
	/* Id des Cpu Cores Abfragen */
	mrc p15, 0, r0, c0, c0, 5
	/* Falls nicht core 0 Core disablen */
	tst r0, #3
	bne _parkCore

/* not modeled in qemu 6.0 */
_enableAlignCheck:
	/* SCTLR des cp15 laden */
	mrc p15, 0, r0, c1, c0, 0
	/* A-bit (c1[1]) auf 1 setzen */
	orr r0, r0, #0x2
	/* neues SCTLR speichern */
	mcr p15, 0, r0, c1, c0, 0


_bsprak:
	/* Stacks für alle Modi vorbereiten */
	/* Wir benutzen den FIQ und den User mode noch nicht */

	cpsid if                  /* Disable IRQ and FIQ */

	cps	#0x13                 /* Switch to supervisor mode */
	ldr sp, =__stack_svc_top
	cps	#0x12                 /* Switch to IRQ mode */
	ldr sp, =__stack_irq_top
	cps	#0x17                 /* Switch to ABT mode */
	ldr sp, =__stack_abt_top
	cps	#0x1b                 /* Switch to UND mode */
	ldr sp, =__stack_und_top
	cps	#0x1f                 /* Switch to system mode */
	ldr sp, =__stack_sys_top

	cps	#0x13                 /* Run kernel in supervisor mode */
	
	/* Exception Vector Table adresse setzen */
	ldr r0, =_vector_table
	mcr p15, 0, r0, c12, c0, 0

	/* Zu c Code springen */
	bl  start_kernel
.Lend:
	WFI
	b .Lend

_parkCore:
	/* Interrupts für Core 1 bis 3 ausschalten  */
	cpsid if
	/* In Endlosschleife springen */
	b .Lend

_exitHyper:

	/* Rücksprungadresse ins Hypervisor Rücksprungregister schreiben.
	 * Dies ist das Label _bsprak für die normale Programmausfürung.
	 */
	ldr lr, =_checkCores
	msr ELR_hyp, lr

	/* Der Hypervisor mode ist auch "nur" eine etwas kompliziertere Exception.
	 * Also springen wir aus dieser zurück.
	 * (Rücksprunge in Aufgabe 2 sind anders zu lösen!).
	 * Dazu zunächst das CPSR auslesen und die Modebits zum Supervisor ändern.
	 * Danach dies in das SPSR für den Rücksprung schreiben.
	 */
	mrs r0, cpsr
	bic r0, r0, #0x1F
	orr r0, r0, #0x13	//Supervisormode
	msr spsr_hyp, r0
	eret

