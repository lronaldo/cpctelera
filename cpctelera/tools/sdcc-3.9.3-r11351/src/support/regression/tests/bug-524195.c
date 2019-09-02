/* Problem with inverting constants
 */
#include <testfwk.h>

enum 
  {
    USEDFLAG = 1
  };

typedef struct _HeapEntry HeapEntry;
typedef unsigned int UINT;

struct _HeapEntry
{
  HeapEntry *pPrev;
  HeapEntry *pNext;
  UINT uSize;
};

static UINT _getSize1(HeapEntry *pEnt)
{
  return pEnt->uSize & ~USEDFLAG;
}

static UINT _getSize2(HeapEntry *pEnt)
{
  return pEnt->uSize & ~1;
}

static UINT _getSize3(HeapEntry *pEnt)
{
  return pEnt->uSize & 0xFFFE;
}

static void
testMask(void)
{
  HeapEntry ent;
  ent.uSize = 123;

  ASSERT(_getSize1(&ent) == 122);
  ASSERT(_getSize2(&ent) == 122);
  ASSERT(_getSize3(&ent) == 122);

  ent.uSize = 0x1234;
  ASSERT(_getSize1(&ent) == 0x1234);
  ASSERT(_getSize2(&ent) == 0x1234);
  ASSERT(_getSize3(&ent) == 0x1234);
}
