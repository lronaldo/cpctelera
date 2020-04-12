/* m08pst.c */

/*
 *  Copyright (C) 1993-2009  Alan R. Baldwin
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

#include "asxxxx.h"
#include "m6808.h"

/*
 * Mnemonic Structure
 */
struct  mne     mne[] = {

        /* machine */

    {   NULL,   ".setdp",       S_SDP,          0,      0       },

        /* system */

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
    {   NULL,   ".fcc",         S_ASCIX,        0,      O_ASCII },
    {   NULL,   ".even",        S_BOUNDARY,     0,      O_EVEN  },
    {   NULL,   ".odd",         S_BOUNDARY,     0,      O_ODD   },
    {   NULL,   ".bndry",       S_BOUNDARY,     0,      O_BNDRY },
/* sdas specific */
    {   NULL,   ".optsdcc",     S_OPTSDCC,      0,      0       },
/* end sdas specific */
//    { NULL,   ".assume",      S_ERROR,        0,      0       },
//    { NULL,   ".error",       S_ERROR,        0,      1       },
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

    {   NULL,   ".hc08",        S_CPU,          0,      X_HC08  },
    {   NULL,   ".hcs08",       S_CPU,          0,      X_HCS08 },
    {   NULL,   ".cs08",        S_CPU,          0,      X_HCS08 },
    {   NULL,   ".6805",        S_CPU,          0,      X_6805  },
    {   NULL,   ".hc05",        S_CPU,          0,      X_HC05  },

        /* 68HC08 */

    {   NULL,   "neg",          S_TYP1,         0,      0x30    },
    {   NULL,   "com",          S_TYP1,         0,      0x33    },
    {   NULL,   "lsr",          S_TYP1,         0,      0x34    },
    {   NULL,   "ror",          S_TYP1,         0,      0x36    },
    {   NULL,   "asr",          S_TYP1,         0,      0x37    },
    {   NULL,   "asl",          S_TYP1,         0,      0x38    },
    {   NULL,   "lsl",          S_TYP1,         0,      0x38    },
    {   NULL,   "rol",          S_TYP1,         0,      0x39    },
    {   NULL,   "dec",          S_TYP1,         0,      0x3A    },
    {   NULL,   "inc",          S_TYP1,         0,      0x3C    },
    {   NULL,   "tst",          S_TYP1,         0,      0x3D    },
    {   NULL,   "clr",          S_TYP1,         0,      0x3F    },

    {   NULL,   "sub",          S_TYP2,         0,      0xA0    },
    {   NULL,   "cmp",          S_TYP2,         0,      0xA1    },
    {   NULL,   "sbc",          S_TYP2,         0,      0xA2    },
    {   NULL,   "cpx",          S_TYP2,         0,      0xA3    },
    {   NULL,   "and",          S_TYP2,         0,      0xA4    },
    {   NULL,   "bit",          S_TYP2,         0,      0xA5    },
    {   NULL,   "lda",          S_TYP2,         0,      0xA6    },
    {   NULL,   "sta",          S_TYP2,         0,      0xA7    },
    {   NULL,   "eor",          S_TYP2,         0,      0xA8    },
    {   NULL,   "adc",          S_TYP2,         0,      0xA9    },
    {   NULL,   "ora",          S_TYP2,         0,      0xAA    },
    {   NULL,   "add",          S_TYP2,         0,      0xAB    },
    {   NULL,   "jmp",          S_TYP2,         0,      0xAC    },
    {   NULL,   "jsr",          S_TYP2,         0,      0xAD    },
    {   NULL,   "ldx",          S_TYP2,         0,      0xAE    },
    {   NULL,   "stx",          S_TYP2,         0,      0xAF    },

    {   NULL,   "bset",         S_TYP3,         0,      0x10    },
    {   NULL,   "bclr",         S_TYP3,         0,      0x11    },

    {   NULL,   "brset",        S_TYP4,         0,      0x00    },
    {   NULL,   "brclr",        S_TYP4,         0,      0x01    },

    {   NULL,   "ais",          S_TYPAI,        0,      0xA7    },
    {   NULL,   "aix",          S_TYPAI,        0,      0xAF    },

    {   NULL,   "sthx",         S_TYPHX,        0,      0x25    },
    {   NULL,   "ldhx",         S_TYPHX,        0,      0x45    },
    {   NULL,   "cphx",         S_TYPHX,        0,      0x65    },

    {   NULL,   "cbeq",         S_CBEQ,         0,      0x31    },
    {   NULL,   "cbeqa",        S_CQAX,         0,      0x41    },
    {   NULL,   "cbeqx",        S_CQAX,         0,      0x51    },

    {   NULL,   "dbnz",         S_DBNZ,         0,      0x3B    },
    {   NULL,   "dbnza",        S_DZAX,         0,      0x4B    },
    {   NULL,   "dbnzx",        S_DZAX,         0,      0x5B    },

    {   NULL,   "mov",          S_MOV,          0,      0x4E    },

