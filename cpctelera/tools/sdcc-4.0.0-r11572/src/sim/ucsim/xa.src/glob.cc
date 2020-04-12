/*
 * Simulator of microcontrollers (glob.cc)
 *
 * Copyright (C) 1999,2002 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
 * Other contributors include:
 *   Karl Bongers karl@turbobit.com,
 *   Johan Knol johan.knol@iduna.nl
 */

/* This file is part of microcontroller simulator: ucsim.

UCSIM is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

UCSIM is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with UCSIM; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA. */
/*@1@*/

#include <stdio.h>

//#include "stypes.h"
#include "glob.h"

/* this needs to match enum definition in glob.h */
const char *op_mnemonic_str[] = {
"BAD_OPCODE",
"ADD",
"ADDC",
"ADDS",
"AND",
"ANL",
"ASL",
"ASR",
"BCC",
"BCS",
"BEQ",
"BG",
"BGE",
"BGT",
"BKPT",
"BL",
"BLE",
"BLT",
"BMI",
"BNE",
"BNV",
"BOV",
"BPL",
"BR",
"CALL",
"CJNE",
"CLR",
"CMP",
"CPL",
"DA",
"DIV_w",
"DIV_d",
"DIVU_b",
"DIVU_w",
"DIVU_d",
"DJNZ",
"FCALL",
"FJMP",
"JB",
"JBC",
"JMP",
"JNB",
"JNZ",
"JZ",
"LEA",
"LSR",
"MOV",
"MOVC",
"MOVS",
"MOVX",
"MUL_w",
"MULU_b",
"MULU_w",
"NEG",
"NOP",
"NORM",
"OR",
"ORL",
"POP",
"POPU",
"PUSH",
"PUSHU",
"RESET",
"RET",
"RETI",
"RL",
"RLC",
"RR",
"RRC",
"SETB",
"SEXT",
"SUB",
"SUBB",
"TRAP",
"XCH",
"XOR",
};

/* this is junk, but we need to keep it until main ucSim code
   is cleaned of dis_entry[] references. */
struct dis_entry glob_disass_xa[]= {
  { 0x0000, 0x00ff, ' ', 1, "nop" },
  { 0x0000, 0x00, 0, 0, NULL}
};

/* plan: keep this list in same order as in User Guide(pg 106)
   until all op-codes are defined.  Figure out how to make simulation
   lookup fast later. */
struct xa_dis_entry disass_xa[]= {
 {0,0x0100,0xf700,' ',2,ADD, REG_REG         },  // ADD Rd, Rs                 0 0 0 0 S 0 0 1  d d d d s s s s
 {0,0x0200,0xf708,' ',2,ADD, REG_IREG        },  // ADD Rd, [Rs]               0 0 0 0 S 0 1 0  d d d d 0 s s s
 {0,0x0208,0xf708,' ',2,ADD, IREG_REG        },  // ADD [Rd], Rs               0 0 0 0 S 0 1 0  s s s s 1 d d d
 {0,0x0400,0xf708,' ',3,ADD, REG_IREGOFF8    },  // ADD Rd, [Rs+offset8]       0 0 0 0 S 1 0 0  d d d d 0 s s s
 {0,0x0408,0xf708,' ',3,ADD, IREGOFF8_REG    },  // ADD [Rd+offset8], Rs       0 0 0 0 S 1 0 0  s s s s 1 d d d
 {0,0x0500,0xf708,' ',4,ADD, REG_IREGOFF16   },  // ADD Rd, [Rs+offset16]      0 0 0 0 S 1 0 1  d d d d 0 s s s
 {0,0x0508,0xf708,' ',4,ADD, IREGOFF16_REG   },  // ADD [Rd+offset16], Rs      0 0 0 0 S 1 0 1  s s s s 1 d d d
 {0,0x0300,0xf708,' ',2,ADD, REG_IREGINC     },  // ADD Rd, [Rs+]              0 0 0 0 S 0 1 1  d d d d 0 s s s
 {0,0x0308,0xf708,' ',2,ADD, IREGINC_REG     },  // ADD [Rd+], Rs              0 0 0 0 S 0 1 1  s s s s 1 d d d
 {0,0x0608,0xf708,' ',3,ADD, DIRECT_REG      },  // ADD direct, Rs             0 0 0 0 S 1 1 0  s s s s 1 x x x
 {0,0x0600,0xf708,' ',3,ADD, REG_DIRECT      },  // ADD Rd, direct             0 0 0 0 S 1 1 0  d d d d 0 x x x
 {0,0x9100,0xff0f,' ',3,ADD, REG_DATA8       },  // ADD Rd, #data8             1 0 0 1 0 0 0 1  d d d d 0 0 0 0
 {0,0x9900,0xff0f,' ',4,ADD, REG_DATA16      },  // ADD Rd, #data16            1 0 0 1 1 0 0 1  d d d d 0 0 0 0
 {0,0x9200,0xff8f,' ',3,ADD, IREG_DATA8      },  // ADD [Rd], #data8           1 0 0 1 0 0 1 0  0 d d d 0 0 0 0
 {0,0x9a00,0xff8f,' ',4,ADD, IREG_DATA16     },  // ADD [Rd], #data16          1 0 0 1 1 0 1 0  0 d d d 0 0 0 0
 {0,0x9300,0xff8f,' ',3,ADD, IREGINC_DATA8   },  // ADD [Rd+], #data8          1 0 0 1 0 0 1 1  0 d d d 0 0 0 0
 {0,0x9b00,0xff8f,' ',4,ADD, IREGINC_DATA16  },  // ADD [Rd+], #data16         1 0 0 1 1 0 1 1  0 d d d 0 0 0 0
 {0,0x9400,0xff8f,' ',4,ADD, IREGOFF8_DATA8  },  // ADD [Rd+offset8], #data8   1 0 0 1 0 1 0 0  0 d d d 0 0 0 0
 {0,0x9c00,0xff8f,' ',5,ADD, IREGOFF8_DATA16 },  // ADD [Rd+offset8], #data16  1 0 0 1 1 1 0 0  0 d d d 0 0 0 0
 {0,0x9500,0xff8f,' ',5,ADD, IREGOFF16_DATA8 },  // ADD [Rd+offset16], #data8  1 0 0 1 0 1 0 1  0 d d d 0 0 0 0
 {0,0x9d00,0xff8f,' ',6,ADD, IREGOFF16_DATA16},  // ADD [Rd+offset16], #data16 1 0 0 1 1 1 0 1  0 d d d 0 0 0 0
 {0,0x9600,0xff8f,' ',4,ADD, DIRECT_DATA8    },  // ADD direct, #data8         1 0 0 1 0 1 1 0  0 b b b 0 0 0 0
 {0,0x9e00,0xff8f,' ',5,ADD, DIRECT_DATA16   },  // ADD direct, #data16        1 0 0 1 1 1 1 0  0 b b b 0 0 0 0

