/*
 * Simulator of microcontrollers (stypes.h)
 *
 * Copyright (C) 1997 Drotos Daniel
 * 
 * To contact author send email to dr.dkdb@gmail.com
 *
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

#ifndef STYPES_HEADER
#define STYPES_HEADER

#include "ddconfig.h"

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

//typedef int8_t TYPE_BYTE;
//typedef uint8_t TYPE_UBYTE;
//typedef int16_t TYPE_WORD;
//typedef uint16_t TYPE_UWORD;
//typedef int32_t TYPE_DWORD;
//typedef uint32_t TYPE_UDWORD;

typedef unsigned char	uchar;
typedef unsigned int	uint;
typedef unsigned long	ulong;

typedef   signed TYPE_BYTE  i8_t;
typedef unsigned TYPE_BYTE  u8_t;
typedef   signed TYPE_WORD  i16_t;
typedef unsigned TYPE_WORD  u16_t;
typedef   signed TYPE_DWORD i32_t;
typedef unsigned TYPE_DWORD u32_t;
typedef   signed TYPE_QWORD i64_t;
typedef unsigned TYPE_QWORD u64_t;

typedef /*i64_t*/u32_t		t_addr;		/* 32 bit max */
typedef u32_t		t_mem;		/* 32 bit max */
typedef i32_t		t_smem;		/* signed 32 bit memory */

#define SPECA SPEC_QWORD
#define SPECM SPEC_DWORD

#define AI(addr)   ((int)(addr))
#define AU(addr)   ((unsigned int)(addr))
#define AI8(addr)  (AI((addr)&0xff))
#define AU8(addr)  (AU((addr)&0xff))
#define AI16(addr) (AI((addr)&0xffff))
#define AU16(addr) (AU((addr)&0xffff))
#define AI32(addr) (AI((addr)&0xffffffff))
#define AU32(addr) (AU((addr)&0xffffffff))

#define MI(v)      ((int)(v))
#define MU(v)      ((unsigned int)(v))
#define MI8(v)     (MI((v)&0xff))
#define MU8(v)     (MU((v)&0xff))
#define MI32(v)    (MI((v)&0xffffffff))
#define MU32(v)    (MU((v)&0xffffffff))

enum {
  max_mem_size= 0x40000000		/* 1 GB */
};

struct id_element
{
  int id;
  const char *id_string;
};

enum error_type {
  err_unknown  = 0x01,
  err_error    = 0x02,
  err_warning  = 0x04
};

// table of disassembled instructions
struct dis_entry
{
  /*uint64_t*/long long code, mask; // max 8 byte of code
  char  branch;
  i8_t length;
  const char *mnemonic;
  bool is_call;
  uchar ticks;
  void *info;
};

// table entry of SFR and BIT names
struct name_entry
{
  int		cpu_type;
  t_addr	addr;
  const char	*name;
};

enum cpu_type {
  CPU_NONE      = 0,
  
  CPU_51	= 0x0001,
  CPU_31	= 0x0002,
  CPU_52	= 0x0004,
  CPU_32	= 0x0008,
  CPU_51R	= 0x0010,
  CPU_89C51R	= 0x0020,
  CPU_251	= 0x0040,
  CPU_DS320 	= 0x0080,
  CPU_DS390	= 0x0100,
  CPU_DS390F	= 0x0200,
  CPU_C521	= 0x0400,
  CPU_517	= 0x0800,
  CPU_F380	= 0x1000,
  CPU_XC88X	= 0x2000,
  
  CPU_ALL_51	= (CPU_51|CPU_31),
  CPU_ALL_52	= (CPU_52|CPU_32|CPU_51R|CPU_89C51R|CPU_251|
		   CPU_DS320|CPU_DS390|CPU_DS390F|
		   CPU_C521|CPU_517|CPU_XC88X|
		   CPU_F380),
  CPU_ALL_DS3X0 = (CPU_DS320|CPU_DS390|CPU_DS390F),
 
  CPU_AVR	= 0x0001,
  CPU_ALL_AVR	= (CPU_AVR),

