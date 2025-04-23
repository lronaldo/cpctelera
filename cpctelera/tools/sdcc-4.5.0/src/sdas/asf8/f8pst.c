/* stm8pst.c */

/*
 *  Copyright (C) 1010  Alan R. Baldwin
 *  Copyright (C) 2022  Philipp K. Krause
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Alan R. Baldwin
 * 721 Berkeley St.
 * Kent, Ohio  44240
 * 
 */

#include "asxxxx.h"
#include "f8.h"

#if 0
/*
 * Coding Banks
 */
struct	bank	bank[2] = {
    /*	The '_CODE' area/bank has a NULL default file suffix.	*/
    {	NULL,		"_CSEG",	NULL,		0,	0,	0,	0,	0	},
    {	&bank[0],	"_DSEG",	"_DS",		1,	0,	0,	0,	B_FSFX	}
};

/*
 * Coding Areas
 */
struct	area	area[2] = {
    {	NULL,		&bank[0],	"_CODE",	0,	0,	0,	A_1BYTE|A_BNK|A_CSEG	},
    {	&area[0],	&bank[1],	"_DATA",	1,	0,	0,	A_1BYTE|A_BNK|A_DSEG	}
};
#endif

/*
 * Basic Relocation Mode Definition
 *
 *	#define		R_NORM	0000		No Bit Positioning
 */
char	mode0[32] = {	/* R_NORM */
	'\200',	'\201',	'\202',	'\203',	'\204',	'\205',	'\206',	'\207',
	'\210',	'\211',	'\212',	'\213',	'\214',	'\215',	'\216',	'\217',
	'\220',	'\221',	'\222',	'\223',	'\224',	'\225',	'\226',	'\227',
	'\230',	'\231',	'\232',	'\233',	'\234',	'\235',	'\236',	'\237'
};

/*
 * Additional Relocation Mode Definitions
 */

/*
 * Bit Relocation Mode Definition
 *
 *	#define		R_BITS	0100		Bit Test Positioning
 */
char	mode1[32] = {	/* R_BITS */
	'\201',	'\202',	'\203',	'\003',	'\004',	'\005',	'\006',	'\007',
	'\010',	'\011',	'\012',	'\013',	'\014',	'\015',	'\016',	'\017',
	'\020',	'\021',	'\022',	'\023',	'\024',	'\025',	'\026',	'\027',
	'\030',	'\031',	'\032',	'\033',	'\034',	'\035',	'\036',	'\037'
};


/*
 *     *m_def is a pointer to the bit relocation definition.
 *	m_flag indicates that bit position swapping is required.
 *	m_dbits contains the active bit positions for the output.
 *	m_sbits contains the active bit positions for the input.
 *
 *	struct	mode
 *	{
 *		char *	m_def;		Bit Relocation Definition
 *		a_uint	m_flag;		Bit Swapping Flag
 *		a_uint	m_dbits;	Destination Bit Mask
 *		a_uint	m_sbits;	Source Bit Mask
 *	};
 */
struct	mode	mode[2] = {
    {	&mode0[0],	0,	0x0000FFFF,	0x0000FFFF	},
    {	&mode1[0],	1,	0x0000000E,	0x00000007	}
};


/*
 * Array of Pointers to mode Structures
 */
struct	mode	*modep[16] = {
	&mode[0],	&mode[1],	NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL
};

/*
 * Mnemonic Structure
 */
struct	mne	mne[] = {

	/* machine */

  //{	NULL,	"CSEG",		S_ATYP,		0,	A_CSEG|A_1BYTE	},
  //{	NULL,	"DSEG",		S_ATYP,		0,	A_DSEG|A_1BYTE	},

	/* system */