    {   NULL,   "bra",          S_BRA,          0,      0x20    },
    {   NULL,   "brn",          S_BRA,          0,      0x21    },
    {   NULL,   "bhi",          S_BRA,          0,      0x22    },
    {   NULL,   "bls",          S_BRA,          0,      0x23    },
    {   NULL,   "bcc",          S_BRA,          0,      0x24    },
    {   NULL,   "bhs",          S_BRA,          0,      0x24    },
    {   NULL,   "bcs",          S_BRA,          0,      0x25    },
    {   NULL,   "blo",          S_BRA,          0,      0x25    },
    {   NULL,   "bne",          S_BRA,          0,      0x26    },
    {   NULL,   "beq",          S_BRA,          0,      0x27    },
    {   NULL,   "bhcc",         S_BRA,          0,      0x28    },
    {   NULL,   "bhcs",         S_BRA,          0,      0x29    },
    {   NULL,   "bpl",          S_BRA,          0,      0x2A    },
    {   NULL,   "bmi",          S_BRA,          0,      0x2B    },
    {   NULL,   "bmc",          S_BRA,          0,      0x2C    },
    {   NULL,   "bms",          S_BRA,          0,      0x2D    },
    {   NULL,   "bil",          S_BRA,          0,      0x2E    },
    {   NULL,   "bih",          S_BRA,          0,      0x2F    },
    {   NULL,   "bge",          S_BRA8,         0,      0x90    },
    {   NULL,   "blt",          S_BRA8,         0,      0x91    },
    {   NULL,   "bgt",          S_BRA8,         0,      0x92    },
    {   NULL,   "ble",          S_BRA8,         0,      0x93    },
    {   NULL,   "bsr",          S_BRA,          0,      0xAD    },

    {   NULL,   "nega",         S_INH,          0,      0x40    },
    {   NULL,   "mul",          S_INH,          0,      0x42    },
    {   NULL,   "coma",         S_INH,          0,      0x43    },
    {   NULL,   "lsra",         S_INH,          0,      0x44    },
    {   NULL,   "rora",         S_INH,          0,      0x46    },
    {   NULL,   "asra",         S_INH,          0,      0x47    },
    {   NULL,   "asla",         S_INH,          0,      0x48    },
    {   NULL,   "lsla",         S_INH,          0,      0x48    },
    {   NULL,   "rola",         S_INH,          0,      0x49    },
    {   NULL,   "deca",         S_INH,          0,      0x4A    },
    {   NULL,   "inca",         S_INH,          0,      0x4C    },
    {   NULL,   "tsta",         S_INH,          0,      0x4D    },
    {   NULL,   "clra",         S_INH,          0,      0x4F    },

    {   NULL,   "negx",         S_INH,          0,      0x50    },
    {   NULL,   "div",          S_INH8,         0,      0x52    },
    {   NULL,   "comx",         S_INH,          0,      0x53    },
    {   NULL,   "lsrx",         S_INH,          0,      0x54    },
    {   NULL,   "rorx",         S_INH,          0,      0x56    },
    {   NULL,   "asrx",         S_INH,          0,      0x57    },
    {   NULL,   "aslx",         S_INH,          0,      0x58    },
    {   NULL,   "lslx",         S_INH,          0,      0x58    },
    {   NULL,   "rolx",         S_INH,          0,      0x59    },
    {   NULL,   "decx",         S_INH,          0,      0x5A    },
    {   NULL,   "incx",         S_INH,          0,      0x5C    },
    {   NULL,   "tstx",         S_INH,          0,      0x5D    },
    {   NULL,   "clrx",         S_INH,          0,      0x5F    },

    {   NULL,   "nsa",          S_INH8,         0,      0x62    },

    {   NULL,   "daa",          S_INH8,         0,      0x72    },

    {   NULL,   "rti",          S_INH,          0,      0x80    },
    {   NULL,   "rts",          S_INH,          0,      0x81    },
    {   NULL,   "bgnd",         S_INH8S,        0,      0x82    },
    {   NULL,   "swi",          S_INH,          0,      0x83    },
    {   NULL,   "tap",          S_INH8,         0,      0x84    },
    {   NULL,   "tpa",          S_INH8,         0,      0x85    },
    {   NULL,   "pula",         S_INH8,         0,      0x86    },
    {   NULL,   "psha",         S_INH8,         0,      0x87    },
    {   NULL,   "pulx",         S_INH8,         0,      0x88    },
    {   NULL,   "pshx",         S_INH8,         0,      0x89    },
    {   NULL,   "pulh",         S_INH8,         0,      0x8A    },
    {   NULL,   "pshh",         S_INH8,         0,      0x8B    },
    {   NULL,   "clrh",         S_INH8,         0,      0x8C    },
    {   NULL,   "stop",         S_INH,          0,      0x8E    },
    {   NULL,   "wait",         S_INH,          0,      0x8F    },

    {   NULL,   "txs",          S_INH8,         0,      0x94    },
    {   NULL,   "tsx",          S_INH8,         0,      0x95    },
    {   NULL,   "tax",          S_INH,          0,      0x97    },
    {   NULL,   "clc",          S_INH,          0,      0x98    },
    {   NULL,   "sec",          S_INH,          0,      0x99    },
    {   NULL,   "cli",          S_INH,          0,      0x9A    },
    {   NULL,   "sei",          S_INH,          0,      0x9B    },
    {   NULL,   "rsp",          S_INH,          0,      0x9C    },
    {   NULL,   "nop",          S_INH,          0,      0x9D    },
    {   NULL,   "txa",          S_INH,          S_EOL,  0x9F    }
};
