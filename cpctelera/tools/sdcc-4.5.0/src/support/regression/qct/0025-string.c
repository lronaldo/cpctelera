
int strlen(const char *);

int
main(void)
{
	const char *p;

	p = "hello";
	return strlen(p) - 5;
}
