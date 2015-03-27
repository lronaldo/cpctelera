/*
   20011008-1.c from the execute part of the gcc torture tests.
 */

#include <testfwk.h>

#ifdef __SDCC
#pragma std_c99
#pragma disable_warning 85
#endif

// Hmm, this test uses identifiers tarting in __, which are reserved for the implementation. We probably should rename them. Philipp

// TODO: Enable when sdcc supports struct assignment.
#if 0
#include <stdint.h>

typedef uint32_t u_int32_t;
typedef uint8_t u_int8_t;

typedef enum {
        TXNLIST_DELETE,
        TXNLIST_LSN,
        TXNLIST_TXNID,
        TXNLIST_PGNO
} db_txnlist_type;

struct __db_lsn; typedef struct __db_lsn DB_LSN;
struct __db_lsn {
        u_int32_t file;
        u_int32_t offset;
};
struct __db_txnlist; typedef struct __db_txnlist DB_TXNLIST;

struct __db_txnlist {
        db_txnlist_type type;
        struct { struct __db_txnlist *le_next; struct __db_txnlist **le_prev; } links;
        union {
                struct {
                        u_int32_t txnid;
                        int32_t generation;
                        int32_t aborted;
                } t;
                struct {


                        u_int32_t flags;
                        int32_t fileid;
                        u_int32_t count;
                        char *fname;
                } d;
                struct {
                        int32_t ntxns;
                        int32_t maxn;
                        DB_LSN *lsn_array;
                } l;
                struct {
                        int32_t nentries;
                        int32_t maxentry;
                        char *fname;
                        int32_t fileid;
                        void *pgno_array;
                        u_int8_t uid[20];
                } p;
        } u;
};

int log_compare (const DB_LSN *a, const DB_LSN *b)
{
  return 1;
}

int
__db_txnlist_lsnadd(int val, DB_TXNLIST *elp, DB_LSN *lsnp, u_int32_t flags)
{
   int i;
 
   for (i = 0; i < (!(flags & (0x1)) ? 1 : elp->u.l.ntxns); i++)
     {
	int __j;
	DB_LSN __tmp;
	val++; 
	for (__j = 0; __j < elp->u.l.ntxns - 1; __j++)
	  if (log_compare(&elp->u.l.lsn_array[__j], &elp->u.l.lsn_array[__j + 1]) < 0)
	  {
	     __tmp = elp->u.l.lsn_array[__j];
	     elp->u.l.lsn_array[__j] = elp->u.l.lsn_array[__j + 1];
	     elp->u.l.lsn_array[__j + 1] = __tmp;
	  }
     }

   *lsnp = elp->u.l.lsn_array[0];
   return val;
}

#define	VLEN	1235
#endif

void
testTortureExecute (void)
{
#if 0
  DB_TXNLIST el;
  DB_LSN lsn, lsn_a[VLEN];
  
  el.u.l.ntxns = VLEN-1;
  el.u.l.lsn_array = lsn_a;
  
  if (__db_txnlist_lsnadd (0, &el, &lsn, 0) != 1)
    ASSERT (0);
  
  if (__db_txnlist_lsnadd (0, &el, &lsn, 1) != VLEN-1)
    ASSERT (0);
  
  return;
#endif
}

