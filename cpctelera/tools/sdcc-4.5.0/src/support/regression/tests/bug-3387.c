/*
   bug-3387.c
   SDCC generates invalid opcode JP NZ,(HL)
 */

#include <testfwk.h>

void some_func(void)
{
}

void (*func_list[])(void) = { some_func };

void testBug(void)
{
    void (*p_func)(void) = func_list[0];
    if (p_func)
        p_func();
}
