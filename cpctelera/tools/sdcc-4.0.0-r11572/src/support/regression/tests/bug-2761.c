/* bug-2761.c
   Incomplete use vector from lospre confused live range separation
 */

#include <testfwk.h>

int *dot;
int *dol;
int fchange;

int array[4] = {0, 1, 2, 3};

void rdelete(int *ad1, int *ad2)
{
    register int  *a1, *a2, *a3;

    a1 = ad1;
    a2 = ad2+1;
    a3 = dol;
    dol -= a2 - a1;
    do {
        *a1++ = *a2++;
    } while (a2 <= a3);
    a1 = ad1;
    if (a1 > dol)
        a1 = dol;
    dot = a1;
    fchange = 1;
}

void testBug(void)
{
  dol = array + 2;
  rdelete(array + 0, array + 0);

  ASSERT(array[0] == 1);
  ASSERT(array[1] == 2);
  ASSERT(array[2] == 2);
  ASSERT(array[3] == 3);

  ASSERT(dot == array + 0);
}

