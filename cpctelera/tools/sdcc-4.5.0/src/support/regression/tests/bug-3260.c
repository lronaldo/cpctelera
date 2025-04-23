#include <testfwk.h>

/* Bug #3260 caused typecast function pointers to pass all */
/* parameters via the stack and not pass any parameter via  */
/* register, even when some should have been passed via    */
/* register. */

/* Look for __SDCC_STACK_AUTO to make sure the port is     */
/* parameters in registers and/or stack and not statically */
#if defined(__SDCC_STACK_AUTO) || defined(PORT_HOST)
int (*p)(int, int) __reentrant;

void (*q)(void) __reentrant;

int f(void)
{
    return (*p)(23, 42);
}

int g(void)
{
    return (*(((int (*)(int, int))q)))(23, 42);
}

int c(int a, int b) __reentrant
{
    return a - b;
}
#endif

void
testBug(void)
{
#if defined(__SDCC_STACK_AUTO) || defined(PORT_HOST)
    p = &c;
    q = (void (*)(void))(&c);
    ASSERT (f() == 23 - 42);
    ASSERT (g() == 23 - 42);
#endif
}

