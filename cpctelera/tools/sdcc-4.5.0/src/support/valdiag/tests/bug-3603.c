/* bug-3464.c

   Issues in the diagnostics for attempting to take the address of the result of an unary operator.
 */

#ifdef TEST1
int *p(int i)
{
    return &(-i); /* ERROR */
}
#endif

