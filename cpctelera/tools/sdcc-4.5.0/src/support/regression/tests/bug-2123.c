/*
   bug-2123.c
 */

#include <testfwk.h>
struct {
  char *p;
  char ct[2];
} arr[] = {
  {arr[1].ct, "a"},
  {arr->ct, "b"},
  {(*arr).ct, "c"},
  {(&arr[2])->ct, "d"},
  {(*(&arr[3])).ct, "e"},
  {(&(*(&arr[4])))->ct, "f"},
};


void testBug(void)
{
  ASSERT (*(arr[0].p) == 'b');
  ASSERT (*(arr[1].p) == 'a');
  ASSERT (*(arr[2].p) == 'a');
  ASSERT (*(arr[3].p) == 'c');
  ASSERT (*(arr[4].p) == 'd');
  ASSERT (*(arr[5].p) == 'e');
}
