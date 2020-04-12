/* bug-3184920.c
 */
#include <testfwk.h>

#include <limits.h>

typedef union registerpair {
	unsigned int	rp_pair;
	struct {
		unsigned char	rp_low;
		unsigned char	rp_high;
	} rp_bytes;
} registerpair_t;

typedef struct regs {
	registerpair_t	regpair_iy;
	registerpair_t	regpair_ix;
	registerpair_t	regpair_bc;
	registerpair_t	regpair_hl;
	registerpair_t	regpair_de;
	registerpair_t	regpair_af;
	registerpair_t	regpair_pc;
} regs_t;

/* register pairs */
#define regs_de regpair_de.rp_pair

/* Individual registers */
#define reg_d  regpair_de.rp_bytes.rp_high
#define reg_e  regpair_de.rp_bytes.rp_low

void
do_nothing(regs_t *r)
{
	r->reg_d = 0xaa;
	r->reg_e = 0x55;
}

void
read_values(unsigned char *a, unsigned char *b)
{
	regs_t	r;

	r.regs_de = 0xaa55;

	do_nothing(&r);

	*a = r.reg_d;
	*b = r.reg_e;
}

void testBug(void)
{
	unsigned char p1;
	unsigned char p2;

	read_values(&p1, &p2);

	ASSERT(p1 == 0xaa);
	ASSERT(p2 == 0x55);
}

