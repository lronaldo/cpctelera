#pragma codeseg BANK1

int
bank1_fn(int x) __banked
{
  return x+3;
}