 {0,0x1100,0xf700,' ',2,ADDC,REG_REG         },  //ADDC Rd, Rs                 0 0 0 1 S 0 0 1  d d d d s s s s
 {0,0x1200,0xf708,' ',2,ADDC,REG_IREG        },  //ADDC Rd, [Rs]               0 0 0 1 S 0 1 0  d d d d 0 s s s
 {0,0x1208,0xf708,' ',2,ADDC,IREG_REG        },  //ADDC [Rd], Rs               0 0 0 1 S 0 1 0  s s s s 1 d d d
 {0,0x1400,0xf708,' ',3,ADDC,REG_IREGOFF8    },  //ADDC Rd, [Rs+offset8]       0 0 0 1 S 1 0 0  d d d d 0 s s s
 {0,0x1408,0xf708,' ',3,ADDC,IREGOFF8_REG    },  //ADDC [Rd+offset8], Rs       0 0 0 1 S 1 0 0  s s s s 1 d d d
 {0,0x1500,0xf708,' ',4,ADDC,REG_IREGOFF16   },  //ADDC Rd, [Rs+offset16]      0 0 0 1 S 1 0 1  d d d d 0 s s s
 {0,0x1508,0xf708,' ',4,ADDC,IREGOFF16_REG   },  //ADDC [Rd+offset16], Rs      0 0 0 1 S 1 0 1  s s s s 1 d d d
 {0,0x1300,0xf708,' ',2,ADDC,REG_IREGINC     },  //ADDC Rd, [Rs+]              0 0 0 1 S 0 1 1  d d d d 0 s s s
 {0,0x1308,0xf708,' ',2,ADDC,IREGINC_REG     },  //ADDC [Rd+], Rs              0 0 0 1 S 0 1 1  s s s s 1 d d d
 {0,0x1608,0xf708,' ',3,ADDC,DIRECT_REG      },  //ADDC direct, Rs             0 0 0 1 S 1 1 0  s s s s 1 x x x
 {0,0x1600,0xf708,' ',3,ADDC,REG_DIRECT      },  //ADDC Rd, direct             0 0 0 1 S 1 1 0  d d d d 0 x x x
 {0,0x9101,0xff0f,' ',3,ADDC,REG_DATA8       },  //ADDC Rd, #data8             1 0 0 1 0 0 0 1  d d d d 0 0 0 1
 {0,0x9901,0xff0f,' ',4,ADDC,REG_DATA16      },  //ADDC Rd, #data16            1 0 0 1 1 0 0 1  d d d d 0 0 0 1
 {0,0x9201,0xff8f,' ',3,ADDC,IREG_DATA8      },  //ADDC [Rd], #data8           1 0 0 1 0 0 1 0  0 d d d 0 0 0 1
 {0,0x9a01,0xff8f,' ',4,ADDC,IREG_DATA16     },  //ADDC [Rd], #data16          1 0 0 1 1 0 1 0  0 d d d 0 0 0 1
 {0,0x9301,0xff8f,' ',3,ADDC,IREGINC_DATA8   },  //ADDC [Rd+], #data8          1 0 0 1 0 0 1 1  0 d d d 0 0 0 1
 {0,0x9b01,0xff8f,' ',4,ADDC,IREGINC_DATA16  },  //ADDC [Rd+], #data16         1 0 0 1 1 0 1 1  0 d d d 0 0 0 1
 {0,0x9401,0xff8f,' ',4,ADDC,IREGOFF8_DATA8  },  //ADDC [Rd+offset8], #data8   1 0 0 1 0 1 0 0  0 d d d 0 0 0 1
 {0,0x9c01,0xff8f,' ',5,ADDC,IREGOFF8_DATA16 },  //ADDC [Rd+offset8], #data16  1 0 0 1 1 1 0 0  0 d d d 0 0 0 1
 {0,0x9501,0xff8f,' ',5,ADDC,IREGOFF16_DATA8 },  //ADDC [Rd+offset16], #data8  1 0 0 1 0 1 0 1  0 d d d 0 0 0 1
 {0,0x9d01,0xff8f,' ',6,ADDC,IREGOFF16_DATA16},  //ADDC [Rd+offset16], #data16 1 0 0 1 1 1 0 1  0 d d d 0 0 0 1
 {0,0x9601,0xff8f,' ',4,ADDC,DIRECT_DATA8    },  //ADDC direct, #data8         1 0 0 1 0 1 1 0  0 b b b 0 0 0 1
 {0,0x9e01,0xff8f,' ',5,ADDC,DIRECT_DATA16   },  //ADDC direct, #data16        1 0 0 1 1 1 1 0  0 b b b 0 0 0 1

 {0,0x1408,0xf780,' ',2,ADDS, REG_DATA4      }, // ADDS Rd, #data4             1 0 1 0 S 0 0 1  d d d d #data4
 {0,0x1408,0xf780,' ',2,ADDS, IREG_DATA4     }, // ADDS [Rd], #data4           1 0 1 0 S 0 1 0  0 d d d #data4
 {0,0x1408,0xf780,' ',2,ADDS, IREGINC_DATA4  }, // ADDS [Rd+], #data4          1 0 1 0 S 0 1 1  0 d d d #data4
 {0,0x1408,0xf780,' ',3,ADDS, IREGOFF8_DATA4 }, // ADDS [Rd+offset8], #data4   1 0 1 0 S 1 0 0  0 d d d #data4
 {0,0x1408,0xf780,' ',4,ADDS, IREGOFF16_DATA4}, // ADDS [Rd+offset16], #data4  1 0 1 0 S 1 0 1  0 d d d #data4
 {0,0x1408,0xf780,' ',3,ADDS, DIRECT_DATA4   }, // ADDS direct, #data4         1 0 1 0 S 1 1 0  0 x x x #data4

 {0,0x5100,0xf700,' ',2, AND,REG_REG         },  // AND Rd, Rs                 0 1 0 1 S 0 0 1  d d d d s s s s 
 {0,0x5200,0xf708,' ',2, AND,REG_IREG        },  // AND Rd, [Rs]               0 1 0 1 S 0 1 0  d d d d 0 s s s 
 {0,0x5208,0xf708,' ',2, AND,IREG_REG        },  // AND [Rd], Rs               0 1 0 1 S 0 1 0  s s s s 1 d d d 
 {0,0x5400,0xf708,' ',3, AND,REG_IREGOFF8    },  // AND Rd, [Rs+offset8]       0 1 0 1 S 1 0 0  d d d d 0 s s s 
 {0,0x5408,0xf708,' ',3, AND,IREGOFF8_REG    },  // AND [Rd+offset8], Rs       0 1 0 1 S 1 0 0  s s s s 1 d d d 
 {0,0x5500,0xf708,' ',4, AND,REG_IREGOFF16   },  // AND Rd, [Rs+offset16]      0 1 0 1 S 1 0 1  d d d d 0 s s s 
 {0,0x5508,0xf708,' ',4, AND,IREGOFF16_REG   },  // AND [Rd+offset16], Rs      0 1 0 1 S 1 0 1  s s s s 1 d d d 
 {0,0x5300,0xf708,' ',2, AND,REG_IREGINC     },  // AND Rd, [Rs+]              0 1 0 1 S 0 1 1  d d d d 0 s s s 
 {0,0x5308,0xf708,' ',2, AND,IREGINC_REG     },  // AND [Rd+], Rs              0 1 0 1 S 0 1 1  s s s s 1 d d d 
 {0,0x5608,0xf708,' ',3, AND,DIRECT_REG      },  // AND direct, Rs             0 1 0 1 S 1 1 0  s s s s 1 x x x 
 {0,0x5600,0xf708,' ',3, AND,REG_DIRECT      },  // AND Rd, direct             0 1 0 1 S 1 1 0  d d d d 0 x x x 
 {0,0x9105,0xff0f,' ',3, AND,REG_DATA8       },  // AND Rd, #data8             1 0 0 1 0 0 0 1  d d d d 0 1 0 1 
 {0,0x9905,0xff0f,' ',4, AND,REG_DATA16      },  // AND Rd, #data16            1 0 0 1 1 0 0 1  d d d d 0 1 0 1 
 {0,0x9205,0xff8f,' ',3, AND,IREG_DATA8      },  // AND [Rd], #data8           1 0 0 1 0 0 1 0  0 d d d 0 1 0 1 
 {0,0x9a05,0xff8f,' ',4, AND,IREG_DATA16     },  // AND [Rd], #data16          1 0 0 1 1 0 1 0  0 d d d 0 1 0 1 
 {0,0x9305,0xff8f,' ',3, AND,IREGINC_DATA8   },  // AND [Rd+], #data8          1 0 0 1 0 0 1 1  0 d d d 0 1 0 1 
 {0,0x9b05,0xff8f,' ',4, AND,IREGINC_DATA16  },  // AND [Rd+], #data16         1 0 0 1 1 0 1 1  0 d d d 0 1 0 1 
 {0,0x9405,0xff8f,' ',4, AND,IREGOFF8_DATA8  },  // AND [Rd+offset8], #data8   1 0 0 1 0 1 0 0  0 d d d 0 1 0 1 
 {0,0x9c05,0xff8f,' ',5, AND,IREGOFF8_DATA16 },  // AND [Rd+offset8], #data16  1 0 0 1 1 1 0 0  0 d d d 0 1 0 1 
 {0,0x9505,0xff8f,' ',5, AND,IREGOFF16_DATA8 },  // AND [Rd+offset16], #data8  1 0 0 1 0 1 0 1  0 d d d 0 1 0 1 
 {0,0x9d05,0xff8f,' ',6, AND,IREGOFF16_DATA16},  // AND [Rd+offset16], #data16 1 0 0 1 1 1 0 1  0 d d d 0 1 0 1 
 {0,0x9605,0xff8f,' ',4, AND,DIRECT_DATA8    },  // AND direct, #data8         1 0 0 1 0 1 1 0  0 b b b 0 1 0 1 
 {0,0x9e05,0xff8f,' ',5, AND,DIRECT_DATA16   },  // AND direct, #data16        1 0 0 1 0 1 1 0  1 b b b 0 1 0 1 

