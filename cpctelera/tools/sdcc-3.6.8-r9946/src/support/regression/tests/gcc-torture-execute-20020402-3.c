/*
   20020402-3.c from the execute part of the gcc torture suite.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#endif

#define NULL ((void *) 0)

// Todo: Enable when long long comes to these ports!
#if !defined(__SDCC_ds390) && !defined(__SDCC_ds400) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_hc08) && !defined(__SDCC_s08)

/* extracted from gdb sources */

typedef unsigned long long CORE_ADDR;

struct blockvector;

struct symtab {
  struct blockvector *blockvector;
};

struct sec {
  void *unused;
};

struct symbol {
  int len;
  char *name;
};

struct block {
	CORE_ADDR startaddr, endaddr;
	struct symbol *function;
	struct block *superblock;
	unsigned char gcc_compile_flag;
	int nsyms;
	struct symbol syms[1];
};

struct blockvector {
	int nblocks;
	struct block *block[2];
};

struct blockvector *blockvector_for_pc_sect(register CORE_ADDR pc,
					    struct symtab *symtab)
{
  register struct block *b;
  register int bot, top, half;
  struct blockvector *bl;

  bl = symtab->blockvector;
  b = bl->block[0];

  bot = 0;
  top = bl->nblocks;

  while (top - bot > 1)
    {
      half = (top - bot + 1) >> 1;
      b = bl->block[bot + half];
      if (b->startaddr <= pc)
	bot += half;
      else
	top = bot + half;
    }

  while (bot >= 0)
    {
      b = bl->block[bot];
      if (b->endaddr > pc)
	{
	  return bl;
	}
      bot--;
    }
  return 0;
}
#endif

void testTortureExecute(void)
{
#if !(defined (__SDCC_mcs51) && defined (__SDCC_MODEL_SMALL)) // Not enough memory
#if !defined(__SDCC_ds390) && !defined(__SDCC_ds400) && !defined(__SDCC_pic14) && !defined(__SDCC_pic16) && !defined(__SDCC_hc08) && !defined(__SDCC_s08) // No long long
  struct block a = { 0, 0x10000, 0, 0, 1, 20 };
  struct block b = { 0x10000, 0x20000, 0, 0, 1, 20 };
  struct blockvector bv = { 2, { &a, &b } };
  struct symtab s = { &bv };

  struct blockvector *ret;

  ret = blockvector_for_pc_sect(0x500, &s);

  ASSERT (ret != NULL && ret->nblocks == 2);
  ASSERT (ret->block[0] != NULL && ret->block[0]->startaddr == 0LL && ret->block[0]->endaddr == 65536LL);
  ASSERT (ret->block[1] != NULL && ret->block[1]->startaddr == 65536LL && ret->block[1]->endaddr == 131072LL);

  return;
#endif
#endif
}
