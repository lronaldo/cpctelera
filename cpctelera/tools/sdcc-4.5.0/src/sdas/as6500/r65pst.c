/* r65pst.c */

/*
 *  Copyright (C) 1995-2023  Alan R. Baldwin
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
 */

/*
 * With Contributions from
 *
 * Marko Makela
 * Sillitie 10 A
 * 01480 Vantaa
 * Finland
 * Internet: Marko dot Makela at Helsinki dot Fi
 * EARN/BitNet: msmakela at finuh
 */

#include "asxxxx.h"
#include "r6500.h"

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

/* None Required */

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
struct	mode	mode[1] = {
    {	&mode0[0],	0,	0x0000FFFF,	0x0000FFFF	}
};

/*
 * Array of Pointers to mode Structures
 */
struct	mode	*modep[16] = {
	&mode[0],	NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL
};

#endif

/*
 * Mnemonic Structure
 */
struct	mne	mne[] = {

        /* machine */

//    {	NULL,	"CSEG",		S_ATYP,		0,	A_CSEG|A_1BYTE	},
//    {	NULL,	"DSEG",		S_ATYP,		0,	A_DSEG|A_1BYTE	},

        /* system */

//    {	NULL,	"BANK",		S_ATYP,		0,	A_BNK	},
    {   NULL,   "CON",          S_ATYP,         0,      A_CON   },
    {   NULL,   "OVR",          S_ATYP,         0,      A_OVR   },
    {   NULL,   "REL",          S_ATYP,         0,      A_REL   },
    {   NULL,   "ABS",          S_ATYP,         0,      A_ABS   },
    {   NULL,   "NOPAG",        S_ATYP,         0,      A_NOPAG },
    {   NULL,   "PAG",          S_ATYP,         0,      A_PAG   },

    {   NULL,   "CODE",         S_ATYP,         0,      A_CODE  },
    {   NULL,   "DATA",         S_ATYP,         0,      A_DATA  },
    {   NULL,   "LOAD",         S_ATYP,         0,      A_LOAD  },
    {   NULL,   "NOLOAD",       S_ATYP,         0,      A_NOLOAD },

