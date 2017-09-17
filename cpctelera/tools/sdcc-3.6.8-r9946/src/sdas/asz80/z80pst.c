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
/*    { NULL,   ".3byte",       S_DATA,         0,      O_3BYTE },      */
/*    { NULL,   ".triple",      S_DATA,         0,      O_3BYTE },      */
/*    { NULL,   ".4byte",       S_DATA,         0,      O_4BYTE },      */
/*    { NULL,   ".quad",        S_DATA,         0,      O_4BYTE },      */
    {   NULL,   ".df",          S_FLOAT,        0,      0       },
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
    {   NULL,   "tstio",        X_TSTIO,        S_EOL,  0x74    }
};
