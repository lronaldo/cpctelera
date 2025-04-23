/* bug-3464.c

   Missing diagnostic on multiple storgae class specifiers if one of them was auto on a declaration at block scope.
 */

#ifdef TEST1
void g(void)
{
	static auto int i; /* ERROR */
	auto static int j; /* ERROR */
}
#endif

