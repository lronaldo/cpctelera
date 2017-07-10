
/* bug-895992.c

   Life Range problem with
   - uninitialized variable
   - loop
   - conditional block
 */

#ifdef TEST1
char p0;

void wait (void);

void foo(void)
{
  unsigned char number;
  unsigned char start = 1;
  unsigned char i;

  do
    {
      for (i = 1; i > 0 ; i--)
        wait();
      if (start)
        {
          number = p0;
          start = 0;
        }
      number--;		/* WARNING(SDCC) */
    }
  while (number != 0);
}
#endif
