/*
   bug-2313.c - didn't compile (validateLink failed due to bug in jump-after-cast-optimization).
 */

#include <testfwk.h>

#pragma disable_warning 85

#pragma std_c99

#include <stdint.h>

typedef uint16_t usize_t;
typedef int16_t arg_t;

typedef struct u_data {
    arg_t       u_argn1;
} u_data;

usize_t valaddr(const char *base, usize_t size)
{
	return(0);
}

struct u_data udata;

#define statloc (int __data *)udata.u_argn1

void f(void)
{
	statloc && !valaddr((char *) statloc, sizeof(int));
}

void testBug(void)
{
	f();
}
