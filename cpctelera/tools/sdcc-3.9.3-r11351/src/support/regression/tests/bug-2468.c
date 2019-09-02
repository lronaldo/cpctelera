/*
   bug-2468.c
*/

#include <testfwk.h>

#pragma disable_warning 24

struct s1
{
  int g;
  unsigned char y;
};

struct s2
{
  int w;
  struct s1 p[2];
  struct s1 sm;
  struct s1 *ps1;
};

struct s1 vs1 = {63, 65};
struct s2 vs2 = {12, {{13, 15}, {45, 46}}, {34, 35}, &vs1};

void testBug (void)
{
  ASSERT (vs2.p[0].y == 15);
  ASSERT (vs2.sm.y == 35);
  ASSERT (vs1.y == 65);

  vs2.p->y++;
  vs2.p[2].y++;
  vs2.ps1->y++;

  ASSERT (vs2.p->y == 16);
  ASSERT (vs2.p[2].y == 36);
  ASSERT (vs2.ps1->y == 66);
}