  //{	NULL,	"BANK",		S_ATYP,		0,	A_BNK	},
    {	NULL,	"CON",		S_ATYP,		0,	A_CON	},
    {	NULL,	"OVR",		S_ATYP,		0,	A_OVR	},
    {	NULL,	"REL",		S_ATYP,		0,	A_REL	},
    {	NULL,	"ABS",		S_ATYP,		0,	A_ABS	},
    {	NULL,	"NOPAG",	S_ATYP,		0,	A_NOPAG	},
    {	NULL,	"PAG",		S_ATYP,		0,	A_PAG	},

#if 0 /* ljm -- figure out what S_BTYP (bank type?) is */
    {	NULL,	"BASE",		S_BTYP,		0,	B_BASE	},
    {	NULL,	"SIZE",		S_BTYP,		0,	B_SIZE	},
    {	NULL,	"FSFX",		S_BTYP,		0,	B_FSFX	},
    {	NULL,	"MAP",		S_BTYP,		0,	B_MAP	},
#endif
    {	NULL,	"NOLOAD",	S_ATYP,		0,	A_NOLOAD},
    {	NULL,	".page",	S_PAGE,		0,	0	},
    {	NULL,	".title",	S_HEADER,	0,	O_TITLE	},
    {	NULL,	".sbttl",	S_HEADER,	0,	O_SBTTL	},
    {	NULL,	".module",	S_MODUL,	0,	0	},
    {	NULL,	".include",	S_INCL,		0,	I_CODE	},
    {	NULL,	".incbin",	S_INCL,		0,	I_BNRY	},
    {	NULL,	".area",	S_AREA,		0,	0	},
    //{	NULL,	".bank",	S_BANK,		0,	0	},
    {	NULL,	".org",		S_ORG,		0,	0	},
    {	NULL,	".radix",	S_RADIX,	0,	0	},
    {	NULL,	".globl",	S_GLOBL,	0,	0	},
    {	NULL,	".local",	S_LOCAL,	0,	0	},
    {	NULL,	".if",		S_CONDITIONAL,	0,	O_IF	},
    {	NULL,	".iff",		S_CONDITIONAL,	0,	O_IFF	},
    {	NULL,	".ift",		S_CONDITIONAL,	0,	O_IFT	},
    {	NULL,	".iftf",	S_CONDITIONAL,	0,	O_IFTF	},
    {	NULL,	".ifdef",	S_CONDITIONAL,	0,	O_IFDEF	},
    {	NULL,	".ifndef",	S_CONDITIONAL,	0,	O_IFNDEF},
    {	NULL,	".ifgt",	S_CONDITIONAL,	0,	O_IFGT	},
    {	NULL,	".iflt",	S_CONDITIONAL,	0,	O_IFLT	},
    {	NULL,	".ifge",	S_CONDITIONAL,	0,	O_IFGE	},
    {	NULL,	".ifle",	S_CONDITIONAL,	0,	O_IFLE	},
    {	NULL,	".ifeq",	S_CONDITIONAL,	0,	O_IFEQ	},
    {	NULL,	".ifne",	S_CONDITIONAL,	0,	O_IFNE	},
    {	NULL,	".ifb",		S_CONDITIONAL,	0,	O_IFB	},
    {	NULL,	".ifnb",	S_CONDITIONAL,	0,	O_IFNB	},
    {	NULL,	".ifidn",	S_CONDITIONAL,	0,	O_IFIDN	},
    {	NULL,	".ifdif",	S_CONDITIONAL,	0,	O_IFDIF	},
    {	NULL,	".iif",		S_CONDITIONAL,	0,	O_IIF	},
    {	NULL,	".iiff",	S_CONDITIONAL,	0,	O_IIFF	},
    {	NULL,	".iift",	S_CONDITIONAL,	0,	O_IIFT	},
    {	NULL,	".iiftf",	S_CONDITIONAL,	0,	O_IIFTF	},
    {	NULL,	".iifdef",	S_CONDITIONAL,	0,	O_IIFDEF},
    {	NULL,	".iifndef",	S_CONDITIONAL,	0,	O_IIFNDEF},
    {	NULL,	".iifgt",	S_CONDITIONAL,	0,	O_IIFGT	},
    {	NULL,	".iiflt",	S_CONDITIONAL,	0,	O_IIFLT	},
    {	NULL,	".iifge",	S_CONDITIONAL,	0,	O_IIFGE	},
    {	NULL,	".iifle",	S_CONDITIONAL,	0,	O_IIFLE	},
    {	NULL,	".iifeq",	S_CONDITIONAL,	0,	O_IIFEQ	},
    {	NULL,	".iifne",	S_CONDITIONAL,	0,	O_IIFNE	},
    {	NULL,	".iifb",	S_CONDITIONAL,	0,	O_IIFB	},
    {	NULL,	".iifnb",	S_CONDITIONAL,	0,	O_IIFNB	},
    {	NULL,	".iifidn",	S_CONDITIONAL,	0,	O_IIFIDN},
    {	NULL,	".iifdif",	S_CONDITIONAL,	0,	O_IIFDIF},
    {	NULL,	".else",	S_CONDITIONAL,	0,	O_ELSE	},
    {	NULL,	".endif",	S_CONDITIONAL,	0,	O_ENDIF	},
    {	NULL,	".list",	S_LISTING,	0,	O_LIST	},
    {	NULL,	".nlist",	S_LISTING,	0,	O_NLIST	},
    {	NULL,	".uleb128",	S_ULEB128,	0,	0	},
    {	NULL,	".sleb128",	S_SLEB128,	0,	0	},
    {	NULL,	".equ",		S_EQU,		0,	O_EQU	},
    {	NULL,	".gblequ",	S_EQU,		0,	O_GBLEQU},
    {	NULL,	".lclequ",	S_EQU,		0,	O_LCLEQU},
    {	NULL,	".byte",	S_DATA,		0,	O_1BYTE	},
    {	NULL,	".db",		S_DATA,		0,	O_1BYTE	},
    {	NULL,	".fcb",		S_DATA,		0,	O_1BYTE	},
    {	NULL,	".word",	S_DATA,		0,	O_2BYTE	},
    {	NULL,	".dw",		S_DATA,		0,	O_2BYTE	},
    {	NULL,	".fdb",		S_DATA,		0,	O_2BYTE	},
    {	NULL,	".3byte",	S_DATA,		0,	O_3BYTE	},
    {	NULL,	".triple",	S_DATA,		0,	O_3BYTE	},
/*    {	NULL,	".4byte",	S_DATA,		0,	O_4BYTE	},	*/
/*    {	NULL,	".quad",	S_DATA,		0,	O_4BYTE	},	*/
    {	NULL,	".blkb",	S_BLK,		0,	O_1BYTE	},
    {	NULL,	".ds",		S_BLK,		0,	O_1BYTE	},
    {	NULL,	".rmb",		S_BLK,		0,	O_1BYTE	},
    {	NULL,	".rs",		S_BLK,		0,	O_1BYTE	},
    {	NULL,	".blkw",	S_BLK,		0,	O_2BYTE	},
    {	NULL,	".blk3",	S_BLK,		0,	O_3BYTE	},
/*    {	NULL,	".blk4",	S_BLK,		0,	O_4BYTE	},	*/
    {	NULL,	".ascii",	S_ASCIX,	0,	O_ASCII	},
    {	NULL,	".ascis",	S_ASCIX,	0,	O_ASCIS	},
    {	NULL,	".asciz",	S_ASCIX,	0,	O_ASCIZ	},
    {	NULL,	".str",		S_ASCIX,	0,	O_ASCII	},
    {	NULL,	".strs",	S_ASCIX,	0,	O_ASCIS	},
    {	NULL,	".strz",	S_ASCIX,	0,	O_ASCIZ	},
    {	NULL,	".fcc",		S_ASCIX,	0,	O_ASCII	},
    {	NULL,	".define",	S_DEFINE,	0,	O_DEF	},
    {	NULL,	".undefine",	S_DEFINE,	0,	O_UNDEF	},
    {	NULL,	".even",	S_BOUNDARY,	0,	O_EVEN	},
    {	NULL,	".odd",		S_BOUNDARY,	0,	O_ODD	},
    {	NULL,	".bndry",	S_BOUNDARY,	0,	O_BNDRY	},
    {	NULL,	".msg"	,	S_MSG,		0,	0	},
    {	NULL,	".assume",	S_ERROR,	0,	O_ASSUME},
    {	NULL,	".error",	S_ERROR,	0,	O_ERROR	},
    //{	NULL,	".msb",		S_MSB,		0,	0	},
/*    {	NULL,	".lohi",	S_MSB,		0,	O_LOHI	},	*/
/*    {	NULL,	".hilo",	S_MSB,		0,	O_HILO	},	*/
/*    {	NULL,	".8bit",	S_BITS,		0,	O_1BYTE	},	*/
/*    {	NULL,	".16bit",	S_BITS,		0,	O_2BYTE	},	*/
/*    {	NULL,	".24bit",	S_BITS,		0,	O_3BYTE	},	*/
/*    {	NULL,	".32bit",	S_BITS,		0,	O_4BYTE	},	*/
    {	NULL,	".end",		S_END,		0,	0	},
/* sdas specific */
    {   NULL,   ".optsdcc",     S_OPTSDCC,      0,      0       },
/* end sdas specific */

