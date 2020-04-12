struct ts { int a; };

typedef struct ts *pts;

pts s;

int test(void)
{
  pts ps;
  ps= s;
  //return ps->a; // GOOD
  return (ps= s)->a; // FAIL
}

void main(void)
{
  test();
}
