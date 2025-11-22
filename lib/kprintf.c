#include <lib/kprintf.h>

#include <arch/bsp/uart.h>

#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

static size_t build_digits(unsigned int value, unsigned int base, char *buf)
{
    static const char digits[] = "0123456789abcdef";
    size_t i = 0;

    do {
        buf[i++] = digits[value % base];
        value /= base;
    } while (value);

    return i;
}

static void emit_unsigned_int(unsigned int value, unsigned int base, int width, bool zero_pad)
{
    char buf[32];

    if (base < 2 || base > 16) {
        return;
    }

    size_t digits = build_digits(value, base, buf);
    int pad = width > (int)digits ? width - (int)digits : 0;
    char pad_char = zero_pad ? '0' : ' ';

    for (int i = 0; i < pad; ++i) {
        uart_putc(pad_char);
    }

    while (digits--) {
        uart_putc(buf[digits]);
    }
}

static void emit_signed_int(int value, int width, bool zero_pad)
{
    char buf[32];
    bool negative = value < 0;
    unsigned int magnitude = negative ? (unsigned int)(-(int64_t)value) : (unsigned int)value;
    size_t digits = build_digits(magnitude, 10, buf);
    int total = (int)digits + (negative ? 1 : 0);
    int pad = width > total ? width - total : 0;

    if (!zero_pad) {
        for (int i = 0; i < pad; ++i) {
            uart_putc(' ');
        }
    }

    if (negative) {
        uart_putc('-');
    }

    if (zero_pad) {
        for (int i = 0; i < pad; ++i) {
            uart_putc('0');
        }
    }

    while (digits--) {
        uart_putc(buf[digits]);
    }
}

void kprintf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    while (*format) {
        if (*format != '%') {
            uart_putc(*format++);
            continue;
        }

        ++format;  // skip '%'

        if (*format == '%') {
            uart_putc('%');
            ++format;
            continue;
        }

        bool zero_pad = false;
        int width = 0;

        if (*format == '0') {
            zero_pad = true;
            ++format;
        }

        if (*format == '8') {
            width = 8;
            ++format;
        } else if (zero_pad) {
            uart_puts("Unknown conversion specifier");
            if (*format) {
                ++format;
            }
            continue;
        }

        char spec = *format;
        if (!spec) {
            break;
        }

        switch (spec) {
        case 'i':
            emit_signed_int(va_arg(args, int), width, zero_pad);
            break;
        case 'u':
            emit_unsigned_int(va_arg(args, unsigned int), 10, width, zero_pad);
            break;
        case 'x':
            emit_unsigned_int(va_arg(args, unsigned int), 16, width, zero_pad);
            break;
        case 'p':
            {
                uintptr_t ptr = (uintptr_t)va_arg(args, void *);
                uart_puts("0x");
                emit_unsigned_int((unsigned int)ptr, 16, 8, true);
            }
            break;
        case 'c':
            uart_putc((char)va_arg(args, int));
            break;
        case 's':
            {
                const char *s = va_arg(args, const char *);
                if (!s) {
                    s = "(null)";
                }
                while (*s) {
                    uart_putc(*s++);
                }
            }
            break;
        default:
            uart_puts("Unknown conversion specifier");
            break;
        }

        ++format;
    }

    va_end(args);
}
