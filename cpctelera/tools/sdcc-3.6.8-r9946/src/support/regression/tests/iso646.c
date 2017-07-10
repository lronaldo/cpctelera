/** Test iso 646

    type: char, short, long
    attr: volatile,
    storage: static,
 */
#include <testfwk.h>

#include <float.h> /* bug #2314 was a conflict between iso646.h and float.h */
#include <iso646.h>

static void
testTwoOpBitwise(void)
{
  {storage} {attr} {type} left, right;

  left = ({type})0x3df7;
  right = ({type})0xc1ec;

  ASSERT(({type})(left bitand right) == ({type})0x1E4);
  ASSERT(({type})(right bitand left) == ({type})0x1E4);
  ASSERT(({type})(left bitand 0xc1ec) == ({type})0x1E4);
  ASSERT(({type})(0x3df7 bitand right) == ({type})0x1E4);

  ASSERT(({type})(left bitor right) == ({type})0xFDFF);
  ASSERT(({type})(right bitor left) == ({type})0xFDFF);
  ASSERT(({type})(left bitor 0xc1ec) == ({type})0xFDFF);
  ASSERT(({type})(0x3df7 bitor right) == ({type})0xFDFF);

  ASSERT(({type})(left xor right) == ({type})0xFC1B);
  ASSERT(({type})(right xor left) == ({type})0xFC1B);
  ASSERT(({type})(left xor 0xc1ec) == ({type})0xFC1B);
  ASSERT(({type})(0x3df7 xor right) == ({type})0xFC1B);
}

static {type}
alwaysTrue(void)
{
    return ({type})1;
}

static {type}
alwaysFalse(void)
{
    return 0;
}

static void
testNot(void)
{
    {type} true = alwaysTrue();
    {type} false = alwaysFalse();

    ASSERT(not false);
    ASSERT(not not true);
    ASSERT(not not not false);
	ASSERT(true not_eq false);
}

