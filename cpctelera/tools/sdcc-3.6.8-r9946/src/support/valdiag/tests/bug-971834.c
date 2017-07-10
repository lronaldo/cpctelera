
/* bug-971834.c

   Life Range problem with
   - uninitialized variable
   - loop
 */

#ifdef TEST1
unsigned char ttt = 2;

short foo (void)
{
  unsigned short a;
  a |= ttt;		/* WARNING(SDCC) */
  return a;
}
#endif


#ifdef TEST2
unsigned char ttt[] = {0xff, 1};

char foo (void)
{
  unsigned char a, i;
  for (i = 0; i < sizeof(ttt); i++)
    a |= ttt[i];	/* WARNING(SDCC) */
  return a;		/* WARNING(SDCC) */
}
#endif

#ifdef TEST3
unsigned char ttt[] = {0xff, 1};
unsigned char b;

char foo (void)
{
  unsigned char i, j;
  unsigned char a;
  for (j = 0; j < sizeof(ttt); j++) {
    for (i = 0; i < sizeof(ttt); i++) {
      a |= ttt[i];	/* WARNING(SDCC) */
      b = a;
    }
  }
  return b;
}
#endif
