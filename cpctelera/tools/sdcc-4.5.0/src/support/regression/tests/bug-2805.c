/* bug-2822.c
   A division optimization evaluated operands to double precision only, thus losing information on long long variables.
 */

#include <testfwk.h>

#include <limits.h>

unsigned long long f(void)
{
    unsigned long long left = (ULLONG_MAX - 2);
    return(left / (ULLONG_MAX - 3));
}

unsigned long long g(void)
{
    unsigned long long left = (ULLONG_MAX - 2);
    return(left % (ULLONG_MAX - 3));
}

unsigned long long fs(void)
{
    unsigned long long left = (LLONG_MAX - 2);
    return(left / (LLONG_MAX - 3));
}

unsigned long long gs(void)
{
    unsigned long long left = (LLONG_MAX - 2);
    return(left % (LLONG_MAX - 3));
}

void testBug(void)
{
    ASSERT (f() == (ULLONG_MAX - 2) / (ULLONG_MAX - 3));
    ASSERT (g() == (ULLONG_MAX - 2) % (ULLONG_MAX - 3));
    ASSERT (fs() == (LLONG_MAX - 2) / (LLONG_MAX - 3));
    ASSERT (gs() == (LLONG_MAX - 2) % (LLONG_MAX - 3));
}

