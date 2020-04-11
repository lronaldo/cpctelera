/** Test use of pdk15 nadd instructions.

*/
#include <testfwk.h>

unsigned char a, b, c;

void f(void)
{
    c = b - (a & 0x5f);
}

void g(void)
{
    c = (a & 0x5f) - c;
}

void testNadd(void)
{
    a = 1;
    b = 24;
    f();
    ASSERT(c == 23);

    a = 24;
    g();
    ASSERT(c == 1);
}

