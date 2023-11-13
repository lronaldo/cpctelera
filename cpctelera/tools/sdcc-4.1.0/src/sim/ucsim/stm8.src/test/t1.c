volatile int x= 1;

void isr_trap(void) __trap
{
  x= 2;
}

void trap()
{
  __asm
    trap;
  __endasm;
}

void
main(void)
{
  trap();
  for (;;)
    ;
}
