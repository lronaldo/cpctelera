extern void abort (void);

int main()
{
  int hicount = 0;
  const unsigned char *c;
  const char *str = "\x7f\xff";
  for (c = (const unsigned char *)str; *c ; c++) {
    if (!(((unsigned int)(*c)) < 0x80)) hicount++;
  }
  if (hicount != 1)
    abort ();
  return 0;
}

