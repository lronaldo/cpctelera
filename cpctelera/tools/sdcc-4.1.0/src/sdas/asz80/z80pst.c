/* z80pst.c */

/*
 *  Copyright (C) 1989-2009  Alan R. Baldwin
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
 * Extensions: P. Felber
 */

#include "asxxxx.h"
#include "z80.h"

/*
 * Mnemonic Structure
 */
struct  mne     mne[] = {

        /* machine */

        /* system */

    {   NULL,   "CON",          S_ATYP,         0,      A_CON   },
    {   NULL,   "OVR",          S_ATYP,         0,      A_OVR   },
    {   NULL,   "REL",          S_ATYP,         0,      A_REL   },
    {   NULL,   "ABS",          S_ATYP,         0,      A_ABS   },
    {   NULL,   "NOPAG",        S_ATYP,         0,      A_NOPAG },
    {   NULL,   "PAG",          S_ATYP,         0,      A_PAG   },


    {   NULL,   ".page",        S_PAGE,         0,      0       },
    {   NULL,   ".title",       S_HEADER,       0,      O_TITLE },
    {   NULL,   ".sbttl",       S_HEADER,       0,      O_SBTTL },
    {   NULL,   ".module",      S_MODUL,        0,      0       },
    {   NULL,   ".include",     S_INCL,         0,      0       },
    {   NULL,   ".area",        S_AREA,         0,      0       },

    {   NULL,   ".org",         S_ORG,          0,      0       },
    {   NULL,   ".radix",       S_RADIX,        0,      0       },
    {   NULL,   ".globl",       S_GLOBL,        0,      0       },
    {   NULL,   ".local",       S_LOCAL,        0,      0       },
    {   NULL,   ".if",          S_CONDITIONAL,  0,      O_IF    },
    {   NULL,   ".iff",         S_CONDITIONAL,  0,      O_IFF   },
    {   NULL,   ".ift",         S_CONDITIONAL,  0,      O_IFT   },
    {   NULL,   ".iftf",        S_CONDITIONAL,  0,      O_IFTF  },
    {   NULL,   ".ifgt",        S_CONDITIONAL,  0,      O_IFGT  },
    {   NULL,   ".iflt",        S_CONDITIONAL,  0,      O_IFLT  },
    {   NULL,   ".ifge",        S_CONDITIONAL,  0,      O_IFGE  },
    {   NULL,   ".ifle",        S_CONDITIONAL,  0,      O_IFLE  },
    {   NULL,   ".ifeq",        S_CONDITIONAL,  0,      O_IFEQ  },
    {   NULL,   ".ifne",        S_CONDITIONAL,  0,      O_IFNE  },
    {   NULL,   ".iif",         S_CONDITIONAL,  0,      O_IIF   },
    {   NULL,   ".iiff",        S_CONDITIONAL,  0,      O_IIFF  },
    {   NULL,   ".iift",        S_CONDITIONAL,  0,      O_IIFT  },
    {   NULL,   ".iiftf",       S_CONDITIONAL,  0,      O_IIFTF },
    {   NULL,   ".iifgt",       S_CONDITIONAL,  0,      O_IIFGT },
    {   NULL,   ".iiflt",       S_CONDITIONAL,  0,      O_IIFLT },
    {   NULL,   ".iifge",       S_CONDITIONAL,  0,      O_IIFGE },
    {   NULL,   ".iifle",       S_CONDITIONAL,  0,      O_IIFLE },
    {   NULL,   ".iifeq",       S_CONDITIONAL,  0,      O_IIFEQ },
    {   NULL,   ".iifne",       S_CONDITIONAL,  0,      O_IIFNE },
    {   NULL,   ".else",        S_CONDITIONAL,  0,      O_ELSE  },
    {   NULL,   ".endif",       S_CONDITIONAL,  0,      O_ENDIF },
    {   NULL,   ".list",        S_LISTING,      0,      O_LIST  },
    {   NULL,   ".nlist",       S_LISTING,      0,      O_NLIST },
    {   NULL,   ".equ",         S_EQU,          0,      O_EQU   },
    {   NULL,   ".gblequ",      S_EQU,          0,      O_GBLEQU},
    {   NULL,   ".lclequ",      S_EQU,          0,      O_LCLEQU},
    {   NULL,   ".byte",        S_DATA,         0,      O_1BYTE },
    {   NULL,   ".db",          S_DATA,         0,      O_1BYTE },
    {   NULL,   ".fcb",         S_DATA,         0,      O_1BYTE },
    {   NULL,   ".word",        S_DATA,         0,      O_2BYTE },
    {   NULL,   ".dw",          S_DATA,         0,      O_2BYTE },
    {   NULL,   ".fdb",         S_DATA,         0,      O_2BYTE },
    {   NULL,   ".3byte",       S_DATA,         0,      O_3BYTE },
    {   NULL,   ".triple",      S_DATA,         0,      O_3BYTE },
/*    { NULL,   ".4byte",       S_DATA,         0,      O_4BYTE },      */
/*    { NULL,   ".quad",        S_DATA,         0,      O_4BYTE },      */
    {   NULL,   ".df",          S_FLOAT,        0,      0       },
    {   NULL,   ".blkb",        S_BLK,          0,      O_1BYTE },
    {   NULL,   ".ds",          S_BLK,          0,      O_1BYTE },
    {   NULL,   ".rmb",         S_BLK,          0,      O_1BYTE },
    {   NULL,   ".rs",          S_BLK,          0,      O_1BYTE },
    {   NULL,   ".blkw",        S_BLK,          0,      O_2BYTE },
    {   NULL,   ".blk3",        S_BLK,          0,      O_3BYTE },
/*    { NULL,   ".blk4",        S_BLK,          0,      O_4BYTE },      */
    {   NULL,   ".ascii",       S_ASCIX,        0,      O_ASCII },
    {   NULL,   ".ascis",       S_ASCIX,        0,      O_ASCIS },
    {   NULL,   ".asciz",       S_ASCIX,        0,      O_ASCIZ },
    {   NULL,   ".str",         S_ASCIX,        0,      O_ASCII },
    {   NULL,   ".strs",        S_ASCIX,        0,      O_ASCIS },
    {   NULL,   ".strz",        S_ASCIX,        0,      O_ASCIZ },
    {   NULL,   ".fcc",         S_ASCIX,        0,      O_ASCII },
    {   NULL,   ".even",        S_BOUNDARY,     0,      O_EVEN  },
    {   NULL,   ".odd",         S_BOUNDARY,     0,      O_ODD   },
    {   NULL,   ".bndry",       S_BOUNDARY,     0,      O_BNDRY },
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

