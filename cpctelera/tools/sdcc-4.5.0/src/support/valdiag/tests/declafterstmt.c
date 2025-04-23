/* we are testing a C99 feature */
#pragma std_c99

/* we do not care about unreferenced variables */
#pragma disable_warning 85

#ifdef TEST1
#pragma std_c89
void test(void)
{
  int a = 0;
  a++;
  int b;
}               /* ERROR */ /* declaration after statement within this block */
#endif

#ifdef TEST2
void test(void)
{
  int a = 0;
  a++;
  int b;
}
#endif

#ifdef TEST3
void test(void)
{
  int a = 0;    /* ERROR */ /* previously defined here */
  a++;
  int a;        /* ERROR */ /* duplicate symbol */
}
#endif

#ifdef TEST4
void test(void)
{
  int a = 0;
  a++;
  int b;
  {
    int a;
  }
}
#endif
