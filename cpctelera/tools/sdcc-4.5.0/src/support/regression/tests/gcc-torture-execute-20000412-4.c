/*
   20000412-4.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

  void f(int i, int j, int radius, int width, int N)
  {
    const int diff   = i-radius;
    const int lowk   = (diff>0 ? diff : 0 );
    int k;
  
    for(k=lowk; k<= 2; k++){
      int idx = ((k-i+radius)*width-j+radius);
      if (idx < 0)
	ASSERT (0);
    }
  
    for(k=lowk; k<= 2; k++);
  }
  
  
  void
  testTortureExecute (void)
  {
    int exc_rad=2;
    int N=8;
    int i;
    for(i=1; i<4; i++)
      f(i,1,exc_rad,2*exc_rad + 1, N);
    return;
  } 