  CPU_Z80	= 0x0001,
  CPU_Z180	= 0x0002,
  CPU_LR35902   = 0x0004,
  CPU_R2K       = 0x0008,
  CPU_R3KA      = 0x0010,
  CPU_EZ80	= 0x0020,
  CPU_Z80N      = 0x0040,
  CPU_GB80      = 0x0080,
  CPU_R3K	= 0x0100,
  CPU_R4K	= 0x0200,
  CPU_R5K	= 0x0400,
  CPU_R6K	= 0x0800,
  CPU_R800	= 0x1000,
  CPU_ALL_Z80   = (CPU_Z80|CPU_Z180|CPU_LR35902|CPU_EZ80|
		   CPU_Z80N|CPU_GB80|
		   CPU_R800),
  CPU_ALL_RXK   =  (CPU_R3K|CPU_R4K|CPU_R5K|CPU_R6K|CPU_R3KA|CPU_R2K),
  
  CPU_XA	= 0x0001,
  CPU_ALL_XA	= (CPU_XA),

  CPU_HC08      = 0x0001,
  CPU_HCS08     = 0x0002,
  CPU_ALL_HC08  = (CPU_HC08|CPU_HCS08),

  CPU_HC11      = 0x0004,
  CPU_HC12      = 0x0008,
  CPU_ALL_HC12  = (CPU_HC11|CPU_HC12),
  
  CPU_PBLAZE_3	= 0x0001,
  CPU_PBLAZE_6	= 0x0002,
  CPU_ALL_PBLAZE= (CPU_PBLAZE_3|CPU_PBLAZE_6),

  CPU_STM8S		= 0x0001,		// S and AF family
  CPU_STM8AF		= 0x0001,
  CPU_STM8SAF		= 0x0001,
  // Devices of S family 0x00 00 00 XX
  DEV_STM8S903		= 0x00000001,
  DEV_STM8S003		= 0x00000002,
  DEV_STM8S005		= 0x00000004,
  DEV_STM8S007		= 0x00000008,
  DEV_STM8S103		= 0x00000010,
  DEV_STM8S105		= 0x00000020,
  DEV_STM8S207		= 0x00000040,
  DEV_STM8S208		= 0x00000080,
  DEV_STM8S		= (DEV_STM8S903|
			   DEV_STM8S003|
			   DEV_STM8S005|
			   DEV_STM8S007|
			   DEV_STM8S103|
			   DEV_STM8S105|
			   DEV_STM8S207|
			   DEV_STM8S208),
  // Devices of AF family 0x00 00 0X 00
  DEV_STM8AF52		= 0x00000100,
  DEV_STM8AF62_12	= 0x00000200,
  DEV_STM8AF62_46	= 0x00000400,
  DEV_STM8AF		= (DEV_STM8AF52|
			   DEV_STM8AF62_12|
			   DEV_STM8AF62_46),

  DEV_STM8SAF		= (DEV_STM8S|DEV_STM8AF),
  
  CPU_STM8L		= 0x0002,		// AL and L family
  // Devices of AL family 0x00 0X 00 00
  DEV_STM8AL3xE		= 0x00010000,
  DEV_STM8AL3x8		= 0x00020000,
  DEV_STM8AL3x346	= 0x00040000,
  DEV_STM8AL		= (DEV_STM8AL3xE|
			   DEV_STM8AL3x8|
			   DEV_STM8AL3x346),
  // Devices of L family 0xXX 00 00 00
  DEV_STM8L051		= 0x01000000,
  DEV_STM8L052C		= 0x02000000,
  DEV_STM8L052R		= 0x04000000,
  DEV_STM8L151x23	= 0x08000000,
  DEV_STM8L15x46	= 0x10000000,
  DEV_STM8L15x8		= 0x20000000,
  DEV_STM8L162		= 0x40000000,
  DEV_STM8L		= (DEV_STM8L051|
			   DEV_STM8L052C|
			   DEV_STM8L052R|
			   DEV_STM8L151x23|
			   DEV_STM8L15x46|
			   DEV_STM8L15x8|
			   DEV_STM8L162),

