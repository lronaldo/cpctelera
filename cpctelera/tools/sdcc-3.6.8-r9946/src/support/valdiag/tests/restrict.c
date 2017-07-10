
/* The restrict keyword can only qualify pointers */

#ifdef TEST1_C99
restrict a;		/* ERROR */
#endif

#ifdef TEST2_C99
restrict int a;		/* ERROR */
#endif

#ifdef TEST3_C99
restrict int a[10];	/* ERROR */
#endif

#ifdef TEST4_C99
restrict int * a;	/* ERROR */
#endif

#ifdef TEST5_C99
restrict struct
  {
    int a;
    int b;
  } x;			/* ERROR */
#endif

#ifdef TEST6_C99
restrict int func(void) {	/* ERROR */
  return 0;
}
#endif

#ifdef TEST7_C99
void func(restrict int x) {	/* ERROR */
  x;				/* IGNORE */
}
#endif


#ifdef TEST_GOOD1_C99
int * restrict a;
#endif

#ifdef TEST_GOOD2_C99
int * func(int * restrict x)
{
  return x;
}
#endif

#ifdef TEST_GOOD3_C99
void func(int * restrict x)
{
  x;				/* IGNORE */
}
#endif
