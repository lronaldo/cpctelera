/** bug-3730.c : An issue in handling of parameter offsets for z80-ports when using --fomit-frame-pointer.
*/

#include <testfwk.h>

#pragma disable_warning 85

#include <stdarg.h>

const char hexTable[] = "0123456789ABCDEF";

int my_putchar(int c)
{
    static const char e[] = "31 32 33 (expected)\n" "31 32 33\n";
    static int i;
    ASSERT (c == e[i++]);
}

void print_hx4(unsigned char u)
{
    u &= 0xf;
    my_putchar(hexTable[u]);
}


void print_hx8(unsigned char u)
{
    print_hx4(u >> 4);
    print_hx4(u);
}


int myprintf(const char *fmt, ...)
{
    char *s;
    char c;
    unsigned int u;

    va_list arg;
    va_start(arg, fmt);

    while ((c = *fmt)) {
        if (c == '%') {
            ++fmt;
            for (;;) {
                c = *fmt;
                if (c < '0' || c > '9')
                    break;
                ++fmt;
            }
            switch (c) {
            case 'x':
                c = va_arg( arg, int);
                print_hx8(c);
                break;
            default:
                break;
            }
        } else {
            my_putchar(c);
        }
        fmt++;
    }
    return 0;
}


void
testBug(void)
{
    char a, b, c;

    myprintf( "31 32 33 (expected)\n");

    a = '1';  b = '2'; c = '3';
    myprintf( "%x %x %x\n", a, b, c);
}

