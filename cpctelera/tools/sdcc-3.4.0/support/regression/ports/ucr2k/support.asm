        .area   _CODE
        .globl  _putchar
        .globl  _exit

__putchar::
        jp      _putchar

__initEmu::
        ret

__exitEmu::
        jp      _exit