 {0,0x0840,0xfffc,' ',3,ANL, CY_BIT          }, //  ANL C, bit                 0 0 0 0 1 0 0 0  0 1 0 0 0 0 b b lsbbit
 {0,0x0850,0xfffc,' ',3,ANL, CY_NOTBIT       }, //  ANL C, /bit                0 0 0 0 1 0 0 0  0 1 0 1 0 0 b b lsbbit
 {0,0xc150,0xf300,' ',2,ASL, REG_REG         }, //  ASL Rd, Rs                 1 1 0 0 S S 0 1  d d d d s s s s
 {0,0xdd00,0xff00,' ',2,ASL, REG_DATA5       }, //  ASL Rd, #data5 (dword)     1 1 0 1 1 1 0 1  d d d #data5
 {0,0xd100,0xf700,' ',2,ASL, REG_DATA4       }, //  ASL Rd, #data4             1 1 0 1 S S 0 1  d d d d #data4
 {0,0xc250,0xf300,' ',2,ASR, REG_REG         }, //  ASR Rd, Rs                 1 1 0 0 S S 1 0  d d d d s s s s
 {0,0xde00,0xff00,' ',2,ASR, REG_DATA5       }, //  ASR Rd, #data5 (dword)     1 1 0 1 1 1 1 0  d d d #data5
 {0,0xd200,0xf700,' ',2,ASR, REG_DATA4       }, //  ASR Rd, #data4             1 1 0 1 S S 1 0  d d d d #data4
 {1,0xf000,0xff00,' ',2,BCC, REL8            }, //  BCC rel8                   1 1 1 1 0 0 0 0  rel8
 {1,0xf100,0xff00,' ',2,BCS, REL8            }, //  BCS rel8                   1 1 1 1 0 0 0 1  rel8
 {1,0xf300,0xff00,' ',2,BEQ, REL8            }, //  BEQ rel8                   1 1 1 1 0 0 1 1  rel8
 {1,0xf800,0xff00,' ',2,BG, REL8             }, //  BG rel8                    1 1 1 1 1 0 0 0  rel8
 {1,0xfa00,0xff00,' ',2,BGE, REL8            }, //  BGE rel8                   1 1 1 1 1 0 1 0  rel8
 {1,0xfc00,0xff00,' ',2,BGT, REL8            }, //  BGT rel8                   1 1 1 1 1 1 0 0  rel8

 {1,0xff00,0xff00,' ',1,BKPT, NO_OPERANDS    }, //  BKPT                       1 1 1 1 1 1 1 1

 {1,0xf900,0xff00,' ',2,BL, REL8             }, //  BL rel8                    1 1 1 1 1 0 0 1  rel8
 {1,0xfd00,0xff00,' ',2,BLE, REL8            }, //  BLE rel8                   1 1 1 1 1 1 0 1  rel8
 {1,0xfb00,0xff00,' ',2,BLT, REL8            }, //  BLT rel8                   1 1 1 1 1 0 1 1  rel8
 {1,0xf700,0xff00,' ',2,BMI, REL8            }, //  BMI rel8                   1 1 1 1 0 1 1 1  rel8
 {1,0xf200,0xff00,' ',2,BNE, REL8            }, //  BNE rel8                   1 1 1 1 0 0 1 0  rel8
 {1,0xf400,0xff00,' ',2,BNV, REL8            }, //  BNV rel8                   1 1 1 1 0 1 0 0  rel8
 {1,0xf500,0xff00,' ',2,BOV, REL8            }, //  BOV rel8                   1 1 1 1 0 1 0 1  rel8
 {1,0xf600,0xff00,' ',2,BPL, REL8            }, //  BPL rel8                   1 1 1 1 0 1 1 0  rel8
 {1,0xfe00,0xff00,' ',2,BR, REL8             }, //  BR rel8                    1 1 1 1 1 1 1 0  rel8

 {1,0xc500,0xff00,'a',3,CALL, REL16          }, //  CALL rel16                 1 1 0 0 0 1 0 1  rel16
 {0,0xc600,0xfff8,'a',2,CALL, IREG           }, //  CALL [Rs]                  1 1 0 0 0 1 1 0  0 0 0 0 0 s s s

 {0,0xe200,0xf708,' ',4,CJNE, REG_DIRECT_REL8}, //  CJNE Rd, direct, rel8      1 1 1 0 S 0 1 0  d d d d 0 x x x
 {0,0xe300,0xff0f,' ',4,CJNE, REG_DATA8_REL8},  //  CJNE Rd, data8, rel8       1 1 1 0 0 0 1 1  d d d d 0 0 0 0
 {0,0xeb00,0xff0f,' ',5,CJNE, REG_DATA16_REL8}, //  CJNE Rd, data16, rel8      1 1 1 0 1 0 1 1  d d d d 0 0 0 0
 {0,0xe308,0xff8f,' ',4,CJNE, IREG_DATA8_REL8}, //  CJNE [Rd], data8, rel8     1 1 1 0 0 0 1 1  0 d d d 1 0 0 0
 {0,0xeb08,0xff8f,' ',5,CJNE, IREG_DATA16_REL8},//  CJNE [Rd], data16, rel8    1 1 1 0 1 0 1 1  0 d d d 1 0 0 0