        /* Machines */

    {   NULL,   ".z80",         S_CPU,          0,      X_Z80   },
    {   NULL,   ".hd64",        S_CPU,          0,      X_HD64  },
    {   NULL,   ".z180",        S_CPU,          0,      X_HD64  },
    {   NULL,   ".zxn",         S_CPU,          0,      X_ZXN   },
    {   NULL,   ".ez80",        S_CPU,          0,      X_EZ80  },

	/* z80 / hd64180 */

    {   NULL,   "ld",           S_LD,           0,      0x40    },

    {   NULL,   "call",         S_CALL,         0,      0xC4    },
    {   NULL,   "jp",           S_JP,           0,      0xC2    },
    {   NULL,   "jr",           S_JR,           0,      0x18    },
    {   NULL,   "djnz",         S_DJNZ,         0,      0x10    },
    {   NULL,   "ret",          S_RET,          0,      0xC0    },

    {   NULL,   "bit",          S_BIT,          0,      0x40    },
    {   NULL,   "res",          S_BIT,          0,      0x80    },
    {   NULL,   "set",          S_BIT,          0,      0xC0    },

    {   NULL,   "inc",          S_INC,          0,      0x04    },
    {   NULL,   "dec",          S_DEC,          0,      0x05    },

    {   NULL,   "add",          S_ADD,          0,      0x80    },
    {   NULL,   "adc",          S_ADC,          0,      0x88    },
    {   NULL,   "sub",          S_SUB,          0,      0x90    },
    {   NULL,   "sbc",          S_SBC,          0,      0x98    },

    {   NULL,   "and",          S_AND,          0,      0xA0    },
    {   NULL,   "cp",           S_AND,          0,      0xB8    },
    {   NULL,   "or",           S_AND,          0,      0xB0    },
    {   NULL,   "xor",          S_AND,          0,      0xA8    },

    {   NULL,   "ex",           S_EX,           0,      0xE3    },

