#pragma disable_warning 84

int foo (int b, int c)
{
  int x;
  if (b)
    return x & c;
  else
    return 1;
}
extern void abort (void);
int main()
{
  if (foo(1, 0) != 0)
    abort ();
  return 0;
}

