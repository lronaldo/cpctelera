/* pdk14pst.c */

/*
 *  Copyright (C) 1998-2009  Alan R. Baldwin
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
 *   This Assembler Ported by
 *      John L. Hartman (JLH)
 *      jhartman at compuserve dot com
 *      noice at noicedebugger dot com
 *
 */

#include "asxxxx.h"
#include "pdk.h"

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

    {   NULL,   "CODE",         S_ATYP,         0,      A_CODE  },
    {   NULL,   "DATA",         S_ATYP,         0,      A_DATA  },
    {   NULL,   "XDATA",        S_ATYP,         0,      A_XDATA },
    {   NULL,   "BIT",          S_ATYP,         0,      A_BIT   },

    {   NULL,   ".page",        S_PAGE,         0,      0       },
    {   NULL,   ".title",       S_HEADER,       0,      O_TITLE },
    {   NULL,   ".sbttl",       S_HEADER,       0,      O_SBTTL },
    {   NULL,   ".module",      S_MODUL,        0,      0       },
    {	NULL,	".include",	S_INCL,		0,	I_CODE	},
    {	NULL,	".incbin",	S_INCL,		0,	I_BNRY	},
    {   NULL,   ".area",        S_AREA,         0,      0       },
    {   NULL,   ".org",         S_ORG,          0,      0       },
    {   NULL,   ".radix",       S_RADIX,        0,      0       },
    {   NULL,   ".globl",       S_GLOBL,        0,      0       },
    {   NULL,   ".local",       S_LOCAL,        0,      0       },
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
    {   NULL,   ".list",        S_LISTING,      0,      O_LIST  },
    {   NULL,   ".nlist",       S_LISTING,      0,      O_NLIST },
    {   NULL,   ".equ",         S_EQU,          0,      O_EQU   },
    {   NULL,   ".gblequ",      S_EQU,          0,      O_GBLEQU},
    {   NULL,   ".lclequ",      S_EQU,          0,      O_LCLEQU},
/* sdas specific */
    {   NULL,   ".optsdcc",     S_OPTSDCC,      0,      0       },
/* end sdas specific */
    {   NULL,   ".byte",        S_DATA,         0,      O_1BYTE },
    {   NULL,   ".db",          S_DATA,         0,      O_1BYTE },
    {   NULL,   ".fcb",         S_DATA,         0,      O_1BYTE },
    {   NULL,   ".word",        S_DATA,         0,      O_2BYTE },
    {   NULL,   ".dw",          S_DATA,         0,      O_2BYTE },
    {   NULL,   ".fdb",         S_DATA,         0,      O_2BYTE },
/*    { NULL,   ".3byte",       S_DATA,         0,      O_3BYTE },      */
/*    { NULL,   ".triple",      S_DATA,         0,      O_3BYTE },      */
/*    { NULL,   ".4byte",       S_DATA,         0,      O_4BYTE },      */
/*    { NULL,   ".quad",        S_DATA,         0,      O_4BYTE },      */
    {   NULL,   ".blkb",        S_BLK,          0,      O_1BYTE },
    {   NULL,   ".ds",          S_BLK,          0,      O_1BYTE },
    {   NULL,   ".rmb",         S_BLK,          0,      O_1BYTE },
    {   NULL,   ".rs",          S_BLK,          0,      O_1BYTE },
    {   NULL,   ".blkw",        S_BLK,          0,      O_2BYTE },
