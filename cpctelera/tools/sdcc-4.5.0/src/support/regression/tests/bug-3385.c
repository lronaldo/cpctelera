/* bug-3385.c
   A bug in an stm8 peephole optimizer rule resulting in wrong code..
 */

#include <testfwk.h>

typedef struct list_node_t {
  struct list_node_t* next;
} list_node_t;

typedef struct list_t {
  list_node_t head;
} list_t;

typedef struct {
  int x;
  list_t list;
} event_t;

void event_init(event_t* self)
{
  self->list.head.next = &self->list.head;
}

void testBug(void)
{
#if !defined(__SDCC_hc08) && !defined(__SDCC_s08) && !defined(__SDCC_mos6502) && !defined(__SDCC_mos65c02) // Bug #3386
  event_t e;
  event_init (&e);
  ASSERT (e.list.head.next == &e.list.head);
#endif
}

