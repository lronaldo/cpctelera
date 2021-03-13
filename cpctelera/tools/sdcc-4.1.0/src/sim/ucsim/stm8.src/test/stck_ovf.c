int f(int i)
{
  return f(i+1);
}

void main(void)
{
  f(1);
}