    {   NULL,   "push",         S_PUSH,         0,      0xC5    },
    {   NULL,   "pop",          S_PUSH,         0,      0xC1    },

    {   NULL,   "in",           S_IN,           0,      0xDB    },
    {   NULL,   "out",          S_OUT,          0,      0xD3    },

    {   NULL,   "rl",           S_RL,           0,      0x10    },
    {   NULL,   "rlc",          S_RL,           0,      0x00    },
    {   NULL,   "rr",           S_RL,           0,      0x18    },
    {   NULL,   "rrc",          S_RL,           0,      0x08    },
    {   NULL,   "sla",          S_RL,           0,      0x20    },
    {   NULL,   "sra",          S_RL,           0,      0x28    },
    {   NULL,   "sll",          S_RL_UNDOCD,    0,      0x30    },
    {   NULL,   "srl",          S_RL,           0,      0x38    },

    {   NULL,   "rst",          S_RST,          0,      0xC7    },

    {   NULL,   "im",           S_IM,           0,      0xED    },

    {   NULL,   "ccf",          S_INH1,         0,      0x3F    },
    {   NULL,   "cpl",          S_INH1,         0,      0x2F    },
    {   NULL,   "daa",          S_INH1,         0,      0x27    },
    {   NULL,   "di",           S_INH1,         0,      0xF3    },
    {   NULL,   "ei",           S_INH1,         0,      0xFB    },
    {   NULL,   "exx",          S_INH1,         0,      0xD9    },
    {   NULL,   "nop",          S_INH1,         0,      0x00    },
    {   NULL,   "halt",         S_INH1,         0,      0x76    },
    {   NULL,   "rla",          S_INH1,         0,      0x17    },
    {   NULL,   "rlca",         S_INH1,         0,      0x07    },
    {   NULL,   "rra",          S_INH1,         0,      0x1F    },
    {   NULL,   "rrca",         S_INH1,         0,      0x0F    },
    {   NULL,   "scf",          S_INH1,         0,      0x37    },

    {   NULL,   "cpd",          S_INH2,         0,      0xA9    },
    {   NULL,   "cpdr",         S_INH2,         0,      0xB9    },
    {   NULL,   "cpi",          S_INH2,         0,      0xA1    },
    {   NULL,   "cpir",         S_INH2,         0,      0xB1    },
    {   NULL,   "ind",          S_INH2,         0,      0xAA    },
    {   NULL,   "indr",         S_INH2,         0,      0xBA    },
    {   NULL,   "ini",          S_INH2,         0,      0xA2    },
    {   NULL,   "inir",         S_INH2,         0,      0xB2    },
    {   NULL,   "ldd",          S_INH2,         0,      0xA8    },
    {   NULL,   "lddr",         S_INH2,         0,      0xB8    },
    {   NULL,   "ldi",          S_INH2,         0,      0xA0    },
    {   NULL,   "ldir",         S_INH2,         0,      0xB0    },
    {   NULL,   "neg",          S_INH2,         0,      0x44    },
    {   NULL,   "otdr",         S_INH2,         0,      0xBB    },
    {   NULL,   "otir",         S_INH2,         0,      0xB3    },
    {   NULL,   "outd",         S_INH2,         0,      0xAB    },
    {   NULL,   "outi",         S_INH2,         0,      0xA3    },
    {   NULL,   "reti",         S_INH2,         0,      0x4D    },
    {   NULL,   "retn",         S_INH2,         0,      0x45    },
    {   NULL,   "rld",          S_INH2,         0,      0x6F    },
    {   NULL,   "rrd",          S_INH2,         0,      0x67    },

    {   NULL,   ".allow_undocumented", X_UNDOCD, 0,     0       },

        /* 64180 */

    {   NULL,   "otdm",         X_INH2,         0,      0x8B    },
    {   NULL,   "otdmr",        X_INH2,         0,      0x9B    },
    {   NULL,   "otim",         X_INH2,         0,      0x83    },
    {   NULL,   "otimr",        X_INH2,         0,      0x93    },
    {   NULL,   "slp",          X_INH2,         0,      0x76    },

    {   NULL,   "in0",          X_IN,           0,      0x00    },
    {   NULL,   "out0",         X_OUT,          0,      0x01    },

