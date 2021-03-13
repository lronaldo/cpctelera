/* Simple malloc tests.
 */
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#if defined(__SDCC_pic16)
#include <malloc.h>
#endif
#include <testfwk.h>

#if defined(__SDCC_pic16)
__xdata char heap[100];
#endif

void mallocfree(void)
{
#if !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) // Lack of memory
	char *a, *b, *c;
	char d[25];

	LOG (("mallocfree()\n"));

	a = malloc(16);
	ASSERT(a);
	memset(a, 2, 16);
	b = malloc(16);
	ASSERT(b);
	memset(b, 1, 16);
	c = calloc(16, 1);
	ASSERT(c);
#ifndef PORT_HOST
	LOG (("1 a %u b %u c %u\n", (unsigned)a, (unsigned)b, (unsigned)c));
#endif
	memset(d, 2, 16);
	ASSERT(!memcmp(d, a, 16));
	memset(d, 1, 16);
	ASSERT(!memcmp(d, b, 16));
	memset(d, 0, 16);
	ASSERT(!memcmp(d, c, 16));

	free(b);
	b = malloc(20);
	memset(b, 3, 20);
#ifndef PORT_HOST
	LOG (("2 a %u b %u c %u\n", (unsigned)a, (unsigned)b, (unsigned)c));
#endif
	memset(d, 2, 16);
	ASSERT(!memcmp(d, a, 16));
	memset(d, 3, 20);
	ASSERT(!memcmp(d, b, 20));
	memset(d, 0, 16);
	ASSERT(!memcmp(d, c, 16));

	free(b);
	b = malloc(10);
	memset(b, 4, 10);
#ifndef PORT_HOST
	LOG (("3 a %u b %u c %u\n", (unsigned)a, (unsigned)b, (unsigned)c));
#endif
	memset(d, 2, 16);
	ASSERT(!memcmp(d, a, 16));
	memset(d, 4, 20);
	ASSERT(!memcmp(d, b, 10));
	memset(d, 0, 16);
	ASSERT(!memcmp(d, c, 16));

	free(b);
	b = malloc(8);
	memset(b, 5, 8);
	b = realloc(b, 4);
	memset(d, 5, 8);
	ASSERT(!memcmp(d, b, 4));
#ifndef PORT_HOST
	LOG (("4 a %u b %u c %u\n", (unsigned)a, (unsigned)b, (unsigned)c));
#endif

	free(a);
	b = realloc(b, 8);
	ASSERT(!memcmp(d, b, 4));

#ifndef PORT_HOST
	LOG (("5 a %u b %u c %u\n", (unsigned)a, (unsigned)b, (unsigned)c));
#endif
	free(b);
#ifndef PORT_HOST
	LOG (("6 a %u b %u c %u\n", (unsigned)a, (unsigned)b, (unsigned)c));
#endif
	free(c);
#ifndef PORT_HOST
	LOG (("7 a %u b %u c %u\n", (unsigned)a, (unsigned)b, (unsigned)c));
#endif

	a = malloc(10);
	memset(a, 6, 10);
	b = malloc(10);
	memset(b, 7, 10);
	c = malloc(10);
	memset(c, 8, 10);
#ifndef PORT_HOST
	LOG (("8 a %u b %u c %u\n", (unsigned)a, (unsigned)b, (unsigned)c));
#endif

	free(b);
	a = realloc(a, 25);
	memset(a + 10, 6, 15);
#ifndef PORT_HOST
	LOG (("9 a %u b %u c %u\n", (unsigned)a, (unsigned)b, (unsigned)c));
#endif

	memset(d, 6, 25);
	ASSERT(!memcmp(d, a, 25));
	memset(d, 8, 10);
	ASSERT(!memcmp(d, c, 10));

	free(a);
	free(c);

	/* Check that we can allocate at least 256 bytes at once. */
#if defined(PORT_HOST) || defined(__SDCC_z80) || defined(__SDCC_z180) || defined(__SDCC_r2k) || defined(__SDCC_r2ka) || defined(__SDCC_r3ka)
	a = malloc(256);
	ASSERT(a);
	free(a);
#endif

	/* Check for overflow in calloc() memory size calculation,  bug #2650. */
#ifndef PORT_HOST
	c = calloc(SIZE_MAX / 256, 258);
	ASSERT(!c);
#endif
#endif
}

void
testMalloc (void)
{
#if !(defined (__SDCC_pdk15) && defined(__SDCC_STACK_AUTO)) // Lack of code memory
#ifndef __SDCC_pdk14
  void __xdata *p1, *p2, *p3;
  char *p;
  unsigned char i;


#if defined(__SDCC_pic16)
  _initHeap (heap, sizeof heap);
#endif

#ifndef PORT_HOST
  p1 = malloc (0xFFFF);
  ASSERT (p1 == NULL);
  LOG (("p1 == NULL when out of memory\n"));
#endif

#if !defined(PORT_HOST) && !defined(__SDCC_gbz80) && !defined(__SDCC_z80)
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

  free (p1);
  free (p3);
  mallocfree();
#endif
#endif
}

