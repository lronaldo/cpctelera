/* z80n-push-nn
   Invalid relocation of Z80N PUSH nn instruction value
 */
#include <testfwk.h>
#include <string.h>

int x;
typedef void* (*memcpy_p)(void *d, const void *s, size_t c);

#ifdef __SDCC_z80n
int f(void) __naked
{
    __asm
	push	#0x1234
	push	#_x
	pop	hl
	pop	de
	ld	a, (hl)
	inc	hl
	ld	h, (hl)
	ld	l, a
	add	hl, de
	ret
;
	.db	#>_x, #<_x
	.dw	#_x
    __endasm;
}
memcpy_p g(void) __naked
{
    __asm
	push	#_memcpy
	pop	hl
	ret
    __endasm;
}
#else
int f(void)
{
    return x + 0x1234;
}
memcpy_p g(void)
{
    return &memcpy;
}
#endif

void testBug3032(void)
{
    x = 0x7531;
    ASSERT (f() == 0x8765);
    ASSERT (g() == &memcpy);
}