    {   NULL,   "mlt",          X_MLT,          0,      0x4C    },

    {   NULL,   "tst",          X_TST,          0,      0x04    },
    {   NULL,   "test",         X_TST,          0,      0x04    },
    {   NULL,   "tstio",        X_TSTIO,        0,      0x74    },

	/* z80-zxn */

    {   NULL,   "swapnib",      X_ZXN_INH2,     0,      0x23    },
    {   NULL,   "swap",         X_ZXN_INH2,     0,      0x23    },
    {   NULL,   "mul",          X_ZXN_MUL,      0,      0x30    },
    {   NULL,   "outinb",       X_ZXN_INH2,     0,      0x90    },
    {   NULL,   "ldix",         X_ZXN_INH2,     0,      0xA4    },
    {   NULL,   "ldirx",        X_ZXN_INH2,     0,      0xB4    },
    {   NULL,   "lddx",         X_ZXN_INH2,     0,      0xAC    },
    {   NULL,   "lddrx",        X_ZXN_INH2,     0,      0xBC    },
    {   NULL,   "ldirscale",    X_ZXN_INH2,     0,      0xB6    },
    {   NULL,   "ldpirx",       X_ZXN_INH2,     0,      0xB7    },
    {   NULL,   "mirror",       X_ZXN_MIRROR,   0,      0       },
    {   NULL,   "nextreg",      X_ZXN_NEXTREG,  0,      0       },
    {   NULL,   "pixeldn",      X_ZXN_INH2,     0,      0x93    },
    {   NULL,   "pixelad",      X_ZXN_INH2,     0,      0x94    },
    {   NULL,   "setae",        X_ZXN_INH2,     0,      0x95    },
    {   NULL,   "cu.wait",      X_ZXN_CU_WAIT,  0,      0       },
    {   NULL,   "cu.move",      X_ZXN_CU_MOVE,  0,      0       },
    {   NULL,   "cu.stop",      X_ZXN_CU_STOP,  0,      0       },
    {   NULL,   "cu.nop",       X_ZXN_CU_NOP,   0,      0       },
    {   NULL,   "bsla",         X_ZXN_INH2,     0,      0x28    },
    {   NULL,   "bsra",         X_ZXN_INH2,     0,      0x29    },
    {   NULL,   "bsrl",         X_ZXN_INH2,     0,      0x2a    },
    {   NULL,   "bsrf",         X_ZXN_INH2,     0,      0x2b    },
    {   NULL,   "brlc",         X_ZXN_INH2,     0,      0x2c    },

	/* eZ80 */

    {   NULL,   ".adl",         X_EZ_ADL,       0,      0       },

    {	NULL,	"ld.il",	S_LD,		M_IL,	0x40	},
    {	NULL,	"ld.is",	S_LD,		M_IS,	0x40	},
    {	NULL,	"ld.l",		S_LD,		M_L,	0x40	},
    {	NULL,	"ld.lil",	S_LD,		M_LIL,	0x40	},
    {	NULL,	"ld.s",		S_LD,		M_S,	0x40	},
    {	NULL,	"ld.sis",	S_LD,		M_SIS,	0x40	},

    {	NULL,	"call.il",	S_CALL,		M_IL,	0xC4	},
    {	NULL,	"call.is",	S_CALL,		M_IS,	0xC4	},

    {	NULL,	"jp.l",		S_JP,		M_L,	0xC2	},
    {	NULL,	"jp.lil",	S_JP,		M_LIL,	0xC2	},
    {	NULL,	"jp.s",		S_JP,		M_S,	0xC2	},
    {	NULL,	"jp.sis",	S_JP,		M_SIS,	0xC2	},

    {	NULL,	"ret.l",	S_RET,		M_L,	0xC0	},
    {	NULL,	"ret.s",	S_RET,		M_S,	0xC0	},

    {	NULL,	"bit.l",	S_BIT,		M_L,	0x40	},
    {	NULL,	"bit.s",	S_BIT,		M_S,	0x40	},
    {	NULL,	"res.l",	S_BIT,		M_L,	0x80	},
    {	NULL,	"res.s",	S_BIT,		M_S,	0x80	},
    {	NULL,	"set.l",	S_BIT,		M_L,	0xC0	},
    {	NULL,	"set.s",	S_BIT,		M_S,	0xC0	},

