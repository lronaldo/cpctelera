/* Simple malloc tests.
 */
#include <stdlib.h>
#if defined(__SDCC_pic16)
#include <malloc.h>
#endif
#include <testfwk.h>

#if defined(__SDCC_pic16)
__xdata char heap[100];
#endif

void
testMalloc (void)
{
#ifndef __SDCC_stm8
  void __xdata *p1, *p2, *p3;
  char *p;
  unsigned char i;

#if !defined(PORT_HOST) && !defined(__SDCC_gbz80) && !defined(__SDCC_z80)
#if defined(__SDCC_pic16)
  _initHeap (heap, sizeof heap);
#endif

  p1 = malloc (2000);
  ASSERT (p1 == NULL);
  LOG (("p1 == NULL when out of memory\n"));
#ifdef PORT_HOST
  LOG (("p1: %p\n", p1));
#else
  LOG (("p1: %u\n", (unsigned) p1));
#endif
#endif

  p1 = malloc (5);
  ASSERT (p1 != NULL);
#ifdef PORT_HOST
  LOG (("p1: %p\n", p1));
#else
  LOG (("p1: %u\n", (unsigned) p1));
#endif

  p2 = malloc (20);
  ASSERT (p2 != NULL);
#ifdef PORT_HOST
  LOG (("p2: %p\n", p2));
#else
  LOG (("p2: %u\n", (unsigned) p2));
#endif

  p = (char*)p2;
  for (i = 0; i < 20; i++, p++)
    *p = i;

  p2 = realloc (p2, 25);
  ASSERT (p2 != NULL);
#ifdef PORT_HOST
  LOG (("p2, after expanding realloc: %p\n", p2));
#else
  LOG (("p2, after expanding realloc: %u\n", (unsigned) p2));
#endif

  p = (char*)p2;
  for (i = 0; i < 20; i++, p++)
    ASSERT (*p == i);

  p2 = realloc (p2, 15);
  ASSERT (p2 != NULL);
#ifdef PORT_HOST
  LOG (("p2, after shrinking realloc: %p\n", p2));
#else
  LOG (("p2, after shrinking realloc: %u\n", (unsigned) p2));
#endif

  p = (char*)p2;
  for (i = 0; i < 15; i++, p++)
    ASSERT (*p == i);

  free (p2);

  p3 = malloc (10);
  ASSERT (p3 != NULL);
#ifdef PORT_HOST
  LOG (("p3, after freeing p2: %p\n", p3));
#else
  LOG (("p3, after freeing p2: %u\n", (unsigned) p3));
#endif
#endif
}