 {0,0x0800,0xfffc,' ',3,CLR, BIT_ALONE       },  // CLR bit                    0 0 0 0 1 0 0 0  0 0 0 0 0 0 b b 
 {0,0x4100,0xf700,' ',2,CMP, REG_REG         },  // CMP Rd, Rs                 0 1 0 0 S 0 0 1  d d d d s s s s
 {0,0x4200,0xf708,' ',2,CMP, REG_IREG        },  // CMP Rd, [Rs]               0 1 0 0 S 0 1 0  d d d d 0 s s s
 {0,0x4208,0xf708,' ',2,CMP, IREG_REG        },  // CMP [Rd], Rs               0 1 0 0 S 0 1 0  s s s s 1 d d d
 {0,0x4400,0xf708,' ',3,CMP, REG_IREGOFF8    },  // CMP Rd, [Rs+offset8]       0 1 0 0 S 1 0 0  d d d d 0 s s s
 {0,0x4408,0xf708,' ',3,CMP, IREGOFF8_REG    },  // CMP [Rd+offset8], Rs       0 1 0 0 S 1 0 0  s s s s 1 d d d
 {0,0x4500,0xf708,' ',4,CMP, REG_IREGOFF16   },  // CMP Rd, [Rs+offset16]      0 1 0 0 S 1 0 1  d d d d 0 s s s
 {0,0x4508,0xf708,' ',4,CMP, IREGOFF16_REG   },  // CMP [Rd+offset16], Rs      0 1 0 0 S 1 0 1  s s s s 1 d d d
 {0,0x4300,0xf708,' ',2,CMP, REG_IREGINC     },  // CMP Rd, [Rs+]              0 1 0 0 S 0 1 1  d d d d 0 s s s
 {0,0x4308,0xf708,' ',2,CMP, IREGINC_REG     },  // CMP [Rd+], Rs              0 1 0 0 S 0 1 1  s s s s 1 d d d
 {0,0x4608,0xf708,' ',3,CMP, DIRECT_REG      },  // CMP direct, Rs             0 1 0 0 S 1 1 0  s s s s 1 x x x
 {0,0x4600,0xf708,' ',3,CMP, REG_DIRECT      },  // CMP Rd, direct             0 1 0 0 S 1 1 0  d d d d 0 x x x
 {0,0x9104,0xff0f,' ',3,CMP, REG_DATA8       },  // CMP Rd, #data8             1 0 0 1 0 0 0 1  d d d d 0 1 0 0
 {0,0x9904,0xff0f,' ',4,CMP, REG_DATA16      },  // CMP Rd, #data16            1 0 0 1 1 0 0 1  d d d d 0 1 0 0
 {0,0x9204,0xff8f,' ',3,CMP, IREG_DATA8      },  // CMP [Rd], #data8           1 0 0 1 0 0 1 0  0 d d d 0 1 0 0
 {0,0x9a04,0xff8f,' ',4,CMP, IREG_DATA16     },  // CMP [Rd], #data16          1 0 0 1 1 0 1 0  0 d d d 0 1 0 0
 {0,0x9304,0xff8f,' ',3,CMP, IREGINC_DATA8   },  // CMP [Rd+], #data8          1 0 0 1 0 0 1 1  0 d d d 0 1 0 0
 {0,0x9b04,0xff8f,' ',4,CMP, IREGINC_DATA16  },  // CMP [Rd+], #data16         1 0 0 1 1 0 1 1  0 d d d 0 1 0 0
 {0,0x9404,0xff8f,' ',4,CMP, IREGOFF8_DATA8  },  // CMP [Rd+offset8], #data8   1 0 0 1 0 1 0 0  0 d d d 0 1 0 0
 {0,0x9c04,0xff8f,' ',5,CMP, IREGOFF8_DATA16 },  // CMP [Rd+offset8], #data16  1 0 0 1 1 1 0 0  0 d d d 0 1 0 0
 {0,0x9504,0xff8f,' ',5,CMP, IREGOFF16_DATA8 },  // CMP [Rd+offset16], #data8  1 0 0 1 0 1 0 1  0 d d d 0 1 0 0
 {0,0x9d04,0xff8f,' ',6,CMP, IREGOFF16_DATA16},  // CMP [Rd+offset16], #data16 1 0 0 1 1 1 0 1  0 d d d 0 1 0 0
 {0,0x9604,0xff8f,' ',4,CMP, DIRECT_DATA8    },  // CMP direct, #data8         1 0 0 1 0 1 1 0  0 b b b 0 1 0 0
 {0,0x9e04,0xff8f,' ',5,CMP, DIRECT_DATA16   },  // CMP direct, #data16        1 0 0 1 0 1 1 0  0 b b b 0 1 0 0
 {0,0x900c,0xf70f,' ',2,CPL, REG             }, //  CPL Rd                     1 0 0 1 S 0 0 0  d d d d 1 0 1 0
 {0,0x9008,0xff0f,' ',2,DA, REG              }, //  DA Rd                      1 0 0 1 0 0 0 0  d d d d 1 0 0 0
 {0,0xe708,0xff00,' ',2,DIV_w, REG_REG       }, //  DIV.w Rd, Rs               1 1 1 0 0 1 1 1  d d d d s s s s
 {0,0xe80b,0xff0f,' ',3,DIV_w, REG_DATA8     }, //  DIV.w Rd, #data8           1 1 1 0 1 0 0 0  d d d d 1 0 1 1
 {0,0xef00,0xff10,' ',2,DIV_d, REG_REG       }, //  DIV.d Rd, Rs               1 1 1 0 1 1 1 1  d d d 0 s s s s
 {0,0xe909,0xff1f,' ',4,DIV_d, REG_DATA16    }, //  DIV.d Rd, #data16          1 1 1 0 1 0 0 1  d d d 0 1 0 0 1
 {0,0xe101,0xff00,' ',3,DIVU_b, REG_REG      }, //  DIVU.b Rd, Rs              1 1 1 0 0 0 0 1  d d d d s s s s
 {0,0xe801,0xff0f,' ',3,DIVU_b, REG_DATA8    }, //  DIVU.b Rd, #data8          1 1 1 0 1 0 0 0  d d d d 0 0 0 1
 {0,0xe500,0xff00,' ',2,DIVU_w, REG_REG      }, //  DIVU.w Rd, Rs              1 1 1 0 0 1 0 1  d d d d s s s s
 {0,0xe803,0xff0f,' ',3,DIVU_w, REG_DATA8    }, //  DIVU.w Rd, #data8          1 1 1 0 1 0 0 0  d d d d 0 0 1 1
 {0,0xed00,0xff10,' ',2,DIVU_d, REG_REG      }, //  DIVU.d Rd, Rs              1 1 1 0 1 1 0 1  d d d 0 s s s s
 {0,0xe901,0xff1f,' ',4,DIVU_d, REG_DATA16   }, //  DIVU.d Rd, #data16         1 1 1 0 1 0 0 1  d d d 0 0 0 0 1

 {0,0x8708,0xf70f,' ',3,DJNZ,   REG_REL8     }, //  DJNZ Rd, rel8              1 0 0 0 S 1 1 1  d d d d 1 0 0 0
 {0,0xe208,0xf7f8,' ',4,DJNZ,   DIRECT_REL8  }, //  DJNZ direct, rel8          1 1 1 0 S 0 1 0  0 0 0 0 1 x x x

