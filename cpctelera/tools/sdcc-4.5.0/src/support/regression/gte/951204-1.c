void f (char *x)
{
  *x = 'x';
}

void main (void)
{
  int i;
  char x = '\0';

  for (i = 0; i < 100; ++i)
    {
      f (&x);
      if (*(const char *) &x != 'x')
	abort ();
    }
  exit (0);
}
