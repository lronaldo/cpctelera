/* bug-3457.c

   Wrong line number on divide by 0 warning

   Based on GPL 2.0 code by "Under4MHz"
 */

#ifdef TEST1
int abs (int);

void test(void) {

    int Y = 0;

    int inf = abs( 1 / Y ); /* WARNING(SDCC) */ /* IGNORE(GCC) */
    int a = 0;
    int b = 0;
    int c = 0;
    for(int i = 0; i < 10; i++)
    {
        c++;
    }
}
#endif