    {   NULL,   ".page",        S_PAGE,         0,      0       },
    {   NULL,   ".title",       S_HEADER,       0,      O_TITLE },
    {   NULL,   ".sbttl",       S_HEADER,       0,      O_SBTTL },
    {   NULL,   ".module",      S_MODUL,        0,      0       },
    {	NULL,	".include",	S_INCL,		0,	I_CODE	},
    {	NULL,	".incbin",	S_INCL,		0,	I_BNRY	},
    {   NULL,   ".area",        S_AREA,         0,      0       },
//    {	NULL,	".psharea",	S_AREA,		0,	O_PSH	},
//    {	NULL,	".poparea",	S_AREA,		0,	O_POP	},
//    {	NULL,	".bank",	S_BANK,		0,	0	},
    {   NULL,   ".org",         S_ORG,          0,      0       },
    {   NULL,   ".radix",       S_RADIX,        0,      0       },
    {   NULL,   ".globl",       S_GLOBL,        0,      0       },
    {   NULL,   ".local",       S_LOCAL,        0,      0       },
    {   NULL,   ".if",          S_CONDITIONAL,  0,      O_IF    },
    {   NULL,   ".iff",         S_CONDITIONAL,  0,      O_IFF   },
    {   NULL,   ".ift",         S_CONDITIONAL,  0,      O_IFT   },
    {   NULL,   ".iftf",        S_CONDITIONAL,  0,      O_IFTF  },
    {	NULL,	".ifdef",	S_CONDITIONAL,	0,	O_IFDEF	},
    {	NULL,	".ifndef",	S_CONDITIONAL,	0,	O_IFNDEF},
    {   NULL,   ".ifgt",        S_CONDITIONAL,  0,      O_IFGT  },
    {   NULL,   ".iflt",        S_CONDITIONAL,  0,      O_IFLT  },
    {   NULL,   ".ifge",        S_CONDITIONAL,  0,      O_IFGE  },
    {   NULL,   ".ifle",        S_CONDITIONAL,  0,      O_IFLE  },
    {   NULL,   ".ifeq",        S_CONDITIONAL,  0,      O_IFEQ  },
    {   NULL,   ".ifne",        S_CONDITIONAL,  0,      O_IFNE  },
    {	NULL,	".ifb",		S_CONDITIONAL,	0,	O_IFB	},
    {	NULL,	".ifnb",	S_CONDITIONAL,	0,	O_IFNB	},
    {	NULL,	".ifidn",	S_CONDITIONAL,	0,	O_IFIDN	},
    {	NULL,	".ifdif",	S_CONDITIONAL,	0,	O_IFDIF	},
    {   NULL,   ".iif",         S_CONDITIONAL,  0,      O_IIF   },
    {   NULL,   ".iiff",        S_CONDITIONAL,  0,      O_IIFF  },
    {   NULL,   ".iift",        S_CONDITIONAL,  0,      O_IIFT  },
    {   NULL,   ".iiftf",       S_CONDITIONAL,  0,      O_IIFTF },
    {	NULL,	".iifdef",	S_CONDITIONAL,	0,	O_IIFDEF},
    {	NULL,	".iifndef",	S_CONDITIONAL,	0,	O_IIFNDEF},
    {   NULL,   ".iifgt",       S_CONDITIONAL,  0,      O_IIFGT },
    {   NULL,   ".iiflt",       S_CONDITIONAL,  0,      O_IIFLT },
    {   NULL,   ".iifge",       S_CONDITIONAL,  0,      O_IIFGE },
    {   NULL,   ".iifle",       S_CONDITIONAL,  0,      O_IIFLE },
    {   NULL,   ".iifeq",       S_CONDITIONAL,  0,      O_IIFEQ },
    {   NULL,   ".iifne",       S_CONDITIONAL,  0,      O_IIFNE },
    {	NULL,	".iifb",	S_CONDITIONAL,	0,	O_IIFB	},
    {	NULL,	".iifnb",	S_CONDITIONAL,	0,	O_IIFNB	},
    {	NULL,	".iifidn",	S_CONDITIONAL,	0,	O_IIFIDN},
    {	NULL,	".iifdif",	S_CONDITIONAL,	0,	O_IIFDIF},
    {   NULL,   ".else",        S_CONDITIONAL,  0,      O_ELSE  },
    {   NULL,   ".endif",       S_CONDITIONAL,  0,      O_ENDIF },
    {   NULL,   ".list",        S_LISTING,      0,      O_LIST  },
    {   NULL,   ".nlist",       S_LISTING,      0,      O_NLIST },
    {   NULL,   ".uleb128",     S_ULEB128,      0,      0       },
    {   NULL,   ".sleb128",     S_SLEB128,      0,      0       },
    {   NULL,   ".equ",         S_EQU,          0,      O_EQU   },
    {   NULL,   ".gblequ",      S_EQU,          0,      O_GBLEQU},
    {   NULL,   ".lclequ",      S_EQU,          0,      O_LCLEQU},
    {   NULL,   ".byte",        S_DATA,         0,      O_1BYTE },
    {   NULL,   ".db",          S_DATA,         0,      O_1BYTE },
    {   NULL,   ".fcb",         S_DATA,         0,      O_1BYTE },
    {   NULL,   ".word",        S_DATA,         0,      O_2BYTE },
    {   NULL,   ".dw",          S_DATA,         0,      O_2BYTE },
    {   NULL,   ".fdb",         S_DATA,         0,      O_2BYTE },
/*    { NULL,   ".3byte",       S_DATA,         0,      O_3BYTE },      */
/*    { NULL,   ".triple",      S_DATA,         0,      O_3BYTE },      */
/*    {	NULL,	".dl",		S_DATA,		0,	O_4BYTE	},	*/
/*    { NULL,   ".4byte",       S_DATA,         0,      O_4BYTE },      */
/*    { NULL,   ".quad",        S_DATA,         0,      O_4BYTE },      */
/*    {	NULL,	".long",	S_DATA,		0,	O_4BYTE	},	*/
    {   NULL,   ".blkb",        S_BLK,          0,      O_1BYTE },
    {   NULL,   ".ds",          S_BLK,          0,      O_1BYTE },
    {   NULL,   ".rmb",         S_BLK,          0,      O_1BYTE },
    {   NULL,   ".rs",          S_BLK,          0,      O_1BYTE },
    {   NULL,   ".blkw",        S_BLK,          0,      O_2BYTE },
/*    { NULL,   ".blk3",        S_BLK,          0,      O_3BYTE },      */
/*    { NULL,   ".blk4",        S_BLK,          0,      O_4BYTE },      */
/*    {	NULL,	".blkl",	S_BLK,		0,	O_4BYTE	},	*/
    {   NULL,   ".ascii",       S_ASCIX,        0,      O_ASCII },
    {   NULL,   ".ascis",       S_ASCIX,        0,      O_ASCIS },
    {   NULL,   ".asciz",       S_ASCIX,        0,      O_ASCIZ },
    {   NULL,   ".str",         S_ASCIX,        0,      O_ASCII },
    {   NULL,   ".strs",        S_ASCIX,        0,      O_ASCIS },
    {   NULL,   ".strz",        S_ASCIX,        0,      O_ASCIZ },
    {   NULL,   ".fcc",         S_ASCIX,        0,      O_ASCII },
    {	NULL,	".define",	S_DEFINE,	0,	O_DEF	},
    {	NULL,	".undefine",	S_DEFINE,	0,	O_UNDEF	},
    {   NULL,   ".even",        S_BOUNDARY,     0,      O_EVEN  },
    {   NULL,   ".odd",         S_BOUNDARY,     0,      O_ODD   },
    {   NULL,   ".bndry",       S_BOUNDARY,     0,      O_BNDRY },
    {	NULL,	".msg",		S_MSG,		0,	0	},
    {	NULL,   ".assume",      S_ERROR,        0,      O_ASSUME},
    {	NULL,   ".error",       S_ERROR,        0,      O_ERROR	},
/*    {	NULL,	".msb",		S_MSB,		0,	0	},	*/
/*    {	NULL,	".lohi",	S_MSB,		0,	O_LOHI	},	*/
/*    {	NULL,	".hilo",	S_MSB,		0,	O_HILO	},	*/
/*    {	NULL,	".8bit",	S_BITS,		0,	O_1BYTE	},	*/
/*    {	NULL,	".16bit",	S_BITS,		0,	O_2BYTE	},	*/
/*    {	NULL,	".24bit",	S_BITS,		0,	O_3BYTE	},	*/
/*    {	NULL,	".32bit",	S_BITS,		0,	O_4BYTE	},	*/
//    {	NULL,	".trace",	S_TRACE,	0,	O_TRC	},
//    {	NULL,	".ntrace",	S_TRACE,	0,	O_NTRC	},
/*    {	NULL,	"_______",	S_CONST,	0,	VALUE	},	*/
//    {	NULL,	".end",		S_END,		0,	0	},

/* sdas specific */
    {   NULL,   ".optsdcc",     S_OPTSDCC,      0,      0       },
/* end sdas specific */


