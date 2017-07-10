
#ifdef TEST1
enum tag
{
  first,
  second,
  third
};
#endif

#ifdef TEST2
enum tag
{
  first,	/* IGNORE */
  second,
  third,
  first,	/* ERROR */
  fourth
};
#endif


#ifdef TEST3
enum
{
  first,	/* IGNORE */
  second,
  third,
  first,	/* ERROR */
  fourth
};
#endif


#ifdef TEST4
enum
{
  first=1,
  second,
  third,
};
#endif


#ifdef TEST5
enum
{
  first=1.1,	/* ERROR */
  second,
  third,
};
#endif

#ifdef TEST6
int second;	/* IGNORE */

enum tag
{
  first,
  second,	/* ERROR */
  third
};
#endif

#ifdef TEST7
enum tag	/* IGNORE */
{
  first,
  second,
  third
};

enum tag {	/* ERROR */
  fourth,
  fifth,
  sixth
};
#endif

#ifdef TEST8
enum tag x;

enum tag
{
  first,
  second,
  third
};
#endif
