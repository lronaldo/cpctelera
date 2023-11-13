/* bug-3011.c

   Crash on (not-yet-implemented as of mid-2020) compound literal.
 */

#ifdef TEST1
void f()
{
    int *p = (int *){1}; /* IGNORE */
}
#endif

