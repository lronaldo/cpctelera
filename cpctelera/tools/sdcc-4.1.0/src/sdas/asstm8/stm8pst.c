/* stm8pst.c */

/*
 *  Copyright (C) 1010  Alan R. Baldwin
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
#include "stm8.h"

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
    {	NULL,	".include",	S_INCL,		0,	0	},
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
    //{	NULL,	".ifdef",	S_CONDITIONAL,	0,	O_IFDEF	},
    //{	NULL,	".ifndef",	S_CONDITIONAL,	0,	O_IFNDEF},
    {	NULL,	".ifgt",	S_CONDITIONAL,	0,	O_IFGT	},
    {	NULL,	".iflt",	S_CONDITIONAL,	0,	O_IFLT	},
    {	NULL,	".ifge",	S_CONDITIONAL,	0,	O_IFGE	},
    {	NULL,	".ifle",	S_CONDITIONAL,	0,	O_IFLE	},
    {	NULL,	".ifeq",	S_CONDITIONAL,	0,	O_IFEQ	},
    {	NULL,	".ifne",	S_CONDITIONAL,	0,	O_IFNE	},
    //{	NULL,	".ifb",		S_CONDITIONAL,	0,	O_IFB	},
    //{	NULL,	".ifnb",	S_CONDITIONAL,	0,	O_IFNB	},
    //{	NULL,	".ifidn",	S_CONDITIONAL,	0,	O_IFIDN	},
    //{	NULL,	".ifdif",	S_CONDITIONAL,	0,	O_IFDIF	},
    {	NULL,	".iif",		S_CONDITIONAL,	0,	O_IIF	},
    {	NULL,	".iiff",	S_CONDITIONAL,	0,	O_IIFF	},
    {	NULL,	".iift",	S_CONDITIONAL,	0,	O_IIFT	},
    {	NULL,	".iiftf",	S_CONDITIONAL,	0,	O_IIFTF	},
    //{	NULL,	".iifdef",	S_CONDITIONAL,	0,	O_IIFDEF},
    //{	NULL,	".iifndef",	S_CONDITIONAL,	0,	O_IIFNDEF},
    {	NULL,	".iifgt",	S_CONDITIONAL,	0,	O_IIFGT	},
    {	NULL,	".iiflt",	S_CONDITIONAL,	0,	O_IIFLT	},
    {	NULL,	".iifge",	S_CONDITIONAL,	0,	O_IIFGE	},
    {	NULL,	".iifle",	S_CONDITIONAL,	0,	O_IIFLE	},
    {	NULL,	".iifeq",	S_CONDITIONAL,	0,	O_IIFEQ	},
    {	NULL,	".iifne",	S_CONDITIONAL,	0,	O_IIFNE	},
    //{	NULL,	".iifb",	S_CONDITIONAL,	0,	O_IIFB	},
    //{	NULL,	".iifnb",	S_CONDITIONAL,	0,	O_IIFNB	},
    //{	NULL,	".iifidn",	S_CONDITIONAL,	0,	O_IIFIDN},
    //{	NULL,	".iifdif",	S_CONDITIONAL,	0,	O_IIFDIF},
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
    //{	NULL,	".define",	S_DEFINE,	0,	O_DEF	},
    //{	NULL,	".undefine",	S_DEFINE,	0,	O_UNDEF	},
    {	NULL,	".even",	S_BOUNDARY,	0,	O_EVEN	},
    {	NULL,	".odd",		S_BOUNDARY,	0,	O_ODD	},
    {	NULL,	".bndry",	S_BOUNDARY,	0,	O_BNDRY	},
    //{	NULL,	".msg"	,	S_MSG,		0,	0	},
    //{	NULL,	".assume",	S_ERROR,	0,	O_ASSUME},
    //{	NULL,	".error",	S_ERROR,	0,	O_ERROR	},
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

	/* stm8 Instructions */

    {	NULL,	"jra",		S_JR,		0,	0x20	},
    {	NULL,	"jreq",		S_JR,		0,	0x27	},
    {	NULL,	"jrf",		S_JR,		0,	0x21	},
    {	NULL,	"jrc",		S_JR,		0,	0x25	},
    {	NULL,	"jrmi",		S_JR,		0,	0x2B	},
    {	NULL,	"jrnc",		S_JR,		0,	0x24	},
    {	NULL,	"jrne",		S_JR,		0,	0x26	},
    {	NULL,	"jrnv",		S_JR,		0,	0x28	},
    {	NULL,	"jrpl",		S_JR,		0,	0x2A	},
    {	NULL,	"jrsge",	S_JR,		0,	0x2E	},
    {	NULL,	"jrsgt",	S_JR,		0,	0x2C	},
    {	NULL,	"jrsle",	S_JR,		0,	0x2D	},
    {	NULL,	"jrslt",	S_JR,		0,	0x2F	},
    {	NULL,	"jrt",		S_JR,		0,	0x20	},
    {	NULL,	"jruge",	S_JR,		0,	0x24	},
    {	NULL,	"jrugt",	S_JR,		0,	0x22	},
    {	NULL,	"jrule",	S_JR,		0,	0x23	},
    {	NULL,	"jrult",	S_JR,		0,	0x25	},
    {	NULL,	"jrv",		S_JR,		0,	0x29	},

    {	NULL,	"jrnh",		S_JRPG,		0,	0x28	},
    {	NULL,	"jrh",		S_JRPG,		0,	0x29	},
    {	NULL,	"jrnm",		S_JRPG,		0,	0x2C	},
    {	NULL,	"jrm",		S_JRPG,		0,	0x2D	},
    {	NULL,	"jril",		S_JRPG,		0,	0x2E	},
    {	NULL,	"jrih",		S_JRPG,		0,	0x2F	},

    {	NULL,	"btjt",		S_JRBT,		0,	0x00	},
    {	NULL,	"btjf",		S_JRBT,		0,	0x01	},

    {	NULL,	"bccm",		S_BT90,		0,	0x11	},
    {	NULL,	"bcpl",		S_BT90,		0,	0x10	},
    {	NULL,	"bres",		S_BT72,		0,	0x11	},
    {	NULL,	"bset",		S_BT72,		0,	0x10	},

    {	NULL,	"ld",		S_LD,		0,	0x06	},
    {	NULL,	"ldf",		S_LDF,		0,	0x0C	},
    {	NULL,	"ldw",		S_LDW,		0,	0x0E	},

    {	NULL,	"mov",		S_MOV,		0,	0x05	},

    {	NULL,	"adc",		S_AOP,		0,	0x09	},
    {	NULL,	"add",		S_AOP,		0,	0x0B	},
    {	NULL,	"and",		S_AOP,		0,	0x04	},
    {	NULL,	"bcp",		S_AOP,		0,	0x05	},
    {	NULL,	"cp",		S_AOP,		0,	0x01	},
    {	NULL,	"or",		S_AOP,		0,	0x0A	},
    {	NULL,	"sub",		S_AOP,		0,	0x00	},
    {	NULL,	"sbc",		S_AOP,		0,	0x02	},
    {	NULL,	"xor",		S_AOP,		0,	0x08	},

    {	NULL,	"addw",		S_ADDW,		0,	0x0C	},
    {	NULL,	"subw",		S_SUBW,		0,	0x0D	},

    {	NULL,	"cpw",		S_CPW,		0,	0x03	},

    {	NULL,	"clr",		S_BOP,		0,	0x0F	},
    {	NULL,	"cpl",		S_BOP,		0,	0x03	},
    {	NULL,	"dec",		S_BOP,		0,	0x0A	},
    {	NULL,	"inc",		S_BOP,		0,      0x0C	},
    {	NULL,	"neg",		S_BOP,		0,	0x00	},
    {	NULL,	"rlc",		S_BOP,		0,	0x09	},
    {	NULL,	"rrc",		S_BOP,		0,	0x06	},
    {	NULL,	"sla",		S_BOP,		0,	0x08	},
    {	NULL,	"sll",		S_BOP,		0,	0x08	},
    {	NULL,	"sra",		S_BOP,		0,	0x07	},
    {	NULL,	"srl",		S_BOP,		0,	0x04	},
    {	NULL,	"tnz",		S_BOP,		0,	0x0D	},
    {	NULL,	"swap",		S_BOP,		0,	0x0E	},

    {	NULL,	"clrw",		S_WOP,		0,	0x5F	},
    {	NULL,	"cplw",		S_WOP,		0,	0x53	},
    {	NULL,	"decw",		S_WOP,		0,	0x5A	},
    {	NULL,	"incw",		S_WOP,		0,      0x5C	},
    {	NULL,	"negw",		S_WOP,		0,	0x50	},
    {	NULL,	"rlcw",		S_WOP,		0,	0x59	},
    {	NULL,	"rrcw",		S_WOP,		0,	0x56	},
    {	NULL,	"slaw",		S_WOP,		0,	0x58	},
    {	NULL,	"sllw",		S_WOP,		0,	0x58	},
    {	NULL,	"sraw",		S_WOP,		0,	0x57	},
    {	NULL,	"srlw",		S_WOP,		0,	0x54	},
    {	NULL,	"tnzw",		S_WOP,		0,	0x5D	},
    {	NULL,	"swapw",	S_WOP,		0,	0x5E	},

    {	NULL,	"mul",		S_MLDV,		0,	0x42	},
    {	NULL,	"div",		S_MLDV,		0,	0x62	},
    {	NULL,	"divw",		S_DIVW,		0,	0x65	},

    {	NULL,	"rlwa",		S_RWA,		0,	0x02	},
    {	NULL,	"rrwa",		S_RWA,		0,	0x01	},

    {	NULL,	"exg",		S_EXG,		0,	0x01	},
    {	NULL,	"exgw",		S_EXGW,	   	0,	0x51	},

    {	NULL,	"pop",		S_POP,	   	0,	0x32	},
    {	NULL,	"push",		S_PUSH,		0,	0x3B	},

    {	NULL,	"popw",		S_PW,	   	0,	0x85	},
    {	NULL,	"pushw",	S_PW,		0,	0x89	},

    {	NULL,	"call",		S_CLJP,		0,	0x0D	},
    {	NULL,	"callf",	S_CLJPF,	0,	0x8D	},
    {	NULL,	"callr",	S_CALLR,	0,	0xAD	},
    {	NULL,	"jp",		S_CLJP,		0,	0x0C	},
    {	NULL,	"jpf",		S_CLJPF,	0,	0xAC	},

    {	NULL,	"break", 	S_INH,		0,	0x8B	},
    {	NULL,	"ccf",		S_INH,	   	0,	0x8C	},
    {	NULL,	"halt",		S_INH,	   	0,	0x8E	},
    {	NULL,	"int",		S_INT,		0,	0x0C	},
    {	NULL,	"iret",		S_INH,		0,	0x80	},
    {	NULL,	"nop",		S_INH,		0,	0x9D	},
    {	NULL,	"rcf",		S_INH,		0,	0x98	},
    {	NULL,	"ret",		S_INH,	   	0,	0x81	},
    {	NULL,	"retf",		S_INH,	   	0,	0x87	},
    {	NULL,	"rim",		S_INH,		0,	0x9A	},
    {	NULL,	"rvf",		S_INH,		0,	0x9C	},
    {	NULL,	"scf",		S_INH,		0,	0x99	},
    {	NULL,	"sim",		S_INH,		0,	0x9B	},
    {	NULL,	"trap",		S_INH,	   	0,	0x83	},
    {	NULL,	"wfi",		S_INH,		0,	0x8F	},

    {	NULL,	"wfe",		S_INH72,	S_EOL,	0x8F	}

};