	/* Macro Processor */

    {	NULL,	".macro",	S_MACRO,	0,	O_MACRO	},
    {	NULL,	".endm",	S_MACRO,	0,	O_ENDM	},
    {	NULL,	".mexit",	S_MACRO,	0,	O_MEXIT	},

    {	NULL,	".narg",	S_MACRO,	0,	O_NARG	},
    {	NULL,	".nchr",	S_MACRO,	0,	O_NCHR	},
    {	NULL,	".ntyp",	S_MACRO,	0,	O_NTYP	},

    {	NULL,	".irp",		S_MACRO,	0,	O_IRP	},
    {	NULL,	".irpc",	S_MACRO,	0,	O_IRPC	},
    {	NULL,	".rept",	S_MACRO,	0,	O_REPT	},

    {	NULL,	".nval",	S_MACRO,	0,	O_NVAL	},

    {	NULL,	".mdelete",	S_MACRO,	0,	O_MDEL	},

	/* f8 instructions */

	/* 8-bit 2-op-instrustions */
    {	NULL,	"sub",		S_2OPSUB,	0,	0x00	},
    {	NULL,	"sbc",		S_2OPSUB,	0,	0x08	},
    {	NULL,	"add",		S_2OP,		0,	0x10	},
    {	NULL,	"adc",		S_2OP,		0,	0x18	},
    {	NULL,	"cp",		S_2OP,		0,	0x20	},
    {	NULL,	"or",		S_2OP,		0,	0x28	},
    {	NULL,	"and",		S_2OP,		0,	0x30	},
    {	NULL,	"xor",		S_2OP,		0,	0x38	},

