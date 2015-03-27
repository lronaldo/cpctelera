/*
   bug3381400.c
*/

#include <testfwk.h>

#pragma disable_warning  85 //no warning about unreferenced variables (W_NO_REFERENCE)

#define UCHAR	unsigned char
#define USHORT	unsigned short
#define PCHAR	unsigned char *

#ifdef __SDCC_z80
__sfr __at 0x05 rSRAM_Page;
#endif

#define FILE_FLAG_PAGE_SIZE		0xffd0

#define BUF_PAGE_NUM	4

#define DEFAULT_MEMORY_PAGE		0x07
void ReadFlashData(USHORT sDstAddr, USHORT sSrcAddr, USHORT sLen, UCHAR iPage);

UCHAR Sys_iSystemPage;

#define DSP_INFO_LEN	0x40
#define DM_START_POS	0x0e

#define PAGE_EDGE	(FILE_FLAG_PAGE_SIZE >> 1)

typedef struct _dsp_page
{
	UCHAR iPage;
	USHORT sPageOffset;
} DSP_PAGE;

void ReadFlashData(USHORT sDstAddr, USHORT sSrcAddr, USHORT sLen, UCHAR iPage)
{
	ASSERT(iPage == 42);
}

void _DspLoadFile(DSP_PAGE * pDspPage, USHORT sDMOffset)
{
	UCHAR i, iPage;
	USHORT sDMAddr;
	UCHAR pInfo[DSP_INFO_LEN];

	iPage = pDspPage->iPage + Sys_iSystemPage;
	ReadFlashData((USHORT)pInfo, (pDspPage->sPageOffset << 1), DSP_INFO_LEN, iPage);	// get dsp info

	for (i = 0; i < BUF_PAGE_NUM+2; i ++)
	{
	}
#ifdef __SDCC_z80
	rSRAM_Page = DEFAULT_MEMORY_PAGE;
#endif
}

void testBug(void)
{
	DSP_PAGE p;
	p.iPage = 23;
	Sys_iSystemPage = 42 - 23;
	_DspLoadFile(&p, 0);
}

