        .area   _CODE
        .globl  _exit

__putchar::
		ld	hl, #2
		add	hl, sp
		ld	l, (hl)
		ld	a, #1
		rst	0x28
		ret

__initEmu::
        ret

__exitEmu::
        jp      _exit
