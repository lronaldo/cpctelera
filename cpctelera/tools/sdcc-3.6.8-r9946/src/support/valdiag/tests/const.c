
char a;
const char ca=2;
const char *pca;
char * const cpa=&a;

void test(void)
{
  a = 1;
#ifdef TEST1
  ca = a;		/* ERROR */
#endif
#ifdef TEST2
  pca = &a;
  *pca = 2;		/* ERROR */
#endif
#ifdef TEST3
  *cpa = 3;
  cpa = &ca;		/* ERROR */
#endif
}
