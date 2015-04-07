
#ifdef TEST1
typedef union {
  long l;
  float f;
} floatlong;

char func(char floatlong)
{
  return floatlong;
}
#endif

#ifdef TEST2
typedef union {
  long l;
  float f;
} floatlong;

long func(float x)
{
  typedef union {
    float f2;
    long l2;
    char c[4];
  } floatlong;
  floatlong fl;
  
  fl.f2=x;
  return fl.l2;
}
#endif

#ifdef TEST3
typedef int I;		/* IGNORE */
typedef int I;		/* ERROR */
#endif

#ifdef TEST4
typedef int I;		/* IGNORE */
typedef char I;		/* ERROR */
#endif