        /* Macro Processor */

    {   NULL,   ".macro",       S_MACRO,        0,      O_MACRO },
    {   NULL,   ".endm",        S_MACRO,        0,      O_ENDM  },
    {   NULL,   ".mexit",       S_MACRO,        0,      O_MEXIT },

    {   NULL,   ".narg",        S_MACRO,        0,      O_NARG  },
    {   NULL,   ".nchr",        S_MACRO,        0,      O_NCHR  },
    {   NULL,   ".ntyp",        S_MACRO,        0,      O_NTYP  },

    {   NULL,   ".irp",         S_MACRO,        0,      O_IRP   },
    {   NULL,   ".irpc",        S_MACRO,        0,      O_IRPC  },
    {   NULL,   ".rept",        S_MACRO,        0,      O_REPT  },

    {   NULL,   ".nval",        S_MACRO,        0,      O_NVAL  },

    {   NULL,   ".mdelete",     S_MACRO,        0,      O_MDEL  },

	/* Special */

    {   NULL,   ".setdp",       S_SDP,          0,      0       },
//    {	NULL,	".dpgbl",	S_PGD,		0,	0	},

	/* Machines */

    {	NULL,	".r6500",	S_CPU,		0,	X_R6500	},
    {	NULL,	".r65f11",	S_CPU,		0,	X_R65F11},
    {	NULL,	".r65c00",	S_CPU,		0,	X_R65C00},
    {	NULL,	".r65c02",	S_CPU,		0,	X_R65C02},

