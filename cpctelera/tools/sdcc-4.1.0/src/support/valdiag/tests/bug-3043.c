/* bug-3043.c

   stm8 and pdk code generator hang on shift by negative literal
 */

#ifdef TEST1
void g(void)
{
}

void fr(int x)
{
  if (((x >> -1))) /* WARNING */
    g();            /* IGNORE */
}

void fl(int x)
{
  if (((x << -1))) /* WARNING */
    g();           /* IGNORE */
}
#endif