    {	NULL,	"inc.l",	S_INC,		M_L,	0x04	},
    {	NULL,	"inc.s",	S_INC,		M_S,	0x04	},
	
    {	NULL,	"dec.l",	S_DEC,		M_L,	0x05	},
    {	NULL,	"dec.s",	S_DEC,		M_S,	0x05	},

    {	NULL,	"add.l",	S_ADD,		M_L,	0x80	},
    {	NULL,	"add.s",	S_ADD,		M_S,	0x80	},

    {	NULL,	"adc.l",	S_ADC,		M_L,	0x88	},
    {	NULL,	"adc.s",	S_ADC,		M_S,	0x88	},

    {	NULL,	"sub.l",	S_SUB,		M_L,	0x90	},
    {	NULL,	"sub.s",	S_SUB,		M_S,	0x90	},

    {	NULL,	"sbc.l",	S_SBC,		M_L,	0x98	},
    {	NULL,	"sbc.s",	S_SBC,		M_S,	0x98	},

    {	NULL,	"and.l",	S_AND,		M_L,	0xA0	},
    {	NULL,	"and.s",	S_AND,		M_S,	0xA0	},
    {	NULL,	"cp.l",		S_AND,		M_L,	0xB8	},
    {	NULL,	"cp.s",		S_AND,		M_S,	0xB8	},
    {	NULL,	"or.l",		S_AND,		M_L,	0xB0	},
    {	NULL,	"or.s",		S_AND,		M_S,	0xB0	},
    {	NULL,	"xor.l",	S_AND,		M_L,	0xA8	},
    {	NULL,	"xor.s",	S_AND,		M_S,	0xA8	},

    {	NULL,	"ex.l",		S_EX,		M_L,	0xE3	},
    {	NULL,	"ex.s",		S_EX,		M_S,	0xE3	},

    {	NULL,	"push.l",	S_PUSH,		M_L,	0xC5	},
    {	NULL,	"push.s",	S_PUSH,		M_S,	0xC5	},
    {	NULL,	"pop.l",	S_PUSH,		M_L,	0xC1	},
    {	NULL,	"pop.s",	S_PUSH,		M_S,	0xC1	},

    {	NULL,	"rl.l",		S_RL,		M_L,	0x10	},
    {	NULL,	"rl.s",		S_RL,		M_S,	0x10	},
    {	NULL,	"rlc.l",	S_RL,		M_L,	0x00	},
    {	NULL,	"rlc.s",	S_RL,		M_S,	0x00	},
    {	NULL,	"rr.l",		S_RL,		M_L,	0x18	},
    {	NULL,	"rr.s",		S_RL,		M_S,	0x18	},
    {	NULL,	"rrc.l",	S_RL,		M_L,	0x08	},
    {	NULL,	"rrc.s",	S_RL,		M_S,	0x08	},
    {	NULL,	"sla.l",	S_RL,		M_L,	0x20	},
    {	NULL,	"sla.s",	S_RL,		M_S,	0x20	},
    {	NULL,	"sra.l",	S_RL,		M_L,	0x28	},
    {	NULL,	"sra.s",	S_RL,		M_S,	0x28	},
    {	NULL,	"srl.l",	S_RL,		M_L,	0x38	},
    {	NULL,	"srl.s",	S_RL,		M_S,	0x38	},

    {	NULL,	"rst.l",	S_RST,		M_L,	0xC7	},
    {	NULL,	"rst.s",	S_RST,		M_S,	0xC7	},

