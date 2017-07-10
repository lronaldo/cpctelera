/** Tests argument passing to functions via va_args.
    Assumes that up to the first two arguments can be passed in registers.

    type1: va_char, int
    type2: va_char, int
    type3: va_char, int
 */
#include <testfwk.h>
#include <stdarg.h>

/* gcc 3.3 throws a warning, if char is in '...' */
#if defined(PORT_HOST)
# define va_char int
#else
# define va_char char
#endif

#ifndef __SDCC_pic16
static {type1}
returnFirstArg(int marker, ...)
{
    va_list ap;
    {type1} i;

    va_start(ap, marker);
    i = va_arg(ap, {type1});

    va_end(ap);

    LOG(("Returning %d\n", i));
    return i;
}

static {type2}
returnSecondArg(int marker, ...)
{
    va_list ap;
    {type2} i;

    va_start(ap, marker);
    UNUSED(va_arg(ap, {type1}));
    i = va_arg(ap, {type2});

    va_end(ap);

    LOG(("Returning %d\n", i));
    return i;
}

static {type2}
returnSecondArgCopy(int marker, ...)
{
    va_list ap1, ap2;
    {type2} i;

    va_start(ap1, marker);
    UNUSED(va_arg(ap1, {type1}));
    va_copy(ap2, ap1);
    i = va_arg(ap2, {type2});

    va_end(ap1);
    va_end(ap2);

    LOG(("Returning %d\n", i));
    return i;
}

static {type3}
returnThirdArg(int marker, ...)
{
    va_list ap;
    {type3} i;

    va_start(ap, marker);
    UNUSED(va_arg(ap, {type1}));
    UNUSED(va_arg(ap, {type2}));
    i = va_arg(ap, {type3});

    va_end(ap);

    LOG(("Returning %d\n", i));
    return i;
}
#endif

void
testArgs(void)
{
#ifndef __SDCC_pic16
    int marker = 12;

    LOG(("First arg: %u\n", returnFirstArg(marker, ({type1})123, ({type2})45, ({type3})67)));
    ASSERT(returnFirstArg(marker, ({type1})123, ({type2})45, ({type3})67) == ({type1})123);
    ASSERT(returnFirstArg(marker, ({type1})-123, ({type2})45, ({type3})67) == ({type1})-123);

    ASSERT(returnSecondArg(marker, ({type1})1, ({type2})-23, ({type3})64) == ({type2})-23);
    ASSERT(returnSecondArg(marker, ({type1})1, ({type2})8, ({type3})64) == ({type2})8);
    
    ASSERT(returnSecondArgCopy(marker, ({type1})1, ({type2})-23, ({type3})64) == ({type2})-23);
    ASSERT(returnSecondArgCopy(marker, ({type1})1, ({type2})8, ({type3})64) == ({type2})8);

    ASSERT(returnThirdArg(marker, ({type1})-33, ({type2})-34, ({type3})-35) == ({type3})-35);
    ASSERT(returnThirdArg(marker, ({type1})-33, ({type2})-34, ({type3})35) == ({type3})35);
#endif
}

