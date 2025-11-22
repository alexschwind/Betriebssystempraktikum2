#ifndef SYSTIMER_H
#define SYSTIMER_H

#define SYSTIMER_BASE (0x7E003000u - 0x3F000000u)

struct systimer {
    unsigned int cs;
    unsigned int clo;
    unsigned int chi;
    unsigned int c0;
    unsigned int c1;
    unsigned int c2;
    unsigned int c3;
};

extern volatile struct systimer *const systimer;

void systimer_clear_match(unsigned int timer); // Write a one to the relevant bit to clear the match detect status bit and the corresponding interrupt request line.

void systimer_increment_compare(unsigned int timer, unsigned int interval); // Increment the compare register by the given interval.

#endif
