/* bug-1952.c

   __func__
 */

#ifdef TEST1
void f(void)
{
	const char *p = __func__;
	__func__[0] = 3; /* ERROR */
}
#endif

