
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

/* This is no longer an error in C11, so test with earlier standard. */
/* Unfortunately, GCC seems to accept this regardless. */
#ifdef TEST3_C99
typedef int I;		/* IGNORE */
typedef int I;		/* ERROR(SDCC) */
#endif

#ifdef TEST4
typedef int I;		/* IGNORE */
typedef char I;		/* ERROR */
#endif

