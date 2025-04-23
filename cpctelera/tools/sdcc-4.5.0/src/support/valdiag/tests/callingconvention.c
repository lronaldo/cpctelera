
#if defined(__SDCC) && defined(__has_sdcccall)
#define SDCCCALL0  __sdcccall(0)
#define SDCCCALL1  __sdcccall(1)
#else
#define SDCCCALL0
#define SDCCCALL1
#endif

#if defined(__SDCC) && defined(__has_z88dk_fastcall)
#define Z88DK_FASTCALL  __z88dk_fastcall
#else
#define Z88DK_FASTCALL
#endif

#if defined(__SDCC) && defined(__has_raisonance)
#define RAISONANCE  __raisonance
#else
#define RAISONANCE
#endif

#ifdef TEST1
int f(int i) SDCCCALL0;        /* IGNORE */
int f(int i) SDCCCALL1 {       /* ERROR(__has_sdcccall) */
  return i;
}
#endif

#ifdef TEST2
int f(int i) SDCCCALL1;        /* IGNORE */
int f(int i) SDCCCALL0 {       /* ERROR(__has_sdcccall) */

  return i;
}
#endif

#ifdef TEST3
int f(int i) Z88DK_FASTCALL;   /* IGNORE */
int f(int i) {                 /* ERROR(__has_z88dk_fastcall) */
  return i;
}
#endif

#ifdef TEST4
int f(int i);                  /* IGNORE */
int f(int i) Z88DK_FASTCALL {  /* ERROR(__has_z88dk_fastcall) */

  return i;
}
#endif

#ifdef TEST5
int f(int i) RAISONANCE;       /* IGNORE */
int f(int i) {                 /* ERROR(__has_raisonance) */
  return i;
}
#endif

#ifdef TEST6
int f(int i);                  /* IGNORE */
int f(int i) RAISONANCE {      /* ERROR(__has_raisonance) */

  return i;
}
#endif