 {1,0xc400,0xff00,' ',4,FCALL,  ADDR24       }, //  FCALL addr24               1 1 0 0 0 1 0 0  
 {1,0xd400,0xff00,' ',4,FJMP,   ADDR24       }, //  FJMP addr24                1 1 0 1 0 1 0 0

 {0,0x9780,0xfffc,' ',4, JB,    BIT_REL8     }, //  JB bit,rel8                1 0 0 1 0 1 1 1  1 0 0 0 0 0 b b
 {0,0x97c0,0xfffc,' ',4, JBC,   BIT_REL8     }, //  JBC bit,rel8               1 0 0 1 0 1 1 1  1 1 0 0 0 0 b b
 {1,0xd500,0xff00,' ',3, JMP,   REL16        }, //  JMP rel16                  1 1 0 1 0 1 0 1  rel16
 {0,0xd670,0xfff8,' ',2, JMP,   IREG         }, //  JMP [Rs]                   1 1 0 1 0 1 1 0  0 1 1 1 0 s s s
 {0,0xd646,0xffff,' ',2, JMP,   A_PLUSDPTR   }, //  JMP [A+dptr]               1 1 0 1 0 1 1 0  0 1 0 0 0 1 1 0
 {0,0xd660,0xfff8,' ',2, JMP,   IIREG        }, //  JMP [[Rs+]]                1 1 0 1 0 1 1 0  0 1 1 0 0 s s s

 {0,0x97a0,0xfffc,' ',4, JNB,   BIT_REL8     }, //  JNB bit,rel8               1 0 0 1 0 1 1 1  1 0 1 0 0 0 b b
 {1,0xee00,0xff00,' ',2, JNZ,   REL8         }, //  JNZ rel8                   1 1 1 0 1 1 1 0  rel8
 {1,0xec00,0xff00,' ',2, JZ,    REL8         }, //  JZ rel8                    1 1 1 0 1 1 0 0  rel8
 {0,0x4000,0xff88,' ',3, LEA,   REG_REGOFF8  }, //  LEA Rd,Rs+offset8          0 1 0 0 0 0 0 0  0 d d d 0 s s s
 {0,0x4800,0xff88,' ',3, LEA,   REG_REGOFF16 }, //  LEA Rd,Rs+offset16         0 1 0 0 0 0 0 0  0 d d d 0 s s s
 /* LSR(3?)  */

 {0,0x8100,0xf700,' ',2,MOV, REG_REG         },  // MOV Rd, Rs                 1 0 0 0 S 0 0 1  d d d d s s s s
 {0,0x8200,0xf708,' ',2,MOV, REG_IREG        },  // MOV Rd, [Rs]               1 0 0 0 S 0 1 0  d d d d 0 s s s
 {0,0x8208,0xf708,' ',2,MOV, IREG_REG        },  // MOV [Rd], Rs               1 0 0 0 S 0 1 0  s s s s 1 d d d
 {0,0x8400,0xf708,' ',3,MOV, REG_IREGOFF8    },  // MOV Rd, [Rs+offset8]       1 0 0 0 S 1 0 0  d d d d 0 s s s
 {0,0x8408,0xf708,' ',3,MOV, IREGOFF8_REG    },  // MOV [Rd+offset8], Rs       1 0 0 0 S 1 0 0  s s s s 1 d d d
 {0,0x8500,0xf708,' ',4,MOV, REG_IREGOFF16   },  // MOV Rd, [Rs+offset16]      1 0 0 0 S 1 0 1  d d d d 0 s s s
 {0,0x8508,0xf708,' ',4,MOV, IREGOFF16_REG   },  // MOV [Rd+offset16], Rs      1 0 0 0 S 1 0 1  s s s s 1 d d d
 {0,0x8300,0xf708,' ',2,MOV, REG_IREGINC     },  // MOV Rd, [Rs+]              1 0 0 0 S 0 1 1  d d d d 0 s s s
 {0,0x8308,0xf708,' ',2,MOV, IREGINC_REG     },  // MOV [Rd+], Rs              1 0 0 0 S 0 1 1  s s s s 1 d d d
 {0,0x8608,0xf708,' ',3,MOV, DIRECT_REG      },  // MOV direct, Rs             1 0 0 0 S 1 1 0  s s s s 1 x x x
 {0,0x8600,0xf708,' ',3,MOV, REG_DIRECT      },  // MOV Rd, direct             1 0 0 0 S 1 1 0  d d d d 0 x x x
 {0,0x9108,0xff0f,' ',3,MOV, REG_DATA8       },  // MOV Rd, #data8             1 0 0 1 0 0 0 1  d d d d 1 0 0 0
 {0,0x9908,0xff0f,' ',4,MOV, REG_DATA16      },  // MOV Rd, #data16            1 0 0 1 1 0 0 1  d d d d 1 0 0 0
 {0,0x9208,0xff8f,' ',3,MOV, IREG_DATA8      },  // MOV [Rd], #data8           1 0 0 1 0 0 1 0  0 d d d 1 0 0 0
 {0,0x9a08,0xff8f,' ',4,MOV, IREG_DATA16     },  // MOV [Rd], #data16          1 0 0 1 1 0 1 0  0 d d d 1 0 0 0
 {0,0x9308,0xff8f,' ',3,MOV, IREGINC_DATA8   },  // MOV [Rd+], #data8          1 0 0 1 0 0 1 1  0 d d d 1 0 0 0
 {0,0x9b08,0xff8f,' ',4,MOV, IREGINC_DATA16  },  // MOV [Rd+], #data16         1 0 0 1 1 0 1 1  0 d d d 1 0 0 0
 {0,0x9408,0xff8f,' ',4,MOV, IREGOFF8_DATA8  },  // MOV [Rd+offset8], #data8   1 0 0 1 0 1 0 0  0 d d d 1 0 0 0
 {0,0x9c08,0xff8f,' ',5,MOV, IREGOFF8_DATA16 },  // MOV [Rd+offset8], #data16  1 0 0 1 1 1 0 0  0 d d d 1 0 0 0
 {0,0x9508,0xff8f,' ',5,MOV, IREGOFF16_DATA8 },  // MOV [Rd+offset16], #data8  1 0 0 1 0 1 0 1  0 d d d 1 0 0 0
 {0,0x9d08,0xff8f,' ',6,MOV, IREGOFF16_DATA16},  // MOV [Rd+offset16], #data16 1 0 0 1 1 1 0 1  0 d d d 1 0 0 0
 {0,0x9608,0xff8f,' ',4,MOV, DIRECT_DATA8    },  // MOV direct, #data8         1 0 0 1 0 1 1 0  0 b b b 1 0 0 0
 {0,0x9e08,0xff8f,' ',5,MOV, DIRECT_DATA16   },  // MOV direct, #data16        1 0 0 1 0 1 1 0  0 b b b 1 0 0 0
 {0,0x9700,0xf788,' ',4,MOV, DIRECT_DIRECT   },  // MOV direct, direct         1 0 0 1 S 1 1 1  0 d d d 0 d d d
 {0,0x900f,0xff0f,' ',2,MOV, REG_USP         },  // MOV Rd, USP                1 0 0 1 0 0 0 0  d d d d 1 1 1 1
 {0,0x980f,0xff0f,' ',2,MOV, USP_REG         },  // MOV USP, RS                1 0 0 1 0 0 0 0  s s s s 1 1 1 1
 {0,0x0820,0xfffc,' ',3,MOV, CY_BIT          },  // MOV C, bit                 0 0 0 0 1 0 0 0  0 0 1 0 0 0 b b
 {0,0x0830,0xfffc,' ',3,MOV, BIT_CY          },  // MOV bit, C                 0 0 0 0 1 0 0 0  0 0 1 1 0 0 b b
 {0,0x8000,0xf308,' ',2,MOVC, REG_IREGINC    },  // MOVC Rd,[Rs+]              1 0 0 0 S 0 0 0  d d d d 0 s s s
 {0,0x904e,0xffff,' ',2,MOVC, A_APLUSDPTR    },  // MOVC A,[A+DPTR]            1 0 0 1 0 0 0 0  0 1 0 0 1 1 1 0
 {0,0x904c,0xffff,' ',2,MOVC, A_APLUSPC      },  // MOVC A,[A+PC]              1 0 0 1 0 0 0 0  0 1 0 0 1 1 0 0
   /* MOVS(6), MOVX(2), MUL.x(6) */