	/* 8-bit 1-op-instrustions */
    {	NULL,	"srl",		S_1OP,		0,	0x40	},
    {	NULL,	"sll",		S_1OP,		0,	0x44	},
    {	NULL,	"rrc",		S_1OP,		0,	0x48	},
    {	NULL,	"rlc",		S_1OP,		0,	0x4c	},
    {	NULL,	"inc",		S_1OP,		0,	0x50	},
    {	NULL,	"dec",		S_1OP,		0,	0x54	},
    {	NULL,	"clr",		S_1OP,		0,	0x58	},
    {	NULL,	"tst",		S_1OP,		0,	0x5c	},
    {	NULL,	"push",		S_1OPPUSH,	0,	0x60	},

	/* 16-bit 2-op-instrustions */
    {	NULL,	"subw",		S_2OPWSUB,	0,	0x70	},
    {	NULL,	"sbcw",		S_2OPWSBC,	0,	0x74	},
    {	NULL,	"addw",		S_2OPWADD,	0,	0x78	},
    {	NULL,	"adcw",		S_2OPWADC,	0,	0x7c	},
    {	NULL,	"orw",		S_2OPW,		0,	0xf0	},
    {	NULL,	"xorw",		S_2OPW,		0,	0xfc	},

	/* 16-bit 1-op-instrustions */
    {	NULL,	"clrw",		S_1OPW,		0,	0xa0	},
    {	NULL,	"incw",		S_1OPW,		0,	0xa4	},
    //{	NULL,	"adcw",		S_1OPW,		0,	0xa8	},
    //{	NULL,	"sbcw",		S_1OPW,		0,	0xac	},
    {	NULL,	"pushw",	S_1OPWPUSH,	0,	0xb0	},
    {	NULL,	"tstw",		S_1OPW,		0,	0xb4	},

    /* 8-bit loads */
    {	NULL,	"ld",		S_LD,		0,	0x80	},
    {	NULL,	"ldi",		S_LDI,		0,	0xed	},

    /* 16-bit loads */
    {	NULL,	"ldw",		S_LDW,		0,	0xc0	},
    {	NULL,	"ldwi",		S_LDI,		0,	0xcf	},
    
    /* 8-bit 0-op-instrustions */     
    {	NULL,	"bool",		S_0OP,		0,	0x98	},
    {	NULL,	"sra",		S_0OP,		0,	0x96	},
    {	NULL,	"daa",		S_0OP,		0,	0x97	},
    {	NULL,	"pop",		S_0OP,		0,	0x99	},
    {	NULL,	"xch",		S_0OPXCH,	0,	0x90	},
    {	NULL,	"msk",		S_0OPMSK,	0,	0xb8	},
    {	NULL,	"cax",		S_0OP,		0,	0x9b	},
    {	NULL,	"mad",		S_0OPMAD,	0,	0xbc	},
    {	NULL,	"rot",		S_0OPROT,	0,	0x95	},
    {	NULL,	"thrd",		S_0OP,		0,	0x9a	},
	{	NULL,	"cax",		S_0OPCAX,	0,	0x9b	},
    //{	NULL,	"push",		S_0OP,		0,	0x90	},
    
