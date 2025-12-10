# Kontextwechsel und präemptives Multitasking
In dieser Aufgabe soll unser Betriebssystem um die Fähigkeit des präemptiven Multitaskings erweitert werden, um so mehrere Threads quasi-parallel ausführen zu können. Der Timer dient dabei als Zeitgeber und gibt das Intervall vor, in dem der Kern den gerade ausgeführten Thread automatisch wechselt.

# Aufgaben
1. eine Threadverwaltung implementieren, die ein Minimum von 32 Threads unterstützt.
2. einen Kontextwechsel ermöglichen. Dafür ist es u.a. notwendig, nach Eintritt in eine Ausnahme
    1. die Register auf dem Stack zu sichern,
    2. die Register eines Threads in dessen TCB zu sichern,
    3. den nächsten Thread zu ermitteln und dessen Werte auszutauschen,
    4. die Register wiederherzustellen und
    5. aus der Ausnahme zurückzuspringen.
3. einen Scheduler zu implementieren, der regelmäßig einen Kontextwechsel anstößt. Es soll kein Thread verhungern und die verfügbare CPU-Zeit gleichmäßig zwischen den Threads aufgeteilt werden (z.B. eine Round-
Robin-Scheduler).
4. einen Supervisor Call zu implementieren, der einen Thread beendet.
5. euch zu überlegen, wie ihr mit dem Fall umgeht, dass es keine Threads zum Ausführen gibt.
6. den Ausnahme-Handler so anzupassen, dass kein Thread das Betriebssystem zum Absturz bringen kann.

## Details
- Es gibt eine Threadverwaltung, die mindestens 32 Threads unterstützt (Ein Idel-Thread zählt zu diesen 32 Threads, also müssen nur 31 reguläre Threads unterstützt werden.) 

- Threads können über void scheduler_thread_create(void(* func)(void *), const void * arg, unsigned int arg_size), erstellt werden, wobei func die Einstiegs-Funktion des neuen Threads ist, arg ein Pointer auf die Argumente der Länge arg_size, ist, die auf den Stack des neuen Threads zu kopieren sind

- Wenn kein Thread erstellt werden konnte, weil bereits alle Threads verwendet sind, wird „Could not create thread.” ausgegeben und keine Aktion getätigt

- Die Einstiegs-Funktion des Threads kriegt beim Start einen Pointer auf das auf den Stack kopierte Argument übergeben

- Jeder Thread hat einen eigenen Stack, welcher mindestens 1KiB groß ist 

- Es gibt eine Funktion void syscall_exit(void), die einen svc ausführt. Jeder svc verursacht, dass der Thread, der diesen ausführte, beendet wird (Wenn ein Thread beendet wird soll kein Registerdump ausgegeben werden.)

- Threads werden beendet, wenn diese die Einstiegs-Funktion beenden, auch wenn, hierbei nicht explizit syscall_exit aufgerufen wird

- Threads werden von einem Scheduler regelmäßig ausgeführt. Dabei ist zu beachten, dass
    - die verfügbare CPU-Zeit gleichmäßig zwischen den Threads aufgeteilt wird
    - die Länge der Zeitscheiben von TIMER_INTERVAL aus config.h abhängig ist
    - der Scheduler nicht unnötig wartet, d.h. wenn ein Thread ausgeführt werden kann, dieser ausgeführt wird
    - der Scheduler damit umgehen kann, dass kein Thread ausgeführt werden kann

- Ein Thread kann den Kernel nicht zum Abstürzen bringen
- Wenn ein Thread abstürzt, wird dieser mit einem Registerdump beendet
- Wenn der Kernel abstürzt, wird ein Registerdump ausgegeben und der gesamte Kernel angehalten
- Threads werden im User Modus ausgeführt


- Zur Demonstration der Funktionalität müssen außerdem die folgenden Anforderungen erfüllt werden
    - Bei jedem Kontextwechsel wird ein Zeilenumbruch ausgegeben.
    - Bei jedem Timer-Interrupt wird ein Ausrufezeichen ausgegeben.
    - Innerhalb der Interrupt Behandlung wird nach dem Empfangen des Zeichens: S ein Supervisor Call ausgelöst, P ein Prefetch Abort erzeugt, A ein Data Abort erzeugt, U eine Undefined Instruction ausgeführt, bei allen anderen Zeichen ein neuer Thread erstellt, welcher die main Funktion aus Anhang A mit dem empfangenen Zeichen als Argument ausführt (Das erstellen von Threads in der Interrupt Behandlung ist i.d.R. nicht zu empfehlen. Der Bequemlichkeit halber erlauben wir dies hier trotzdem.)

- Die in dem Beispiel aus Anhang A verwendeten Funktionen do_xyz() lösen jeweils die ihrem Namen entsprechende Ausnahme aus
- Nach der Initialisierung eures Betriebssystems muss der String === Betriebssystem gestartet === ausgeben werden.
- Nach dieser Ausgabe ruft das Betriebssystem test_kernel() auf, welche in config.h deklariert ist
- Nachdem test_kernel() aufgerufen wurde, wird der Scheduler ausgeführt

# Hinweise
- Achtet beim Initialisieren des PSR für einen neuen Thread darauf, dass alle Bits sinnvoll gesetzt sind.
- Der Register Checker wird eine Warnung ausgeben, dass nicht alle Tests möglich sind aufgrund des User Modus. Dies ist in Ordnung.
- Damit sich der Kernel in jeder Situation den Anforderungen entsprechend verhält, ist es sinnvoll, sicherzustellen, dass sich die Ausführung verschiedener Kernelfunktionen gegenseitig ausschließt. Da wir nur einen aktiven Kern haben, kann dies realisiert werden, in dem Interrupts im Kernel maskiert werden.

# Anhang A

```c
#include <user/main.h>
#include <tests/regcheck.h>
#include <config.h>

void main(void * args) {
	test_user(args);
	char c = *((char *) args);
	switch (c) {
		case 'a':
			do_data_abort();
			return;
		case 'p':
			do_prefetch_abort();
			return;
		case 'u':
			do_undef();
			return;
		case 's':
			do_svc();
			return;
		case 'c':
			register_checker();
			return;
	}
	
	for(unsigned int n = 0; n < PRINT_COUNT; n++){
		for(volatile unsigned int i = 0; i < BUSY_WAIT_COUNTER; i++){}
		uart_putc(c);
	}
}
```