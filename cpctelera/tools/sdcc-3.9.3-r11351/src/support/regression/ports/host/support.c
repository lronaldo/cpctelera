/** Host specific support routines.
 */
#include <stdio.h>
#include <stdlib.h>

void
_putchar(char c)
{
    putchar(c);
}

void
_initEmu(void)
{
}

void
_exitEmu(void)
{
    exit(0);
}
