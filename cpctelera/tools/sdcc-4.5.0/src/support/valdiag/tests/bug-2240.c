/* bug-2240.c

   Missing error and on attemping to assign a value to a increment / decrement expression.
 */

#ifdef TEST1
char E=0, b=1;

char *p;

void g(void)
{
  (p++)[1] = b;
}

void f(void)
{
  ++E = b; /* ERROR */
}
#endif

#ifdef TEST2
char E=0, b=1;

char *p;

void g(void)
{
  (--p)[1] = b;
}

void f(void)
{
  E-- = b; /* ERROR */
}
#endif

