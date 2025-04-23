/* bug-2793.c
   A parameter changed from unsigned int to unsigned long in CSE.
 */

#include <testfwk.h>

typedef unsigned int	UINT;
typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;

BYTE global_buf[10];

static void mem_set (void* dst, int val, UINT cnt)
{
	ASSERT (dst == global_buf);
	ASSERT (val == 0);
	ASSERT (cnt == 1);
}

void f_mkfs (
	void* work,
	UINT len
)
{
	const UINT n_fats = 1;

	BYTE *buf;
	WORD ss = 1;
	DWORD szb_buf, sz_buf, sect, nsect, n;
	DWORD b_fat;
	DWORD sz_fat;
	UINT i;

	b_fat = 1;
	sz_fat = 0;

	{
		buf = (BYTE*)work;
		sz_buf = len / ss;
		szb_buf = sz_buf * ss;
	}

	{
		mem_set(buf, 0, (UINT)szb_buf);
		sect = b_fat;
		for (i = 0; i < n_fats; i++) {
			nsect = sz_fat;
			do {
				n = (nsect > sz_buf) ? sz_buf : nsect;	
				mem_set(buf, 0, ss);
				sect += n; nsect -= n;
			} while (nsect);
		}
	}
}

void testBug(void)
{
	f_mkfs (global_buf, 1);
}