  DEV_STM8ALL		= (DEV_STM8AL|DEV_STM8L),
  
  CPU_STM8101		= 0x0004,		// L101 family
  CPU_STM8L101		= 0x0004,
  // Devices of L101 family 0x00 00 X0 00
  DEV_STM8101		= 0x00001000,
  DEV_STM8L101		= 0x00001000,
  
  CPU_ALL_STM8	= (CPU_STM8S|CPU_STM8L|CPU_STM8101),

  CPU_ST7       = 0x0001,
  CPU_ALL_ST7   = (CPU_ST7),

  // MOS6502 and variants
  CPU_6502	= 0x0001,	// NMOS
  CPU_6502C	= 0x0002,	// 6502 + HALT pin
  CPU_6510	= 0x0004,	// 6502 + integrated port
  CPU_8500	= 0x0008,	// 6510 CMOS
  CPU_8502	= 0x0010,	// 8500 2 MHz
  CPU_7501	= 0x0020,	// 6502 HMOS-1
  CPU_8501	= 0x0040,	// 6502 HMOS-2

  // 6502 based, but not 100% compatible
  CPU_65C02	= 0x0100,	// extended inst.set
  CPU_65C02S	= 0x0200,      	// 65C02 variant, different inst.set
  CPU_65CE02	= 0x0400,	// extension of 65C02

  // Intel 8080, 8085
  CPU_I8080	= 0x0001,
  CPU_I8085	= 0x0002,

  // Padauk
  CPU_PDK13	= 0x0001,
  CPU_PDK14	= 0x0002,
  CPU_PDK15	= 0x0004,
  CPU_PDK16     = 0x0008,
  CPU_PDKX      = 0x0010,
  
  CPU_F8	= 0x0001,

  CPU_P1516	= 0x0001,
  CPU_P2223	= 0x0002,

  // MCS48 Intel 8048 family
  CPU_I8021	= 0x0001, // 1k-? "1"
  CPU_I8022	= 0x0002, // 2k-? "2"
  CPU_MCS21	= (CPU_I8021|CPU_I8022),
  CPU_I8035	= 0x0010, // 0k-64 "8"
  CPU_I8039	= 0x0020, // 0k-128 "8"
  CPU_I8040	= 0x0040, // 0k-256 "8"
  CPU_MCS30	= (CPU_I8035|CPU_I8039|CPU_I8040),
  CPU_I8041	= 0x0100, // "4"
  CPU_I8041A	= 0x0200, // "A"
  CPU_MCS41	= (CPU_I8041|CPU_I8041A),
  CPU_I8048	= 0x1000, // 1k-64 "8"
  CPU_I8049	= 0x2000, // 2k-128 "8"
  CPU_I8050	= 0x4000, // 4k-256 "8"
  CPU_MCS48	= (CPU_I8048|CPU_I8049|CPU_I8050),

  // OISC
  CPU_OISC	= 0x0001,
  CPU_URISC     = 0x0002,
  CPU_MISC16	= 0x0004,
  CPU_EM        = 0x0008,
  
  // technology
  CPU_CMOS	= 0x0001,
  CPU_HMOS	= 0x0002,
};


struct cpu_entry
{
  const char *type_str;
  enum cpu_type  type;
  int  subtype;
  const char *type_help;
  const char *sub_help;
};

/* Classes of memories, this is index on the list */
enum mem_class
{
  MEM_ROM= 0,
  MEM_XRAM,
  MEM_IRAM,
  MEM_SFR,
  MEM_DUMMY,
  MEM_IXRAM,
  MEM_STACK,
  MEM_TYPES
};

#define MEM_SFR_ID	"sfr"
#define MEM_XRAM_ID	"xram"
#define MEM_IXRAM_ID	"ixram"
#define MEM_IRAM_ID	"iram"
#define MEM_STACK_ID	"stack"
#define MEM_ROM_ID	"rom"

