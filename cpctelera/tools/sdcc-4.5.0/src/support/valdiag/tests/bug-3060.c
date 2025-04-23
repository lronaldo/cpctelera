/* bug-3389.c

   Missing error message and segfault on attempt to initalize a global variable using an expression that contains a pointer dereference.
 */

#ifdef TEST1

int i;
int j = i; /* ERROR */

struct s
{
	char c[2];
    int a;
};

struct s x;
char *c = x.c; // SDCC users expect this to work.
int y = ((struct s*)&x)->a; /* ERROR */

#endif

