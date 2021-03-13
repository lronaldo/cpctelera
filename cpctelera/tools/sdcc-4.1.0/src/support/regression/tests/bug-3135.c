/* bug-3135.c
   A stack access overwrote a value held in hl on gbz80.
 */
 
#include <testfwk.h>

typedef unsigned char UBYTE;

#define MAX_PROJECTILES 2
#define MAX_ACTIVE_ACTORS 3
#define MAX_ACTORS 4

typedef struct _BankPtr
{
  unsigned char bank;
  unsigned int offset;
} BankPtr;

typedef struct _PROJECTILE {
  UBYTE col_group;
} Projectile;

typedef struct {
  BankPtr hit_1_ptr;
  BankPtr hit_2_ptr;
} Actor;

Projectile projectiles[MAX_PROJECTILES];
UBYTE actors_active[MAX_ACTIVE_ACTORS];
Actor actors[MAX_ACTORS];

void TestFn2(BankPtr* events_ptr) {
  ASSERT (events_ptr->bank == 5 && events_ptr->offset == 0xFC00);
}

void TestFn() {
  UBYTE hit;
  UBYTE i;

  for (i = 0; i != 1; i++) {
      hit = actors_active[0];
      if (hit != 0xFF) {
        if (projectiles[i].col_group == 2) {
            TestFn2(&actors[hit].hit_1_ptr);
        } else if (projectiles[i].col_group == 4) {
            TestFn2(&actors[hit].hit_2_ptr);
        }
      }
  }
}

void
testBug(void)
{
  projectiles[0].col_group = 2;
  actors_active[0] = 0;
  actors[0].hit_1_ptr.bank = 5;
  actors[0].hit_1_ptr.offset = 0xFC00;

  TestFn();
}

