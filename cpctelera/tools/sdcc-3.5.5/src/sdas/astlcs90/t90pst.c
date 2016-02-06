/* t90pst.c */

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

/*
 * TLCS90 Port: R. Keuchel
 */

#include "asxxxx.h"
#include "t90.h"

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

    {   NULL,   ".t90",         S_CPU,          0,      X_T90   },

    // NOTE: the opcode is not always correct here, changed in code..
    {   NULL,   "push",         S_PUSH,         0,      0x50    },
    {   NULL,   "pop",          S_PUSH,         0,      0x58    },

    {   NULL,   "call",         S_CALL,         0,      0x1C    },
    {   NULL,   "ret",          S_RET,          0,      0x1E    },
    {   NULL,   "reti",         S_INH1,         0,      0x1F    },

    {   NULL,   "swi",          S_INH1,         0,      0xFF    },

    {   NULL,   "jp",           S_JP,           0,      0x1A    },
    {   NULL,   "jr",           S_JR,           0,      0xC0    },
    {   NULL,   "djnz",         S_DJNZ,         0,      0x18    },

    {   NULL,   "ex",           S_EX,           0,      0x50    },

    {   NULL,   "nop",          S_INH1,         0,      0x00    },
    {   NULL,   "halt",         S_INH1,         0,      0x01    },
    {   NULL,   "di",           S_INH1,         0,      0x02    },
    {   NULL,   "ei",           S_INH1,         0,      0x03    },

    {   NULL,   "exx",          S_INH1,         0,      0x0A    },

    {   NULL,   "daa",          S_INH1,         0,      0x0B    },
    {   NULL,   "scf",          S_INH1,         0,      0x0D    },
    {   NULL,   "ccf",          S_INH1,         0,      0x0E    },

    {   NULL,   "cpl",          S_INH1,         0,      0x10    },
    {   NULL,   "neg",          S_INH1,         0,      0x11    },

    {   NULL,   "inc",          S_INC,          0,      0x80    },
    {   NULL,   "dec",          S_DEC,          0,      0x88    },

    {   NULL,   "bit",          S_BIT,          0,      0xA8    },
    {   NULL,   "res",          S_RES,          0,      0xB0    },
    {   NULL,   "set",          S_SET,          0,      0xB8    },


    {   NULL,   "rld",          S_RLD,          0,      0x10    },
    {   NULL,   "rrd",          S_RRD,          0,      0x11    },

    {   NULL,   "rlca",         S_INH1,         0,      0xA0    },
    {   NULL,   "rrca",         S_INH1,         0,      0xA1    },
    {   NULL,   "rla",          S_INH1,         0,      0xA2    },
    {   NULL,   "rra",          S_INH1,         0,      0xA3    },

    {   NULL,   "rlc",          S_RLC,           0,      0xA0    }, // a
    {   NULL,   "rrc",          S_RRC,           0,      0xA1    }, // a, mem
    {   NULL,   "rl",           S_RL,            0,      0xA2    },
    {   NULL,   "rr",           S_RR,            0,      0xA3    }, // a,c
    {   NULL,   "sla",          S_SLA,           0,      0xA4    }, // a and mm
    {   NULL,   "sra",          S_SRA,           0,      0xA5    }, // a and mm
    {   NULL,   "sll",          S_SLL,           0,      0xA6    }, // a and mm
    {   NULL,   "srl",          S_SRL,           0,      0xA7    }, // a,b,c and mm, a7/e7/fe/f8

    {   NULL,   "ld",           S_LD,           0,      0x20    },

    {   NULL,   "callr",        S_CALLR,        0,      0x1D    }, 

    {   NULL,   "mul",          S_MUL,          0,      0x12    }, 
    {   NULL,   "div",          S_DIV,          0,      0x13    }, 

    {   NULL,   "add",          S_ADD,          0,      0x60    },
    {   NULL,   "adc",          S_ADC,          0,      0x61    },
    {   NULL,   "sub",          S_SUB,          0,      0x62    },
    {   NULL,   "sbc",          S_SBC,          0,      0x63    },
    {   NULL,   "and",          S_AND,          0,      0x64    },
    {   NULL,   "xor",          S_XOR,          0,      0x65    },
    {   NULL,   "or",           S_OR,           0,      0x66    },
    {   NULL,   "cp",           S_CP,           0,      0x67    },
    {   NULL,   "ldar",         S_LDAR,         0,      0x17    },

    {   NULL,   "ldi",          S_LDI,          0,      0x58    }, // fe 58
    {   NULL,   "ldir",         S_LDIR,         0,      0x59    }, // fe 59
    {   NULL,   "ldd",          S_LDD,          0,      0x5A    }, // fe 5a
    {   NULL,   "lddr",         S_LDDR,         0,      0x5B    }, // fe 5b
    {   NULL,   "cpi",          S_CPI,          0,      0x5C    }, // fe 5c
    {   NULL,   "cpir",         S_CPIR,         0,      0x5D    }, // fe 5d
    {   NULL,   "cpd",          S_CPD,          0,      0x5D    }, // fe 5e
    {   NULL,   "cpdr",         S_CPDR,         0,      0x5D    }, // fe 5f

    {   NULL,   "ldw",          S_LDW,          0,      0x20    },

    {   NULL,   "incw",        S_INCW,          0,      0x97    },
    {   NULL,   "incx",        S_INCX,          0,      0x07    },
    {   NULL,   "decw",        S_DECW,          0,      0x9F    },
    {   NULL,   "decx",        S_DECX,        S_EOL,    0x0F    } // NOTE: must be last (S_EOL)!

};
