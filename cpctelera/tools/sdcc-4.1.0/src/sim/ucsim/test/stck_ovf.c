int x;

int f(int i)
{
  return f(i+1);
}

void main(void)
{
  x= 1;
  f(x);
}
