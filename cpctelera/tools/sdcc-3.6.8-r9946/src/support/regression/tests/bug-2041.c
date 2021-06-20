/*
   bug-2041.c
   pad: 254,255
 */

#include <testfwk.h>

#if defined (__SDCC_mcs51) || defined (__SDCC_ds390) || (__SDCC_ds400)

struct slist
{
  struct slist __xdata *next;
  unsigned short value;
};

extern void slist_init (struct slist __xdata *p);

void slist_init (struct slist __xdata *p)
{
  p->next = p;
}

unsigned char __xdata pad[{pad}];
struct slist __xdata list;

#endif

void testBug (void)
{
#if defined (__SDCC_mcs51) || defined (__SDCC_ds390) || (__SDCC_ds400)
  slist_init(&list);
  list.value = 0x5555;
  ASSERT (list.next->value == 0x5555);
  list.next->value = 0x3333;
  ASSERT (list.next->next->value == 0x3333);
#endif
}

