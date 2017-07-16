
int x;

#ifdef TEST0
void foo(void)
{
  while (1) ;
}
#endif

#ifdef TEST1
void foo(void)
{
  while (1) ;
  x++;		/* WARNING(SDCC) */
}
#endif

#ifdef TEST2
void foo(void)
{
  int y=1;
  while (y) ;	/* WARNING(SDCC) */
  x++;		/* WARNING(SDCC) */
}
#endif
