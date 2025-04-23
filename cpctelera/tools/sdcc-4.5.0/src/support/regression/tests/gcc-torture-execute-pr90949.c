/*
   pr90949.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma disable_warning 85
#endif

void my_puts (const char *str) { }

void my_free (void *p) { }


struct Node
{
  struct Node *child;
};

struct Node space[2] = { {0} };

struct Node *my_malloc (int bytes)
{
  return &space[0];
}

void
walk (struct Node *module, int cleanup)
{
  if (module == 0)
    {
      return;
    }
  if (!cleanup)
    {
      my_puts ("No cleanup");
    }
  walk (module->child, cleanup);
  if (cleanup)
    {
      my_free (module);
    }
}

void
testTortureExecute (void)
{
  struct Node *node = my_malloc (sizeof (struct Node));
  node->child = 0;
  walk (node, 1);
}