 {0,0x900b,0xf70f,' ',2,NEG, REG             }, //  NEG Rd                     1 0 0 1 S 0 0 0  d d d d 1 0 1 1
 {1,0x0000,0xff00,' ',1,NOP, NO_OPERANDS     }, //  NOP                        0 0 0 0 0 0 0 0
 {0,0xc300,0xff00,' ',2,NORM, REG_REG        }, //  NORM Rd,Rs                 1 1 0 0 S S 1 1  d d d d s s s s
 {0,0x6100,0xf700,' ',2, OR, REG_REG         },  //  OR Rd, Rs                 0 1 1 0 S 0 0 1  d d d d s s s s
 {0,0x6200,0xf708,' ',2, OR, REG_IREG        },  //  OR Rd, [Rs]               0 1 1 0 S 0 1 0  d d d d 0 s s s
 {0,0x6208,0xf708,' ',2, OR, IREG_REG        },  //  OR [Rd], Rs               0 1 1 0 S 0 1 0  s s s s 1 d d d
 {0,0x6400,0xf708,' ',3, OR, REG_IREGOFF8    },  //  OR Rd, [Rs+offset8]       0 1 1 0 S 1 0 0  d d d d 0 s s s
 {0,0x6408,0xf708,' ',3, OR, IREGOFF8_REG    },  //  OR [Rd+offset8], Rs       0 1 1 0 S 1 0 0  s s s s 1 d d d
 {0,0x6500,0xf708,' ',4, OR, REG_IREGOFF16   },  //  OR Rd, [Rs+offset16]      0 1 1 0 S 1 0 1  d d d d 0 s s s
 {0,0x6508,0xf708,' ',4, OR, IREGOFF16_REG   },  //  OR [Rd+offset16], Rs      0 1 1 0 S 1 0 1  s s s s 1 d d d
 {0,0x6300,0xf708,' ',2, OR, REG_IREGINC     },  //  OR Rd, [Rs+]              0 1 1 0 S 0 1 1  d d d d 0 s s s
 {0,0x6308,0xf708,' ',2, OR, IREGINC_REG     },  //  OR [Rd+], Rs              0 1 1 0 S 0 1 1  s s s s 1 d d d
 {0,0x6608,0xf708,' ',3, OR, DIRECT_REG      },  //  OR direct, Rs             0 1 1 0 S 1 1 0  s s s s 1 x x x
 {0,0x6600,0xf708,' ',3, OR, REG_DIRECT      },  //  OR Rd, direct             0 1 1 0 S 1 1 0  d d d d 0 x x x
 {0,0x9106,0xff0f,' ',3, OR, REG_DATA8       },  //  OR Rd, #data8             1 0 0 1 0 0 0 1  d d d d 0 1 1 0
 {0,0x9906,0xff0f,' ',4, OR, REG_DATA16      },  //  OR Rd, #data16            1 0 0 1 1 0 0 1  d d d d 0 1 1 0
 {0,0x9206,0xff8f,' ',3, OR, IREG_DATA8      },  //  OR [Rd], #data8           1 0 0 1 0 0 1 0  0 d d d 0 1 1 0
 {0,0x9a06,0xff8f,' ',4, OR, IREG_DATA16     },  //  OR [Rd], #data16          1 0 0 1 1 0 1 0  0 d d d 0 1 1 0
 {0,0x9306,0xff8f,' ',3, OR, IREGINC_DATA8   },  //  OR [Rd+], #data8          1 0 0 1 0 0 1 1  0 d d d 0 1 1 0
 {0,0x9b06,0xff8f,' ',4, OR, IREGINC_DATA16  },  //  OR [Rd+], #data16         1 0 0 1 1 0 1 1  0 d d d 0 1 1 0
 {0,0x9406,0xff8f,' ',4, OR, IREGOFF8_DATA8  },  //  OR [Rd+offset8], #data8   1 0 0 1 0 1 0 0  0 d d d 0 1 1 0
 {0,0x9c06,0xff8f,' ',5, OR, IREGOFF8_DATA16 },  //  OR [Rd+offset8], #data16  1 0 0 1 1 1 0 0  0 d d d 0 1 1 0
 {0,0x9506,0xff8f,' ',5, OR, IREGOFF16_DATA8 },  //  OR [Rd+offset16], #data8  1 0 0 1 0 1 0 1  0 d d d 0 1 1 0
 {0,0x9d06,0xff8f,' ',6, OR, IREGOFF16_DATA16},  //  OR [Rd+offset16], #data16 1 0 0 1 1 1 0 1  0 d d d 0 1 1 0
 {0,0x9606,0xff8f,' ',4, OR, DIRECT_DATA8    },  //  OR direct, #data8         1 0 0 1 0 1 1 0  0 b b b 0 1 1 0
 {0,0x9e06,0xff8f,' ',5, OR, DIRECT_DATA16   },  //  OR direct, #data16        1 0 0 1 0 1 1 0  0 b b b 0 1 1 0
 {0,0x0860,0xfffc,' ',3, ORL, CY_BIT         },  //  ORL C, bit                0 0 0 0 1 0 0 0  0 1 1 0 0 0 b b
 {0,0x0870,0xfffc,' ',3, ORL, CY_NOTBIT       },  //  ORL C, /bit               0 0 0 0 1 0 0 0  0 1 1 1 0 0 b b
 {0,0x8710,0xf7f8,' ',3, POP, DIRECT         },  //  POP direct                1 0 0 0 S 1 1 1  0 0 0 1 0 d d d
 {1,0x2700,0xb700,' ',2, POP, RLIST          },  //  POP Rlist                 0 H 1 0 S 1 1 1  rlist
 {0,0x8700,0xf7f8,' ',3, POPU, DIRECT        },  //  POPU direct               1 0 0 0 S 1 1 1  0 0 0 0 0 d d d
 {1,0x3700,0xb700,' ',2, POPU, RLIST         },  //  POPU Rlist                0 H 1 1 S 1 1 1  rlist
 {0,0x8730,0xf7f8,' ',3, PUSH, DIRECT        },  //  PUSH direct               1 0 0 0 S 1 1 1  0 0 1 1 0 d d d
 {1,0x0700,0xb700,' ',2, PUSH, RLIST         },  //  PUSH Rlist                0 H 0 0 S 1 1 1  rlist
 {0,0x8720,0xf7f8,' ',3, PUSHU, DIRECT       },  //  PUSHU direct              1 0 0 0 S 1 1 1  0 0 1 0 0 d d d
 {1,0x1700,0xb700,' ',2, PUSHU, RLIST        },  //  PUSHU Rlist               0 H 0 1 S 1 1 1  rlist

