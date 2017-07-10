#ifdef TESTchar
char a;
#endif

#ifdef TESTint
int a;
#endif

#ifdef TESTlong
long a;
#endif

#ifdef TESTshort
short a;
#endif

#ifdef TESTintlong
long int a;
#endif

#ifdef TESTintshort
short int a;
#endif

#ifdef TESTsigned
signed a;
#endif

#ifdef TESTunsigned
unsigned a;
#endif

#ifdef TESTintsigned
signed int a;
#endif

#ifdef TESTintunsigned
unsigned int a;
#endif

#ifdef TESTfloat
float a;
#endif

#ifdef TESTfloatsigned
signed float a;		/* ERROR */
#endif

#ifdef TESTfloatunsigned
unsigned float a;	/* ERROR */
#endif

#ifdef TESTfloatshort
short float a;		/* ERROR */
#endif

#ifdef TESTfloatlong
long float a;		/* ERROR */
#endif

#ifdef TESTdouble
double a;		/* WARNING(SDCC) */
#endif

#ifdef TESTdoubleshort
short double a;		/* ERROR */
#endif

#ifdef TESTdoublelong
long double a;		/* WARNING(SDCC) */
#endif

#ifdef TESTdoublesigned
signed double a;		/* ERROR */
#endif

#ifdef TESTdoubleunsigned
unsigned double a;		/* ERROR */
#endif

#ifdef TESTbit
__bit a;			/* ERROR(__z80||__gbz80||__hc08||PORT_HOST) */
#endif

#ifdef TESTsu1
signed unsigned int a;	/* ERROR */
#endif

#ifdef TESTsu2
unsigned signed int a;	/* ERROR */
#endif

#ifdef TESTsu3
unsigned signed a;	/* ERROR */
#endif

#ifdef TESTsu4
signed unsigned a;	/* ERROR */
#endif

