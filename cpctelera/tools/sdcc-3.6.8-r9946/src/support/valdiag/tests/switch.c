
char x;

/* Valid switch statement */
#ifdef TEST1
char foo(void)
{
  switch(x)
    {
      char y;
      
      case 0:
        return 0;
      case 1:
        return 1;
      default:
        y = x+1;
        return y;
    }
}
#endif

/* Error, duplicate cases */
#ifdef TEST2
char foo(void)
{
  switch(x)
    {
      char y;
      
      case 0:		/* IGNORE */
        return 0;
      case 1:
        return 1;
      case 0:		/* ERROR */
        return 0;
      default:
        y = x;
        return y;
    }
}
#endif

/* Error, more than one default */
#ifdef TEST3
char foo(void)
{
  switch(x)
    {
      char y;
      
      case 0:
        return 0;
      case 1:
        return 1;
      default:		/* IGNORE */
        y = x;
        return y;
      default:		/* ERROR */
        return 2;
    }
}
#endif

/* Warn about unreachable code */
#ifdef TEST4
char foo(void)
{
  switch(x)
    {
      char y;
      x++;		/* WARNING(SDCC) */
      
      case 0:
        return 0;
      case 1:
        return 1;
      default:
        y = x;
        return x;
    }
}
#endif

/* Warn about unreachable initializer */
#ifdef TEST5
char foo(void)
{
  switch(x)
    {
      char y=1;		/* WARNING(SDCC) */
      
      case 0:
        return 0;
      case 1:
        return 1;
      default:
        return y;	/* IGNORE */
    }
}
#endif

/* Error, missing switch */
#ifdef TEST6
char foo(void)
{
    {
      case 0:		/* ERROR */
        return 0;
      case 1:		/* ERROR */
        return 1;
      default:		/* ERROR */
        return x;
    }
}
#endif

/* Error, switch condition must be integral */
#ifdef TEST7
char foo(void)
{
  float f;
  f=x;
  switch(f)		/* ERROR */
    {
      char y;
      
      case 0:
        return 0;
      case 1:
        return 1;
      default:
        y = x;
        return x;
    }
  return 0;
}
#endif

/* Error, cases must be integral */
#ifdef TEST8
char foo(void)
{
  switch(x)
    {
      char y;
      
      case 0.0:		/* ERROR */
        return 0;
      case 1:
        return 1;
      default:
        y = x;
        return x;
    }
}
#endif