/*    { NULL,   ".blk3",        S_BLK,          0,      O_3BYTE },      */
/*    { NULL,   ".blk4",        S_BLK,          0,      O_4BYTE },      */
    {   NULL,   ".ascii",       S_ASCIX,        0,      O_ASCII },
    {   NULL,   ".ascis",       S_ASCIX,        0,      O_ASCIS },
    {   NULL,   ".asciz",       S_ASCIX,        0,      O_ASCIZ },
    {   NULL,   ".str",         S_ASCIX,        0,      O_ASCII },
    {   NULL,   ".strs",        S_ASCIX,        0,      O_ASCIS },
    {   NULL,   ".strz",        S_ASCIX,        0,      O_ASCIZ },
    {	NULL,	".fcc",		S_ASCIX,	0,	O_ASCII	},
    {	NULL,	".define",	S_DEFINE,	0,	O_DEF	},
    {	NULL,	".undefine",	S_DEFINE,	0,	O_UNDEF	},
    {	NULL,	".even",	S_BOUNDARY,	0,	O_EVEN	},
    {	NULL,	".odd",		S_BOUNDARY,	0,	O_ODD	},
    {	NULL,	".bndry",	S_BOUNDARY,	0,	O_BNDRY	},
    {	NULL,	".msg"	,	S_MSG,		0,	0	},
    {	NULL,	".assume",	S_ERROR,	0,	O_ASSUME},
    {	NULL,	".error",	S_ERROR,	0,	O_ERROR	},

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

        /* machine */

    {   NULL,   "a",            S_A,            0,      0xFF      },

    {   NULL,   "nop",          S_NOP,          0,      0x0000    },
    {   NULL,   "ret",          S_RET,          0,      0x003A    },
    {   NULL,   "reti",         S_RETI,         0,      0x003B    },
    {   NULL,   "mov",          S_MOV,          0,      0x1700    },
    {   NULL,   "mov.io",       S_MOV,          0,      0x1700 | PDK_OPCODE_ADR_IO},
    {   NULL,   "ldt16",        S_LDT16,        0,      0x00c1    },
    {   NULL,   "stt16",        S_STT16,        0,      0x00c0    },
    {   NULL,   "idxm",         S_IDXM,         0,      0x00e0    },
    {   NULL,   "xch",          S_XCH,          0,      0x09C0    },
    {   NULL,   "pushaf",       S_PUSHAF,       0,      0x0032    },
    {   NULL,   "popaf",        S_POPAF,        0,      0x0033    },
    {   NULL,   "push",         S_PUSHAF,       0,      0x2032    },
    {   NULL,   "pop",          S_POPAF,        0,      0x2033    },
    {   NULL,   "add",          S_ADD,          0,      0x1000    },
    {   NULL,   "addc",         S_ADDC,         0,      0x0010    },
    {   NULL,   "sub",          S_SUB,          0,      0x1100    },
    {   NULL,   "subc",         S_SUBC,         0,      0x0011    },
    {   NULL,   "inc",          S_INC,          0,      0x0900    },
    {   NULL,   "dec",          S_DEC,          0,      0x0940    },
    {   NULL,   "clear",        S_CLEAR,        0,      0x0980    },
    {   NULL,   "sr",           S_SR,           0,      0x001A    },
    {   NULL,   "src",          S_SRC,          0,      0x001C    },
    {   NULL,   "sl",           S_SL,           0,      0x001B    },
    {   NULL,   "slc",          S_SLC,          0,      0x001D    },
    {   NULL,   "swap",         S_SWAP,         0,      0x001E    },
    {   NULL,   "and",          S_AND,          0,      0x1400    },
    {   NULL,   "or",           S_OR,           0,      0x1500    },
    {   NULL,   "xor",          S_XOR,          0,      0x1600    },
    {   NULL,   "xor.io",       S_XOR,          0,      0x1600 | PDK_OPCODE_ADR_IO},
    {   NULL,   "not",          S_NOT,          0,      0x0018    },
    {   NULL,   "neg",          S_NEG,          0,      0x0019    },
    {   NULL,   "set0",         S_SET0,         0,      0x0000    },
    {   NULL,   "set0.io",      S_SET0,         0,      0x0000 | PDK_OPCODE_ADR_IO},
    {   NULL,   "set1",         S_SET1,         0,      0x0000    },
    {   NULL,   "set1.io",      S_SET1,         0,      0x0000 | PDK_OPCODE_ADR_IO},
    {   NULL,   "ceqsn",        S_CEQSN,        0,      0x1200    },
    {   NULL,   "t0sn",         S_T0SN,         0,      0x0000    },
    {   NULL,   "t0sn.io",      S_T0SN,         0,      0x0000 | PDK_OPCODE_ADR_IO},
    {   NULL,   "t1sn",         S_T1SN,         0,      0x0001    },
    {   NULL,   "t1sn.io",      S_T1SN,         0,      0x0001 | PDK_OPCODE_ADR_IO},
    {   NULL,   "izsn",         S_IZSN,         0,      0x0012    },
    {   NULL,   "dzsn",         S_DZSN,         0,      0x0013    },
    {   NULL,   "call",         S_CALL,         0,      0x3800    },
    {   NULL,   "goto",         S_GOTO,         0,      0x3000    },
    {   NULL,   "pcadd",        S_PCADD,        0,      0x0017    },
    {   NULL,   "engint",       S_ENGINT,       0,      0x0038    },
    {   NULL,   "disgint",      S_DISGINT,      0,      0x0039    },
    {   NULL,   "stopsys",      S_STOPSYS,      0,      0x0036    },
    {   NULL,   "stopexe",      S_STOPEXE,      0,      0x0037    },
    {   NULL,   "reset",        S_RESET,        0,      0x0035    },
    {   NULL,   "wdreset",      S_WDRESET,      0,      0x0030    },
    {   NULL,   "cneqsn",       S_CNEQSN,       0,      0x1300    },
    {   NULL,   "mul",          S_MUL,          0,      0x003C    },
    {   NULL,   "ldsptl",       S_LDSPTL,       0,      0x0006    },
    {   NULL,   "delay",        S_DELAY,        0,      0x001F    },
    {   NULL,   "popwpc",       S_POPWPC,       0,      0x0060    },
    {   NULL,   "pmode",        S_PMODE,        0,      0x0040    },
    {   NULL,   "ldspth",       S_LDSPTH,       S_EOL,  0x0007    },
};
