#include "hw.h"

void main(void)
{
  while (1)
    {
      P2_3= !P2_3;
      P3= P3^2;
    }
}
