/** Tests the basic logical operations.

    type: char, int, long
    storage: static, 
    attr: volatile
    values: 5, 350, 31734
 */
#include <testfwk.h>

static {type}
alwaysTrue(void)
{
    return ({type}){values};
}

static {type}
alwaysFalse(void)
{
    return 0;
}

static {type}
neverGetHere1(void)
{
    FAILM("Shouldn't get here 1");
    return 0;
}

static {type}
neverGetHere2(void)
{
    FAILM("Shouldn't get here 2");
    return 0;
}

static {type}
neverGetHere3(void)
{
    FAILM("Shouldn't get here 3");
    return 0;
}

static int hit;

static void
resetGetHere(void)
{
    hit = 0;
}

static {type}
alwaysGetHere(void)
{
    hit++;
    return 1;
}

static void
testLogicalAnd(void)
{
    {type} true = alwaysTrue();
    {type} false = alwaysFalse();

    ASSERT(true);
    ASSERT(!false);
    ASSERT(true && true && true);
    ASSERT(true && !false);
    ASSERT(!false && true);

    /* Test that the evaluation is aborted on the first false. */
    if (true && false && neverGetHere1()) {
        /* Tested using neverGetHere1() */
    }

    /* Alternate that is similar. */
    if (true && false) {
        neverGetHere2();
        /* Tested using neverGetHere2() */
    }

    resetGetHere();
    /* Test that the evaluation is done left to right. */
    if (alwaysGetHere() && true && false) {
        ASSERT(hit == 1);
    }
}

static void
testLogicalOr(void)
{
    {type} true = alwaysTrue();
    {type} false = alwaysFalse();

    ASSERT(true);
    ASSERT(!false);
    ASSERT(false || false || true);
    ASSERT(!true || !false);
    ASSERT(false || true);

    /* Test that the evaluation is aborted on the first hit. */
    if (false || true || neverGetHere3()) {
        /* Tested using neverGetHere3() */
    }

    resetGetHere();
    /* Test that the evaluation is done left to right. */
    if (alwaysGetHere() || true || false) {
        ASSERT(hit == 1);
    }
}

static void
testNot(void)
{
    {type} true = alwaysTrue();
    {type} false = alwaysFalse();

    ASSERT(!false);
    ASSERT(!!true);
    ASSERT(!!!false);
}

static void
testFlagToVariable(void)
{
    {type} true = alwaysTrue();
    {type} false = alwaysFalse();
    {type} val = !true;

    ASSERT(!val);
    val = !!false;
    ASSERT(!false);
}