 {0,0xd610,0xffff,' ',2, RESET, NO_OPERANDS  },  //  RESET                     1 1 0 1 0 1 1 0  0 0 0 1 0 0 0 0
 {0,0xd680,0xffff,' ',2, RET, NO_OPERANDS    },  //  RET                       1 1 0 1 0 1 1 0  1 0 0 0 0 0 0 0
 {0,0xd690,0xffff,' ',2, RETI, NO_OPERANDS   },  //  RETI                      1 1 0 1 0 1 1 0  1 0 0 1 0 0 0 0 
   /* RL, RLC, RR, RRC */
 {0,0x0810,0xfffc,' ',3, SETB, BIT_ALONE     },  //  SETB bit                  0 0 0 0 1 0 0 0  0 0 0 1 0 0 b b 
 {0,0x9009,0xf70f,' ',2, SEXT, REG           },  //  SEXT Rd                   1 0 0 1 S 0 0 0  d d d d 1 0 0 1
 {0,0x2100,0xf700,' ',2,SUB, REG_REG         },  // SUB Rd, Rs                 0 0 1 0 S 0 0 1  d d d d s s s s
 {0,0x2200,0xf708,' ',2,SUB, REG_IREG        },  // SUB Rd, [Rs]               0 0 1 0 S 0 1 0  d d d d 0 s s s
 {0,0x2208,0xf708,' ',2,SUB, IREG_REG        },  // SUB [Rd], Rs               0 0 1 0 S 0 1 0  s s s s 1 d d d
 {0,0x2400,0xf708,' ',3,SUB, REG_IREGOFF8    },  // SUB Rd, [Rs+offset8]       0 0 1 0 S 1 0 0  d d d d 0 s s s
 {0,0x2408,0xf708,' ',3,SUB, IREGOFF8_REG    },  // SUB [Rd+offset8], Rs       0 0 1 0 S 1 0 0  s s s s 1 d d d
 {0,0x2500,0xf708,' ',4,SUB, REG_IREGOFF16   },  // SUB Rd, [Rs+offset16]      0 0 1 0 S 1 0 1  d d d d 0 s s s
 {0,0x2508,0xf708,' ',4,SUB, IREGOFF16_REG   },  // SUB [Rd+offset16], Rs      0 0 1 0 S 1 0 1  s s s s 1 d d d
 {0,0x2300,0xf708,' ',2,SUB, REG_IREGINC     },  // SUB Rd, [Rs+]              0 0 1 0 S 0 1 1  d d d d 0 s s s
 {0,0x2308,0xf708,' ',2,SUB, IREGINC_REG     },  // SUB [Rd+], Rs              0 0 1 0 S 0 1 1  s s s s 1 d d d
 {0,0x2608,0xf708,' ',3,SUB, DIRECT_REG      },  // SUB direct, Rs             0 0 1 0 S 1 1 0  s s s s 1 x x x
 {0,0x2600,0xf708,' ',3,SUB, REG_DIRECT      },  // SUB Rd, direct             0 0 1 0 S 1 1 0  d d d d 0 x x x
 {0,0x9102,0xff0f,' ',3,SUB, REG_DATA8       },  // SUB Rd, #data8             1 0 0 1 0 0 0 1  d d d d 0 0 1 0
 {0,0x9902,0xff0f,' ',4,SUB, REG_DATA16      },  // SUB Rd, #data16            1 0 0 1 1 0 0 1  d d d d 0 0 1 0
 {0,0x9202,0xff8f,' ',3,SUB, IREG_DATA8      },  // SUB [Rd], #data8           1 0 0 1 0 0 1 0  0 d d d 0 0 1 0
 {0,0x9a02,0xff8f,' ',4,SUB, IREG_DATA16     },  // SUB [Rd], #data16          1 0 0 1 1 0 1 0  0 d d d 0 0 1 0
 {0,0x9302,0xff8f,' ',3,SUB, IREGINC_DATA8   },  // SUB [Rd+], #data8          1 0 0 1 0 0 1 1  0 d d d 0 0 1 0
 {0,0x9b02,0xff8f,' ',4,SUB, IREGINC_DATA16  },  // SUB [Rd+], #data16         1 0 0 1 1 0 1 1  0 d d d 0 0 1 0
 {0,0x9402,0xff8f,' ',4,SUB, IREGOFF8_DATA8  },  // SUB [Rd+offset8], #data8   1 0 0 1 0 1 0 0  0 d d d 0 0 1 0
 {0,0x9c02,0xff8f,' ',5,SUB, IREGOFF8_DATA16 },  // SUB [Rd+offset8], #data16  1 0 0 1 1 1 0 0  0 d d d 0 0 1 0
 {0,0x9502,0xff8f,' ',5,SUB, IREGOFF16_DATA8 },  // SUB [Rd+offset16], #data8  1 0 0 1 0 1 0 1  0 d d d 0 0 1 0
 {0,0x9d02,0xff8f,' ',6,SUB, IREGOFF16_DATA16},  // SUB [Rd+offset16], #data16 1 0 0 1 1 1 0 1  0 d d d 0 0 1 0
 {0,0x9602,0xff8f,' ',4,SUB, DIRECT_DATA8    },  // SUB direct, #data8         1 0 0 1 0 1 1 0  0 b b b 0 0 1 0
 {0,0x9e02,0xff8f,' ',5,SUB, DIRECT_DATA16   },  // SUB direct, #data16        1 0 0 1 0 1 1 0  0 b b b 0 0 1 0

