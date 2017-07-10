/*
   regtrack.c - testing register tracking
*/

#include <testfwk.h>
#include <string.h>

volatile unsigned char __xdata t;

__pdata      unsigned char ta[] = {0x00,0x01,0x01,0x02,0x01,0xfe,0x7f,0xfe,0xef};
__code const unsigned char tb[] = {0x00,0x01,0x01,0x02,0x01,0xfe,0x7f,0xfe,0xef};

static void 
foo (unsigned char which)
{
  unsigned char i, k;  // should be allocated to registers
  volatile unsigned char m = 1;

  k = 2;
  do
    {
      t = 0xab;
      i = 2;
      do
        {
          switch( which )
            {
            case 1:
              k = 1;
              t = 1;    // mov
              break;

            case 2:
              t = 0x01; 
              t = 0x02; // inc
              break;

            case 3:
              t = 0x05;
              t = 0x04;
              t = 0x03; // dec
              break;

            case 4:
              t = ~0x04;
              t = 0x04; // cpl
              break;

            case 5:
              t = 0x05 << 1;
              t = 0x05; // rr
              break;

            case 6:
              t = 0x06 >> 1;
              t = 0x06; // rl
              break;

            case 7:
              t = 0x70;
              t = 0x07; // swap
              break;

            case 0x08: 
              t = 0x0a;
              k = 0x02;
              t = 0x08; // xrl
              break;

            case 0x09: 
              t = 0x0f;
              k = 0xf9;
              t = 0x09; // anl
              break;

            case 0x0a: 
              t = 0x08;
              k = 0x02;
              t = 0x0a; // orl
              break;

            case 0x0b: 
              t = 0x0b * 7;
              k = 7;
              t = t/7;  // div
              break;

            case 0x0c: 
              t = 4;
              k = 3;
              t = t * 3;  // mul
              break;
            }
        }
      while (--i);
      
      if (!i)
        k = m; // prepare to exit outer loop
    }
  while (--k);

}




void 
testRegTrack (void)
{
  ASSERT (0 == (char)memcmp (ta, tb, sizeof tb));

  foo (1); ASSERT (t == 1);
  foo (2); ASSERT (t == 2);
  foo (3); ASSERT (t == 3);
  foo (4); ASSERT (t == 4);
#if 1  
  /* various checks for equality */
  foo (5); ASSERT (!(t ^ 5));
  foo (6); ASSERT (0 == (t ^ 6));
  foo (7); ASSERT (!(t - 7));
  foo (8); ASSERT (0 == (t - 8));
  foo (9); ASSERT (0 == ((unsigned char)(t + (0x100 - 9))));
  foo (10); ASSERT (!((unsigned char)(t + (0x100 - 10))));
  foo (11); ASSERT (t >= 11 && t <= 11);
  foo (12); ASSERT (t > 11 && t < 13);
#else  
  foo (5); ASSERT (t == 5);
  foo (6); ASSERT (t == 6);
  foo (7); ASSERT (t == 7);
  foo (8); ASSERT (t == 8);
  foo (9); ASSERT (t == 9);
  foo (10); ASSERT (t == 10);
  foo (11); ASSERT (t == 11);
  foo (12); ASSERT (t == 12);
#endif
}