	/* 650X / 651X Family  Instructions */

    {	NULL,	"adc",		S_DOP,		0,	0x60	},
    {	NULL,	"and",		S_DOP,		0,	0x20	},
    {	NULL,	"cmp",		S_DOP,		0,	0xC0	},
    {	NULL,	"eor",		S_DOP,		0,	0x40	},
    {	NULL,	"lda",		S_DOP,		0,	0xA0	},
    {	NULL,	"ora",		S_DOP,		0,	0x00	},
    {	NULL,	"sbc",		S_DOP,		0,	0xE0	},
    {	NULL,	"sta",		S_DOP,		0,	0x80	},

    {	NULL,	"asl",		S_SOP,		0,	0x00	},
    {	NULL,	"lsr",		S_SOP,		0,	0x40	},
    {	NULL,	"rol",		S_SOP,		0,	0x20	},
    {	NULL,	"ror",		S_SOP,		0,	0x60	},
    {	NULL,	"dec",		S_SOP,		0,	0xC0	},
    {	NULL,	"inc",		S_SOP,		0,	0xE0	},

    {	NULL,	"bpl",		S_BRA1,		0,	0x10	},
    {	NULL,	"bmi",		S_BRA1,		0,	0x30	},
    {	NULL,	"bvc",		S_BRA1,		0,	0x50	},
    {	NULL,	"bvs",		S_BRA1,		0,	0x70	},
    {	NULL,	"bcc",		S_BRA1,		0,	0x90	},
    {	NULL,	"bhs",		S_BRA1,		0,	0x90	},
    {	NULL,	"bcs",		S_BRA1,		0,	0xB0	},
    {	NULL,	"blo",		S_BRA1,		0,	0xB0	},
    {	NULL,	"bne",		S_BRA1,		0,	0xD0	},
    {	NULL,	"beq",		S_BRA1,		0,	0xF0	},

    {	NULL,	"bit",		S_BIT,		0,	0x20	},

    {	NULL,	"brk",		S_INH1,		0,	0x00	},
    {	NULL,	"clc",		S_INH1,		0,	0x18	},
    {	NULL,	"cld",		S_INH1,		0,	0xD8	},
    {	NULL,	"cli",		S_INH1,		0,	0x58	},
    {	NULL,	"clv",		S_INH1,		0,	0xB8	},
    {	NULL,	"dex",		S_INH1,		0,	0xCA	},
    {	NULL,	"dey",		S_INH1,		0,	0x88	},
    {	NULL,	"inx",		S_INH1,		0,	0xE8	},
    {	NULL,	"iny",		S_INH1,		0,	0xC8	},
    {	NULL,	"nop",		S_INH1,		0,	0xEA	},
    {	NULL,	"pha",		S_INH1,		0,	0x48	},
    {	NULL,	"php",		S_INH1,		0,	0x08	},
    {	NULL,	"pla",		S_INH1,		0,	0x68	},
    {	NULL,	"plp",		S_INH1,		0,	0x28	},
    {	NULL,	"rti",		S_INH1,		0,	0x40	},
    {	NULL,	"rts",		S_INH1,		0,	0x60	},
    {	NULL,	"sec",		S_INH1,		0,	0x38	},
    {	NULL,	"sed",		S_INH1,		0,	0xF8	},
    {	NULL,	"sei",		S_INH1,		0,	0x78	},
    {	NULL,	"tax",		S_INH1,		0,	0xAA	},
    {	NULL,	"tay",		S_INH1,		0,	0xA8	},
    {	NULL,	"tsx",		S_INH1,		0,	0xBA	},
    {	NULL,	"txa",		S_INH1,		0,	0x8A	},
    {	NULL,	"txs",		S_INH1,		0,	0x9A	},
    {	NULL,	"tya",		S_INH1,		0,	0x98	},