// States of simulator
enum sim_state {
  SIM_NONE	= 0,
  SIM_GO	= 0x01,	// Processor is running
  SIM_QUIT	= 0x02,	// Program must exit
  SIM_STARTEMU	= 0x04,	// Start emulation mode
  SIM_EMU	= 0x08	// Run in emulation mode
};

/* States of CPU */
enum cpu_state {
  stGO		= 0,	/* Normal state */
  stIDLE	= 1,	/* Idle mode is active */
  stPD		= 2	/* Power Down mode is active */
};

/* Result of instruction simulation */
enum inst_result {
  resGO		= 0,	/* OK, go on */
  resWDTRESET	= 1,	/* Reset by WDT */
  resINTERRUPT	= 2,	/* Interrupt accepted */
  resSTOP	= 100,	/* Stop if result greater than this */
  resHALT	= 101,	/* Serious error, halt CPU */
  resINV_ADDR	= 102,	/* Invalid indirect address */
  resSTACK_OV	= 103,	/* Stack overflow */
  resBREAKPOINT	= 104,	/* Fetch Breakpoint */
  resUSER	= 105,	/* Stopped by user */
  resINV_INST	= 106,	/* Invalid instruction */
  resINST_INV	= 106,	/* Invalid instruction */
  resINV	= 106,	/* Invalid instruction */
  resBITADDR	= 107,	/* Bit address is uninterpretable */
  resERROR	= 108,	/* Error happened during instruction exec */
  resSTEP	= 109,	/* Step command done, no more exex needed */
  resSIMIF	= 110,	/* Stopped by simulated prog itself through sim interface */
  resNOT_DONE	= 111,	/* Instruction has not simulated */
  resEVENTBREAK = 112,  /* Event breakpoint */
  resSELFJUMP	= 113,  /* Jump to itself */
};
  
#define BIT_MASK(bitaddr) (1 << (bitaddr & 0x07))


/* Type of breakpoints */
enum brk_perm
{
  brkFIX,	/* f */
  brkDYNAMIC	/* d */
};

enum brk_type
{
  brkFETCH,	/* f */
  brkEVENT	/* e */
};

enum brk_event
{
  brkNONE,
  brkWXRAM,	/* wx */
  brkRXRAM,	/* rx */
  brkRCODE,	/* rc */
  brkWIRAM,	/* wi */
  brkRIRAM,	/* ri */
  brkWSFR,	/* ws */
  brkRSFR,	/* rs */
  brkREAD,
  brkWRITE,
  brkACCESS
};

/* Interrupt levels */
enum intr_levels {
//IT_NO		= -1, /* not in interrupt service */
  IT_LOW	= 1, /* low level interrupt service */
  IT_HIGH	= 2 /* service of high priority interrupt */
};

/* cathegories of hw elements (peripherals) */
enum hw_cath {
  HW_DUMMY	= 0x0000,
  HW_TIMER	= 0x0002,
  HW_UART	= 0x0004,
  HW_PORT	= 0x0008,
  HW_GPIO	= 0x0008,
  HW_PCA	= 0x0010,
  HW_INTERRUPT	= 0x0020,
  HW_WDT	= 0x0040,
  HW_SIMIF	= 0x0080,
  HW_RESET	= 0x0100,
  HW_CLOCK	= 0x0200,
  HW_CALC	= 0x0400,
  HW_FLASH	= 0x0800,
  HW_CPU	= 0x1000
};

// Events that can happen in peripherals
enum hw_event {
  EV_OVERFLOW,
  EV_PORT_CHANGED,
  EV_T2_MODE_CHANGED,
  EV_CLK_ON,
  EV_CLK_OFF
};

// flags of hw units
enum hw_flags {
  HWF_NONE	= 0,
  HWF_INSIDE	= 0x0001,
  HWF_OUTSIDE	= 0x0002,
  HWF_MISC	= 0x0004
};

/* Letter cases */
enum letter_case {
  case_upper,  /* all is upper case */
  case_lower,  /* all is lower case */
  case_case    /* first letter is upper, others are lower case */
};


#endif

/* End of utils.src/stypes.h */
