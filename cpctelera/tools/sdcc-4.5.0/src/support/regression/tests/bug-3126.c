/* bug-3126.c
   A still-needed pointer in register pair hl was overwritten when hl was used for a stack access on gbz80.
 */

#include <testfwk.h>

unsigned char test1;
unsigned char test2;

typedef struct _BankPtr
{
  unsigned char bank;
  unsigned int offset;
} BankPtr;

const BankPtr scene_bank_ptrs[] = {{0x06, 0x3C53}};

void TestFn2(unsigned char i)
{
  test2 = i;
}

void TestFn(unsigned int index)
{
  unsigned char bank;
  unsigned int data_ptr;

  bank = scene_bank_ptrs[index].bank;
  data_ptr = scene_bank_ptrs[index].offset;

  /* This section doesn't matter but the bug doesn't trigger without it */
  test1 = 1;
  TestFn2(bank);
  /**/

  ASSERT(data_ptr == 0x3C53);
}

void
testBug(void)
{
  TestFn(0);
}