    {	NULL,	"cpx",		S_CP,		0,	0xE0	},
    {	NULL,	"cpy",		S_CP,		0,	0xC0	},

    {	NULL,	"ldx",		S_LDSTX,	0,	0xA0	},
    {	NULL,	"stx",		S_LDSTX,	0,	0x80	},

    {	NULL,	"ldy",		S_LDSTY,	0,	0xA0	},
    {	NULL,	"sty",		S_LDSTY,	0,	0x80	},

    {	NULL,	"jmp",		S_JMP,		0,	0x4C	},

    {	NULL,	"jsr",		S_JSR,		0,	0x20	},

	/* Additional R65F1X series Instructions */
	
    {	NULL,	"bbr0",		S_BB,		0,	0x0F	},
    {	NULL,	"bbr1",		S_BB,		0,	0x1F	},
    {	NULL,	"bbr2",		S_BB,		0,	0x2F	},
    {	NULL,	"bbr3",		S_BB,		0,	0x3F	},
    {	NULL,	"bbr4",		S_BB,		0,	0x4F	},
    {	NULL,	"bbr5",		S_BB,		0,	0x5F	},
    {	NULL,	"bbr6",		S_BB,		0,	0x6F	},
    {	NULL,	"bbr7",		S_BB,		0,	0x7F	},
    {	NULL,	"bbs0",		S_BB,		0,	0x8F	},
    {	NULL,	"bbs1",		S_BB,		0,	0x9F	},
    {	NULL,	"bbs2",		S_BB,		0,	0xAF	},
    {	NULL,	"bbs3",		S_BB,		0,	0xBF	},
    {	NULL,	"bbs4",		S_BB,		0,	0xCF	},
    {	NULL,	"bbs5",		S_BB,		0,	0xDF	},
    {	NULL,	"bbs6",		S_BB,		0,	0xEF	},
    {	NULL,	"bbs7",		S_BB,		0,	0xFF	},

    {	NULL,	"rmb0",		S_MB,		0,	0x07	},
    {	NULL,	"rmb1",		S_MB,		0,	0x17	},
    {	NULL,	"rmb2",		S_MB,		0,	0x27	},
    {	NULL,	"rmb3",		S_MB,		0,	0x37	},
    {	NULL,	"rmb4",		S_MB,		0,	0x47	},
    {	NULL,	"rmb5",		S_MB,		0,	0x57	},
    {	NULL,	"rmb6",		S_MB,		0,	0x67	},
    {	NULL,	"rmb7",		S_MB,		0,	0x77	},
    {	NULL,	"smb0",		S_MB,		0,	0x87	},
    {	NULL,	"smb1",		S_MB,		0,	0x97	},
    {	NULL,	"smb2",		S_MB,		0,	0xA7	},
    {	NULL,	"smb3",		S_MB,		0,	0xB7	},
    {	NULL,	"smb4",		S_MB,		0,	0xC7	},
    {	NULL,	"smb5",		S_MB,		0,	0xD7	},
    {	NULL,	"smb6",		S_MB,		0,	0xE7	},
    {	NULL,	"smb7",		S_MB,		0,	0xF7	},
	
	/* Additional R65C00 series Instructions */
	
    {	NULL,	"bra",		S_BRA2,		0,	0x80	},
	
    {	NULL,	"phx",		S_INH2,		0,	0xDA	},
    {	NULL,	"phy",		S_INH2,		0,	0x5A	},
    {	NULL,	"plx",		S_INH2,		0,	0xFA	},
    {	NULL,	"ply",		S_INH2,		0,	0x7A	},

    {	NULL,	"mul",		S_INH3,		0,	0x02	},

	/* Additional R65C02 series Instructions */

    {	NULL,	"stz",		S_STZ,		0,	0x60	},

    {	NULL,	"trb",		S_TB,		0,	0x10	},
    {	NULL,	"tsb",		S_TB,		S_EOL,	0x00	}
};
