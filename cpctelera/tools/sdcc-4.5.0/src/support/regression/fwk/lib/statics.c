/* needed by tests/bug1477149.c */

static long long_1 = 1;

static long s_get_long_1(void)
{
	long alfa = long_1;
	long beta = long_1 + alfa;
	long gamma = long_1 + beta;
	return alfa + beta + gamma;
}

long get_long_1(void)
{
	return s_get_long_1();
}

static float float_1 = 1;

static float s_get_float_1(void)
{
	float alfa = float_1;
	float beta = float_1 + alfa;
	float gamma = float_1 + beta;
	return alfa + beta + gamma;
}

float get_float_1(void)
{
	return s_get_float_1();
}

/* for bug 3038028 */
static char s_get_indexed(char index, char *msg)
{
	return msg[index];
}

char get_indexed(char index, char *msg)
{
	return s_get_indexed(index, msg);
}
