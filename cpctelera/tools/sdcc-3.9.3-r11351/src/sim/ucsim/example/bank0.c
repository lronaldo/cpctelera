#pragma codeseg BANK0

int
bank0_fn(int x) __banked
{
  return x+2;
}
