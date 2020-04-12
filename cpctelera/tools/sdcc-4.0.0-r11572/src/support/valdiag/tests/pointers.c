
#ifdef TEST0

typedef void (*fptr1_t)(void);
typedef void (*fptr2_t)(int);

fptr1_t fptr1;
fptr2_t fptr2;

void *vp = 0;

void testPtr(void)
{
	fptr1 = vp;    /* WARNING */
	fptr2 = fptr1; /* IGNORE(GCC) */
	vp = fptr1;    /* WARNING */
	fptr1 = (void *)0;
}

#endif

