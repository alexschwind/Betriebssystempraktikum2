#ifndef KPRINTF_H
#define KPRINTF_H

void kprintf(const char *format, ...) __attribute__((format(printf, 1, 2)));

#endif