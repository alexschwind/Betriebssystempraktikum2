# User / Kernel Interface
Wir definieren eine ordentliche Schnittstelle zwischen Anwendungen und Betriebssystem und führen blockierende Systemrufe ein, damit wartende Threads nicht unnötig die CPU blockieren.

1. ✅Legt eine dokumentierte Aufrufkonvention für Systemrufe mittels SVC fest.
2. ✅Implementiert die Systemrufe syscall_exit, syscall_putc, syscall_getc, syscall_create und syscall_sleep.
3. ✅Stellt sicher, dass die Anwendungen und der Kernel unabhängig voneinander gebaut werden können.

## Details
1. Es gibt eine dokumentierte Aufrufkonvention für Systemaufrufe mittels SVC
2. Threads können über die Funktion...
	- ✅void syscall_exit [[noreturn]] (void) einen Syscall auslösen, um sich selbst zu beenden
	- ✅void syscall_create_thread(void (*f) (void *), void * args, unsigned int arg_size) einen Syscall auslösen, um einen neuen Thread zu erstellen
	- char syscall_getc(void) einen Syscall auslösen, um ein Zeichen einzulesen
	- ✅void syscall_putc(char c) einen Syscall auslösen, um ein Zeichen auszugeben
	- ✅void syscall_sleep(unsigned int cycles) einen Syscall auslösen, um sich selbst für mindestens cycles-viele Zeitscheiben nicht ausführen zu lassen (Angefangene Zeitscheiben dürfen als ganze gezählt werden.)
	- ✅void syscall_undefined(void) einen dem Kernel unbekannten Syscall ausführen

3. Die Systemaufrufe syscall_getc und syscall_sleep arbeiten blockierend, d.h. die Threads werden so lange vom Scheduler nicht erfasst, bis ein Zeichen eingelesen werden konnte, oder entsprechend die Anzahl der Zeitscheiben abgelaufen ist

4. Ein Thread ist nicht in der Lage, den Kernel zum Abstürzen zu bringen,insbesondere nicht durch das Auslösen von Ausnahmen


5. ✅Der Aufruf eines unbekannten Syscalls beendet den Thread, der diesen tätigte, und gibt einen Register Dump des Threads aus


6. ✅Wird ein Syscall aus dem Kernel heraus aufgerufen, so wird ein Registerdump ausgegeben, der gesamte Kernel angehalten, und anschließend \4 ausgegeben


7. Empfangene Zeichen werden, wenn sie nicht direkt verwendet werden können, in einem Ringbuffer gesichert


8. ✅Der Kernel muss vom User Code unabhängig baubar sein. Insbesondere müssen make kernel_only und make user_only funktionieren und die verschiedenen Code-Dateien den Variablen SRC und USRC sinnvoll zugeordnet werden
9. ✅Nach der Initialisierung eures Betriebssystems muss der String === Betriebssystem gestartet === ausgeben werden
10. ✅Nach dieser Ausgabe ruft das Betriebssystem test_kernel() auf, welche in config.h deklariert ist
11. ✅Nachdem test_kernel() aufgerufen wurde, wird der Scheduler ausgeführt
12. ✅Zur Demonstration führt der initiale Thread die Funktion main aus dem Anhang aus

13. ✅Desweiteren führt der Kernel den zu syscall_exit() korrespondierenden Syscall aus, sobald ein S empfangen wurde

## Hinweise
- Damit Zeichen empfangen werden können, müssen Interrupts innerhalb der Threads aktiv sein.
- An gewissen kritischen Punkten gestaltet sich die komplette Trennung von Kernel und User schwierig. Zum Beispiel muss der Kernel das Label für den initialen Thread kennen. Informiert euch hierfür über die Verwendung von [[gnu::weak]] bzw. [[gnu::weak, gnu::alias()]], um das Problem zu umgehen.
- Überlegt euch auch, wie der Syscall syscall_sleep mit dem Wert 0 umgehen sollte.
- Damit sich der Kernel in jeder Situation den Anforderungen entsprechend verhält, ist es sinnvoll, sicherzustellen, dass sich die Ausführung verschiedener Kernelfunktionen gegenseitig ausschließt. Da wir nur einen aktiven Kern haben, kann dies realisiert werden, in dem Interrupts im Kernel maskiert werden.


## Demonstration
Bei der aus dem Demonstartions-Code resultierenden Ausgaben ist insbesondere das zeitliche Verhalten spannend. Die folgenden Beispiele gehen davon aus, dass qemu mit -icount 9 ausgeführt wurde, BUSY_WAIT_COUNTER den Wert 300000, PRINT_COUNT den Wert 10 und TIMER_INTERVAL den Wert 1000000 hat.

2x aktives Warten ((sleep 1; echo -n "BC") | make qemu): BCBBCBBCBBCBBCBCCCCC
1x aktiv + 1x passiv ((sleep 1; echo -n "bC") | make qemu): bCbCbbCbbCbCbbCbCCCC
2x passiv ((sleep 1; echo -n "bc") | make qemu): bcbcbcbbcbcbbcbcbccc
1x aktiv + 2x passiv ((sleep 1; echo -n "Bcd") | make qemu): BcdBBcdBBcBdcBBdcBBcdcdcdccddd
2x aktiv + 1x passiv ((sleep 1; echo -n "BCd") | make qemu): BCdBBdCBdBCBdBCdBBdCBdCdCCdCdC