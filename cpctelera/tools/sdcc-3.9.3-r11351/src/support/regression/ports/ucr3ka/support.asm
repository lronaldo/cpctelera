        .area   _CODE
        .globl  _exit

SADR=0xC0
        
__putchar::
        ioi
        ld      (SADR),a
        ret

__initEmu::
        ret

__exitEmu::
        jp      _exit
