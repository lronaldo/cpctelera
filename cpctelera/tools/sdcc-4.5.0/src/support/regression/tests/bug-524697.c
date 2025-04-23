/* Bad addition for adding a length and char[]
 */
#include <testfwk.h>

#if defined(__SDCC_mcs51)
#define SZ_SIZE 89
#else
#define SZ_SIZE 90
#endif

#if !defined(__SDCC_pdk14) && !defined(__SDCC_pic14) // Lack of memory
typedef struct _Foo
{
  char sz[SZ_SIZE];
} Foo;

typedef struct _Bar
{
  unsigned int uLen;
} Bar;

char *getOffset(Foo *pFoo, Bar *pBar)
{
  return pFoo->sz + pBar->uLen;
}
#endif

void
testOffset(void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pic14) // Lack of memory
  Foo foo = {
    "Foo"
  };
  Bar bar = {
    3
  };

  ASSERT(getOffset(&foo, &bar) 
	 == (((char *)&foo) + 3));
#endif
}
