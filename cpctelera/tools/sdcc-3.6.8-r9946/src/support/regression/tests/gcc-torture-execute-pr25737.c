/*
   pr25737.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

struct delay_block {
  struct delay_block *succ;
};

static struct delay_block Timer_Queue;

struct delay_block* time_enqueue (struct delay_block *d)
{
  struct delay_block *q = Timer_Queue.succ; 
  d->succ =  (void *)0;
  return Timer_Queue.succ;
}

void
testTortureExecute (void)
{
  Timer_Queue.succ = &Timer_Queue;
  if (time_enqueue (&Timer_Queue) != (void*)0)
    ASSERT (0);
  return;
}

