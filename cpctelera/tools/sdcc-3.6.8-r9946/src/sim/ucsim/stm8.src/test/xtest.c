#include <stdio.h>

char buf[10];

void
main(void)
{
  sprintf(buf, "%x", 0x1234);
  for (;;);
}
