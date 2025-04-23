
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
      char y; /* IGNORE */
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
  float f; /* IGNORE */
  f=x;
  switch(f)		/* ERROR */
    {
      char y; /* IGNORE */
      
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
      char y; /* IGNORE */
      
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

#ifdef TEST9
_Pragma("save")
_Pragma("std_c23")

/* Error: 'case' range expressions require C2y or later */
char foo(void)
{
  switch(x)
    {
      case 1 ... 3:     /* ERROR */
        return 1;
      default:
        return 0;
    }
}

_Pragma("restore")
#endif

#ifdef TEST10
_Pragma("save")
_Pragma("std_c2y")

/* Same code as above, but valid in C2y */
char foo(void)
{
  switch(x)
    {
      case 1 ... 3:
        return 1;
      default:
        return 0;
    }
}

_Pragma("restore")
#endif

#ifdef TEST11
_Pragma("save")
_Pragma("std_c2y")

/* Error: overlap with case range */
char foo(void)
{
  switch(x)
    {
      case 1:           /* IGNORE */
        return 1;
      case 1 ... 3:     /* ERROR */
        return 3;
      default:
        return 0;
    }
}

_Pragma("restore")
#endif

#ifdef TEST12
_Pragma("save")
_Pragma("std_c2y")

/* Warning: 'case' range empty; case ignored */
char foo(void)
{
  switch(x)
    {
      case 1:
        return 1;
      case 3 ... 1:     /* WARNING(SDCC) */
        return 3;       /* IGNORE */ /* unreachable code warning */
      default:
        return 0;
    }
}

_Pragma("restore")
#endif
