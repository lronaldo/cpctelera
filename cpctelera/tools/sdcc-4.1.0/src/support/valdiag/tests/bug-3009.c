/* bug-3009.c

   Segfault on expression that is not an integer constant.
 */

#ifdef TEST1
int f();

enum some_enum
{
val = f() /* ERROR */
};
#endif