    /* 16-bit 0-op-instrustions */
    {	NULL,	"negw",		S_0OPW,		0,	0xfa	},
    {	NULL,	"boolw",	S_0OPW,		0,	0xfb	},
    {	NULL,	"srlw",		S_0OPW,		0,	0xe0	},
    {	NULL,	"sllw",		S_0OPWSLL,	0,	0xe1	},
    {	NULL,	"rrcw",		S_0OPWRLC,	0,	0xe2	},
    {	NULL,	"rlcw",		S_0OPWRLC,	0,	0xe3	},
    {	NULL,	"sraw",		S_0OPW,		0,	0xe4	},
    {	NULL,	"mul",		S_0OPW,		0,	0xb9	},
    {	NULL,	"popw",		S_0OPW,		0,	0xe9	},
    {	NULL,	"xchw",		S_0OPWXCH,	0,	0xf4	},
    {	NULL,	"caxw",		S_0OPW,		0,	0xf9	},
//    {	NULL,	"addw",		S_0OPW,		0,	0xea	},
    {	NULL,	"cpw",		S_0OPWCP,	0,	0xf8	},
    {	NULL,	"incnw",	S_0OPW,		0,	0xf6	},
    {	NULL,	"decw",		S_0OPWDEC,	0,	0xf3	},
//    {	NULL,	"pushw",	S_0OPW,		0,	0xe8	},
//    {	NULL,	"sllw",		S_0OPW,		0,	0xe5	},
//    {	NULL,	"rlcw",		S_0OPW,		0,	0xe6	},
//    {	NULL,	"rrcw",		S_0OPW,		0,	0xe7	},
	{	NULL,	"cltz",		S_0OPW,		0,	0xed	},
    {	NULL,	"sex",		S_0OPWSEX,	0,	0xee	},
    {	NULL,	"zex",		S_0OPWSEX,	0,	0xef	},
//    {	NULL,	"addw",		S_0OPW,		0,	0xec	},
	{	NULL,	"caxw",		S_0OPWCAX,	0,	0xf9	},

    /* bit instructions */
    {	NULL,	"xchb",		S_BIT,		0,	0x68	},

	/* relative jumps */
    {	NULL,	"jr",		S_JR,		0,	0xd0	},
    {	NULL,	"jrz",		S_JR,		0,	0xd2	},
    {	NULL,	"jrnz",		S_JR,		0,	0xd3	},
    {	NULL,	"jrc",		S_JR,		0,	0xd4	},
    {	NULL,	"jrnc",		S_JR,		0,	0xd5	},
    {	NULL,	"jrn",		S_JR,		0,	0xd6	},
    {	NULL,	"jrnn",		S_JR,		0,	0xd7	},
    {	NULL,	"jrno",		S_JR,		0,	0xd9	},
    {	NULL,	"jro",		S_JR2,		0,	0xd9	},
    {	NULL,	"jrsge",	S_JR,		0,	0xda	},
    {	NULL,	"jrslt",	S_JR,		0,	0xdb	},
    {	NULL,	"jrsle",	S_JR,		0,	0xdd	},
    {	NULL,	"jrsgt",	S_JR2,		0,	0xdd	},
    {	NULL,	"jrle",		S_JR,		0,	0xdf	},
    {	NULL,	"jrgt",		S_JR2,		0,	0xdf	},
    {	NULL,	"dnjnz",	S_DNJNZ,	0,	0xd1	},

    /* other instructions */
	{	NULL,	"call",		S_JP,		0,	0x66	},
	{	NULL,	"jp",		S_JP,		0,	0x64	},
	{	NULL,	"ret",		S_RET,		0,	0xba	},
	{	NULL,	"reti",		S_RET,		0,	0xbb	},

	/* nop */
    {	NULL,	"nop",		S_TRAP,		0,	0x08	},
    
    /* trap */
    {	NULL,	"trap",		S_TRAP,	S_EOL,	0x00	}

};