 {0,0x3100,0xf700,' ',2,SUBB,REG_REG         },  //SUBB Rd, Rs                 0 0 1 1 S 0 0 1  d d d d s s s s
 {0,0x3200,0xf708,' ',2,SUBB,REG_IREG        },  //SUBB Rd, [Rs]               0 0 1 1 S 0 1 0  d d d d 0 s s s
 {0,0x3208,0xf708,' ',2,SUBB,IREG_REG        },  //SUBB [Rd], Rs               0 0 1 1 S 0 1 0  s s s s 1 d d d
 {0,0x3400,0xf708,' ',3,SUBB,REG_IREGOFF8    },  //SUBB Rd, [Rs+offset8]       0 0 1 1 S 1 0 0  d d d d 0 s s s
 {0,0x3408,0xf708,' ',3,SUBB,IREGOFF8_REG    },  //SUBB [Rd+offset8], Rs       0 0 1 1 S 1 0 0  s s s s 1 d d d
 {0,0x3500,0xf708,' ',4,SUBB,REG_IREGOFF16   },  //SUBB Rd, [Rs+offset16]      0 0 1 1 S 1 0 1  d d d d 0 s s s
 {0,0x3508,0xf708,' ',4,SUBB,IREGOFF16_REG   },  //SUBB [Rd+offset16], Rs      0 0 1 1 S 1 0 1  s s s s 1 d d d
 {0,0x3300,0xf708,' ',2,SUBB,REG_IREGINC     },  //SUBB Rd, [Rs+]              0 0 1 1 S 0 1 1  d d d d 0 s s s
 {0,0x3308,0xf708,' ',2,SUBB,IREGINC_REG     },  //SUBB [Rd+], Rs              0 0 1 1 S 0 1 1  s s s s 1 d d d
 {0,0x3608,0xf708,' ',3,SUBB,DIRECT_REG      },  //SUBB direct, Rs             0 0 1 1 S 1 1 0  s s s s 1 x x x
 {0,0x3600,0xf708,' ',3,SUBB,REG_DIRECT      },  //SUBB Rd, direct             0 0 1 1 S 1 1 0  d d d d 0 x x x
 {0,0x9103,0xff0f,' ',3,SUBB,REG_DATA8       },  //SUBB Rd, #data8             1 0 0 1 0 0 0 1  d d d d 0 0 1 1
 {0,0x9903,0xff0f,' ',4,SUBB,REG_DATA16      },  //SUBB Rd, #data16            1 0 0 1 1 0 0 1  d d d d 0 0 1 1
 {0,0x9203,0xff8f,' ',3,SUBB,IREG_DATA8      },  //SUBB [Rd], #data8           1 0 0 1 0 0 1 0  0 d d d 0 0 1 1
 {0,0x9a03,0xff8f,' ',4,SUBB,IREG_DATA16     },  //SUBB [Rd], #data16          1 0 0 1 1 0 1 0  0 d d d 0 0 1 1
 {0,0x9303,0xff8f,' ',3,SUBB,IREGINC_DATA8   },  //SUBB [Rd+], #data8          1 0 0 1 0 0 1 1  0 d d d 0 0 1 1
 {0,0x9b03,0xff8f,' ',4,SUBB,IREGINC_DATA16  },  //SUBB [Rd+], #data16         1 0 0 1 1 0 1 1  0 d d d 0 0 1 1
 {0,0x9403,0xff8f,' ',4,SUBB,IREGOFF8_DATA8  },  //SUBB [Rd+offset8], #data8   1 0 0 1 0 1 0 0  0 d d d 0 0 1 1
 {0,0x9c03,0xff8f,' ',5,SUBB,IREGOFF8_DATA16 },  //SUBB [Rd+offset8], #data16  1 0 0 1 1 1 0 0  0 d d d 0 0 1 1
 {0,0x9503,0xff8f,' ',5,SUBB,IREGOFF16_DATA8 },  //SUBB [Rd+offset16], #data8  1 0 0 1 0 1 0 1  0 d d d 0 0 1 1
 {0,0x9d03,0xff8f,' ',6,SUBB,IREGOFF16_DATA16},  //SUBB [Rd+offset16], #data16 1 0 0 1 1 1 0 1  0 d d d 0 0 1 1
 {0,0x9603,0xff8f,' ',4,SUBB,DIRECT_DATA8    },  //SUBB direct, #data8         1 0 0 1 0 1 1 0  0 b b b 0 0 1 1
 {0,0x9e03,0xff8f,' ',5,SUBB,DIRECT_DATA16   },  //SUBB direct, #data16        1 0 0 1 0 1 1 0  0 b b b 0 0 1 1
 {0,0xd630,0xfff0,' ',2,TRAP,DATA4           },  //TRAP #data4                 1 1 0 1 0 1 1 0  0 0 1 1 #data4
   /* XCH(3) */

 {0,0x7100,0xf700,' ',2,XOR, REG_REG         },  // XOR Rd, Rs                 0 1 1 1 S 0 0 1  d d d d s s s s
 {0,0x7200,0xf708,' ',2,XOR, REG_IREG        },  // XOR Rd, [Rs]               0 1 1 1 S 0 1 0  d d d d 0 s s s
 {0,0x7208,0xf708,' ',2,XOR, IREG_REG        },  // XOR [Rd], Rs               0 1 1 1 S 0 1 0  s s s s 1 d d d
 {0,0x7400,0xf708,' ',3,XOR, REG_IREGOFF8    },  // XOR Rd, [Rs+offset8]       0 1 1 1 S 1 0 0  d d d d 0 s s s
 {0,0x7408,0xf708,' ',3,XOR, IREGOFF8_REG    },  // XOR [Rd+offset8], Rs       0 1 1 1 S 1 0 0  s s s s 1 d d d
 {0,0x7500,0xf708,' ',4,XOR, REG_IREGOFF16   },  // XOR Rd, [Rs+offset16]      0 1 1 1 S 1 0 1  d d d d 0 s s s
 {0,0x7508,0xf708,' ',4,XOR, IREGOFF16_REG   },  // XOR [Rd+offset16], Rs      0 1 1 1 S 1 0 1  s s s s 1 d d d
 {0,0x7300,0xf708,' ',2,XOR, REG_IREGINC     },  // XOR Rd, [Rs+]              0 1 1 1 S 0 1 1  d d d d 0 s s s
 {0,0x7308,0xf708,' ',2,XOR, IREGINC_REG     },  // XOR [Rd+], Rs              0 1 1 1 S 0 1 1  s s s s 1 d d d
 {0,0x7608,0xf708,' ',3,XOR, DIRECT_REG      },  // XOR direct, Rs             0 1 1 1 S 1 1 0  s s s s 1 x x x
 {0,0x7600,0xf708,' ',3,XOR, REG_DIRECT      },  // XOR Rd, direct             0 1 1 1 S 1 1 0  d d d d 0 x x x
 {0,0x9107,0xff0f,' ',3,XOR, REG_DATA8       },  // XOR Rd, #data8             1 0 0 1 0 0 0 1  d d d d 0 1 1 1
 {0,0x9907,0xff0f,' ',4,XOR, REG_DATA16      },  // XOR Rd, #data16            1 0 0 1 1 0 0 1  d d d d 0 1 1 1
 {0,0x9207,0xff8f,' ',3,XOR, IREG_DATA8      },  // XOR [Rd], #data8           1 0 0 1 0 0 1 0  0 d d d 0 1 1 1
 {0,0x9a07,0xff8f,' ',4,XOR, IREG_DATA16     },  // XOR [Rd], #data16          1 0 0 1 1 0 1 0  0 d d d 0 1 1 1
 {0,0x9307,0xff8f,' ',3,XOR, IREGINC_DATA8   },  // XOR [Rd+], #data8          1 0 0 1 0 0 1 1  0 d d d 0 1 1 1
 {0,0x9b07,0xff8f,' ',4,XOR, IREGINC_DATA16  },  // XOR [Rd+], #data16         1 0 0 1 1 0 1 1  0 d d d 0 1 1 1
 {0,0x9407,0xff8f,' ',4,XOR, IREGOFF8_DATA8  },  // XOR [Rd+offset8], #data8   1 0 0 1 0 1 0 0  0 d d d 0 1 1 1
 {0,0x9c07,0xff8f,' ',5,XOR, IREGOFF8_DATA16 },  // XOR [Rd+offset8], #data16  1 0 0 1 1 1 0 0  0 d d d 0 1 1 1
 {0,0x9507,0xff8f,' ',5,XOR, IREGOFF16_DATA8 },  // XOR [Rd+offset16], #data8  1 0 0 1 0 1 0 1  0 d d d 0 1 1 1
 {0,0x9d07,0xff8f,' ',6,XOR, IREGOFF16_DATA16},  // XOR [Rd+offset16], #data16 1 0 0 1 1 1 0 1  0 d d d 0 1 1 1
 {0,0x9607,0xff8f,' ',4,XOR, DIRECT_DATA8    },  // XOR direct, #data8         1 0 0 1 0 1 1 0  0 b b b 0 1 1 1
 {0,0x9e07,0xff8f,' ',5,XOR, DIRECT_DATA16   },  // XOR direct, #data16        1 0 0 1 0 1 1 0  0 b b b 0 1 1 1

 {0,0x0000,0x0000,  0,1,BAD_OPCODE, REG_REG}
};


/* End of xa.src/glob.cc */
