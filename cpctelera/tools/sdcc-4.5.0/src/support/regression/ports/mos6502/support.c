
// sim65 constants
#define SYS_open  0xfff4
#define SYS_close 0xfff5
#define SYS_read  0xfff6
#define SYS_write 0xfff7
#define SYS_args  0xfff8
#define SYS_exit  0xfff9

typedef void (*sim65_write_t)(int count, const char* buf, int fd) __reentrant;

// sim65 expects a software stack
unsigned short stack[8];

// stack pointer location defined in sim65 header
unsigned short* __at(0x0) stackptr;

void
_putchar(unsigned char c)
{
  stackptr = stack;
  stackptr[0] = (short)&c;
  stackptr[1] = 1;
  (*(sim65_write_t)SYS_write)(1, &c, 1);
}

void
_initEmu(void)
{
}

void
_exitEmu(void)
{
  __asm
    jsr SYS_exit
  __endasm;
}