    {	NULL,	"cpd.l",	S_INH2,		M_L,	0xA9	},
    {	NULL,	"cpd.s",	S_INH2,		M_S,	0xA9	},
    {	NULL,	"cpdr.l",	S_INH2,		M_L,	0xB9	},
    {	NULL,	"cpdr.s",	S_INH2,		M_S,	0xB9	},
    {	NULL,	"cpi.l",	S_INH2,		M_L,	0xA1	},
    {	NULL,	"cpi.s",	S_INH2,		M_S,	0xA1	},
    {	NULL,	"cpir.l",	S_INH2,		M_L,	0xB1	},
    {	NULL,	"cpir.s",	S_INH2,		M_S,	0xB1	},
    {	NULL,	"ind.l",	S_INH2,		M_L,	0xAA	},
    {	NULL,	"ind.s",	S_INH2,		M_S,	0xAA	},
    {	NULL,	"ind2",		X_EZ_INH2,	0,	0x8C	},
    {	NULL,	"ind2.l",	X_EZ_INH2,	M_L,	0x8C	},
    {	NULL,	"ind2.s",	X_EZ_INH2,	M_S,	0x8C	},
    {	NULL,	"indr.l",	S_INH2,		M_L,	0xBA	},
    {	NULL,	"indr.s",	S_INH2,		M_S,	0xBA	},
    {	NULL,	"ind2r",	X_EZ_INH2,	0,	0x9C	},
    {	NULL,	"ind2r.l",	X_EZ_INH2,	M_L,	0x9C	},
    {	NULL,	"ind2r.s",	X_EZ_INH2,	M_S,	0x9C	},
    {	NULL,	"indrx",	X_EZ_INH2,	0,	0xCA	},
    {	NULL,	"indrx.l",	X_EZ_INH2,	M_L,	0xCA	},
    {	NULL,	"indrx.s",	X_EZ_INH2,	M_S,	0xCA	},
    {	NULL,	"indm",		X_EZ_INH2,	0,	0x8A	},
    {	NULL,	"indm.l",	X_EZ_INH2,	M_L,	0x8A	},
    {	NULL,	"indm.s",	X_EZ_INH2,	M_S,	0x8A	},
    {	NULL,	"indmr",	X_EZ_INH2,	0,	0x9A	},
    {	NULL,	"indmr.l",	X_EZ_INH2,	M_L,	0x9A	},
    {	NULL,	"indmr.s",	X_EZ_INH2,	M_S,	0x9A	},
    {	NULL,	"ini.l",	S_INH2,		M_L,	0xA2	},
    {	NULL,	"ini.s",	S_INH2,		M_S,	0xA2	},
    {	NULL,	"ini2",		X_EZ_INH2,	0,	0x84	},
    {	NULL,	"ini2.l",	X_EZ_INH2,	M_L,	0x84	},
    {	NULL,	"ini2.s",	X_EZ_INH2,	M_S,	0x84	},
    {	NULL,	"inir.l",	S_INH2,		M_L,	0xB2	},
    {	NULL,	"inir.s",	S_INH2,		M_S,	0xB2	},
    {	NULL,	"inirx",	X_EZ_INH2,	0,	0xC2	},
    {	NULL,	"inirx.l",	X_EZ_INH2,	M_L,	0xC2	},
    {	NULL,	"inirx.s",	X_EZ_INH2,	M_S,	0xC2	},
    {	NULL,	"ini2r",	X_EZ_INH2,	0,	0x94	},
    {	NULL,	"ini2r.l",	X_EZ_INH2,	M_L,	0x94	},
    {	NULL,	"ini2r.s",	X_EZ_INH2,	M_S,	0x94	},
    {	NULL,	"inim",		X_EZ_INH2,	0,	0x82	},
    {	NULL,	"inim.l",	X_EZ_INH2,	M_L,	0x82	},
    {	NULL,	"inim.s",	X_EZ_INH2,	M_S,	0x82	},
    {	NULL,	"inimr",	X_EZ_INH2,	0,	0x92	},
    {	NULL,	"inimr.l",	X_EZ_INH2,	M_L,	0x92	},
    {	NULL,	"inimr.s",	X_EZ_INH2,	M_S,	0x92	},
    {	NULL,	"ldd.l",	S_INH2,		M_L,	0xA8	},
    {	NULL,	"ldd.s",	S_INH2,		M_S,	0xA8	},
    {	NULL,	"lddr.l",	S_INH2,		M_L,	0xB8	},
    {	NULL,	"lddr.s",	S_INH2,		M_S,	0xB8	},
    {	NULL,	"ldi.l",	S_INH2,		M_L,	0xA0	},
    {	NULL,	"ldi.s",	S_INH2,		M_S,	0xA0	},
    {	NULL,	"ldir.l",	S_INH2,		M_L,	0xB0	},
    {	NULL,	"ldir.s",	S_INH2,		M_S,	0xB0	},
    {	NULL,	"otdr.l",	S_INH2,		M_L,	0xBB	},
    {	NULL,	"otdr.s",	S_INH2,		M_S,	0xBB	},
    {	NULL,	"otd2r",	X_EZ_INH2,	0,	0xBC	},
    {	NULL,	"otd2r.l",	X_EZ_INH2,	M_L,	0xBC	},
    {	NULL,	"otd2r.s",	X_EZ_INH2,	M_S,	0xBC	},
    {	NULL,	"otdrx",	X_EZ_INH2,	0,	0xCB	},
    {	NULL,	"otdrx.l",	X_EZ_INH2,	M_L,	0xCB	},
    {	NULL,	"otdrx.s",	X_EZ_INH2,	M_S,	0xCB	},
    {	NULL,	"otir.l",	S_INH2,		M_L,	0xB3	},
    {	NULL,	"otir.s",	S_INH2,		M_S,	0xB3	},
    {	NULL,	"oti2r",	X_EZ_INH2,	0,	0xB4	},
    {	NULL,	"oti2r.l",	X_EZ_INH2,	M_L,	0xB4	},
    {	NULL,	"oti2r.s",	X_EZ_INH2,	M_S,	0xB4	},
    {	NULL,	"otirx",	X_EZ_INH2,	0,	0xC3	},
    {	NULL,	"otirx.l",	X_EZ_INH2,	M_L,	0xC3	},
    {	NULL,	"otirx.s",	X_EZ_INH2,	M_S,	0xC3	},
    {	NULL,	"outd.l",	S_INH2,		M_L,	0xAB	},
    {	NULL,	"outd.s",	S_INH2,		M_S,	0xAB	},
    {	NULL,	"outd2",	X_EZ_INH2,	0,	0xAC	},
    {	NULL,	"outd2.l",	X_EZ_INH2,	M_L,	0xAC	},
    {	NULL,	"outd2.s",	X_EZ_INH2,	M_S,	0xAC	},
    {	NULL,	"outi.l",	S_INH2,		M_L,	0xA3	},
    {	NULL,	"outi.s",	S_INH2,		M_S,	0xA3	},
    {	NULL,	"outi2",	X_EZ_INH2,	0,	0xA4	},
    {	NULL,	"outi2.l",	X_EZ_INH2,	M_L,	0xA4	},
    {	NULL,	"outi2.s",	X_EZ_INH2,	M_S,	0xA4	},
    {	NULL,	"otdm.l",	X_INH2,		M_L,	0x8B	},
    {	NULL,	"otdm.s",	X_INH2,		M_S,	0x8B	},
    {	NULL,	"otdmr.l",	X_INH2,		M_L,	0x9B	},
    {	NULL,	"otdmr.s",	X_INH2,		M_S,	0x9B	},
    {	NULL,	"otim.l",	X_INH2,		M_L,	0x83	},
    {	NULL,	"otim.s",	X_INH2,		M_S,	0x83	},
    {	NULL,	"otimr.l",	X_INH2,		M_L,	0x93	},
    {	NULL,	"otimr.s",	X_INH2,		M_S,	0x93	},
    {	NULL,	"reti.l",	S_INH2,		M_L,	0x4D	},
    {	NULL,	"reti.s",	S_INH2,		M_S,	0x4D	},
    {	NULL,	"retn.l",	S_INH2,		M_L,	0x45	},
    {	NULL,	"retn.s",	S_INH2,		M_S,	0x45	},

    {	NULL,	"mlt.l",	X_MLT,		M_L,	0x4C	},
    {	NULL,	"mlt.s",	X_MLT,		M_S,	0x4C	},

    {	NULL,	"tst.l",	X_TST,		M_L,	0x04	},
    {	NULL,	"tst.s",	X_TST,		M_S,	0x04	},

    {	NULL,	"lea",		X_EZ_LEA,	0,	0x00	},
    {	NULL,	"lea.l",	X_EZ_LEA,	M_L,	0x00	},
    {	NULL,	"lea.s",	X_EZ_LEA,	M_S,	0x00	},

    {	NULL,	"pea",		X_EZ_PEA,	0,	0x65	},
    {	NULL,	"pea.l",	X_EZ_PEA,	M_L,	0x65	},
    {	NULL,	"pea.s",	X_EZ_PEA,	M_S,	0x65	},

    {	NULL,	"rsmix",	X_EZ_INH2,	0,	0x7E	},
    {	NULL,	"stmix",	X_EZ_INH2,	S_EOL,	0x7D	}

};
