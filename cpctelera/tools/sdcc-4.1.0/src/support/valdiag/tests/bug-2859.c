/* bug-2859.c
   Cannot compare function to 0.
 */

void a(void) {}

#ifdef TEST1
int b(void)
{
  /* compare function pointer to non-zero integer literal */
  return a != 1; /* ERROR */
}
#endif

#ifdef TEST1
int c(void)
{
  /* compare function pointer to 0 with an operator other than ==,!= */
  return a >= 0; /* ERROR */
}
#endif

#ifdef TEST1
int d(void)
{
  /* compare function pointer to 0 with an operator other than ==,!= */
  return a < 0; /* ERROR */
}
#endif
