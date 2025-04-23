/* bug-3368.c
   An issue in the handling of array elements passed as struct parameters.
 */
 
#include <testfwk.h>

#include <stdarg.h>

#pragma disable_warning 85

typedef struct{int x,y;}ipoint;
ipoint ipts1[]={{1,2},{3,4}};

static int va1(int nargs,ipoint i,...);

void
f1 (void)
{
	va1(4,ipts1[0],ipts1[1]);
}

static int va2(int nargs,...);

void
f2 (void)
{
	va2(4,ipts1[0],ipts1[1]);
}

void testBug(void)
{
	f1();
	f2();
}

static int va1(int nargs,ipoint i1,...)
{
	va_list args;
	ipoint i2;

	ASSERT(i1.x == 1);
	ASSERT(i1.y == 2);

	va_start(args,i1);

	i2 = va_arg(args,ipoint);
	ASSERT(i2.x == 3);
	ASSERT(i2.y == 4);
}

static int va2(int nargs,...)
{
	va_list args;
	ipoint i1, i2;

	va_start(args,nargs);

	i1 = va_arg(args,ipoint);
	ASSERT(i1.x == 1);
	ASSERT(i1.y == 2);

	i2 = va_arg(args,ipoint);
	ASSERT(i2.x == 3);
	ASSERT(i2.y == 4);
}

