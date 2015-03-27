/** memory function test
*/
#include <testfwk.h>

#include <string.h>

unsigned char destination[4];
const unsigned char source[4] = {0, 1, 2, 3};
int c;

void testmemory(void)
{
  volatile size_t zero = 0;
  volatile size_t one = 1;

  ASSERT(source[0] == 0);

  /* Test memset() */
  c = 8;
  memset(destination, c, one);
  ASSERT(destination[0] == 8);
  memset(destination, c, 3);
  ASSERT(destination[2] == 8);
  destination[3] = 23;
  memset(destination, 42, 3);
  ASSERT(destination[0] == 42);
  ASSERT(destination[2] == 42);
  ASSERT(destination[3] == 23);
  memset(destination, 23, 1);
  ASSERT(destination[0] == 23);
  ASSERT(destination[1] == 42);
  memset(destination, 42, 1);

  /* Test memcpy() */
  memcpy(destination, source, 0);
  ASSERT(destination[0] == 42);
  memcpy(destination, source, zero);
  ASSERT(destination[0] == 42);
  memcpy(destination + 1, source + 1, 2);
  ASSERT(destination[0] == 42);
  ASSERT(destination[2] == source[2]);
  ASSERT(destination[3] == 23);
  memcpy(destination, source, one);
  ASSERT(destination[0] == 0);

  /* Test memcmp() */
  memcpy(destination, source, 4);
  ASSERT(memcmp(destination, source, 4) == 0);

  /* Test memmove() */
  memcpy(destination, source, 4);
  memmove(destination, destination + 1, 3);
  ASSERT(destination[0] == source[1]);
  ASSERT(destination[2] == source[3]);
  ASSERT(destination[3] == source[3]);
  memcpy(destination, source, 4);
  memmove(destination + 1, destination, 3);
  ASSERT(destination[0] == source[0]);
  ASSERT(destination[1] == source[0]);
  ASSERT(destination[3] == source[2]);

  /* Test memchr() */
  ASSERT(NULL == memchr(destination, 5, 4));
  /*ASSERT(destination == memchr(destination, 0, 4));
  ASSERT(destination + 3 == memchr(destination, 3, 4));*/

  ASSERT(strlen("test") == 4);
  ASSERT(strlen("t") == 1);
  ASSERT(strlen("") == 0);
}

