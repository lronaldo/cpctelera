/* bug-2773.c

   Missing diagnostic on duplicate paramter name
 */

#ifdef TEST1
extern void *h2 (int g2, int g2); /* ERROR */
#endif

#ifdef TEST2
int i;

extern void *h (int g, int g) { /* ERROR */
	i = g;
}
#endif

