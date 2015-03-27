/*
 * Simulator of microcontrollers (stypes.h)
 *
 * Copyright (C) 1999,99 Drotos Daniel, Talker Bt.
 * 
 * To contact author send email to drdani@mazsola.iit.uni-miskolc.hu
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


typedef unsigned char	uchar;
typedef unsigned int	uint;
typedef unsigned long	ulong;
typedef TYPE_DWORD	t_addr;		/* 32 bit max */
typedef TYPE_UWORD	t_mem;		/* 16 bit max */
typedef TYPE_WORD	t_smem;		/* signed 16 bit memory */

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

// table of dissassembled instructions
struct dis_entry
{
  uint  code, mask;
  char  branch;
  uchar length;
  const char *mnemonic;
};

// table entry of SFR and BIT names
struct name_entry
{
  int cpu_type;
  t_addr addr;
  const char *name;
};


struct cpu_entry
{
  const char *type_str;
  int  type;
  int  technology;
};

#define CPU_51		0x0001
#define CPU_31		0x0002
#define CPU_52		0x0004
#define CPU_32		0x0008
#define CPU_51R		0x0010
#define CPU_89C51R	0x0020
#define CPU_251		0x0040
#define CPU_DS390	0x0080
#define CPU_DS390F	0x0100
#define CPU_ALL_51	(CPU_51|CPU_31)
#define CPU_ALL_52	(CPU_52|CPU_32|CPU_51R|CPU_89C51R|CPU_251|CPU_DS390|CPU_DS390F)

#define CPU_AVR		0x0001
#define CPU_ALL_AVR	(CPU_AVR)

#define CPU_Z80		0x0001
#define CPU_Z180	0x0002
#define CPU_LR35902	0x0004
#define CPU_R2K		0x0008
#define CPU_R3KA        0x0010
#define CPU_ALL_Z80	(CPU_Z80|CPU_Z180|CPU_R2K|CPU_LR35902|CPU_R3KA)

#define CPU_XA		0x0001
#define CPU_ALL_XA	(CPU_XA)

#define CPU_HC08       0x0001
#define CPU_HCS08      0x0002
#define CPU_ALL_HC08   (CPU_HC08|CPU_HCS08)

#define CPU_STM8	0x0001
#define CPU_ALL_STM8	(CPU_STM8)

#define CPU_ST7       0x0001
#define CPU_ALL_ST7   (CPU_ST7)

#define CPU_CMOS	0x0001
#define CPU_HMOS	0x0002

/* Classes of memories, this is index on the list */
enum mem_class
{
  MEM_ROM= 0,
  MEM_XRAM,
  MEM_IRAM,
  MEM_SFR,
  MEM_DUMMY,
  MEM_IXRAM,
  MEM_TYPES
};

#define MEM_ROM_ID	"rom"
#define MEM_SFR_ID	"sfr"
#define MEM_XRAM_ID	"xram"
#define MEM_IXRAM_ID	"ixram"
#define MEM_IRAM_ID	"iram"

// States of simulator
#define SIM_NONE	0
#define SIM_GO		0x01	// Processor is running
#define SIM_QUIT	0x02	// Program must exit

/* States of CPU */
#define stGO		0	/* Normal state */
#define stIDLE		1	/* Idle mode is active */
#define stPD		2	/* Power Down mode is active */

/* Result of instruction simulation */
#define resGO		0	/* OK, go on */
#define resWDTRESET	1	/* Reseted by WDT */
#define resINTERRUPT	2	/* Interrupt accepted */
#define resSTOP		100	/* Stop if result greather then this */
#define resHALT		101	/* Serious error, halt CPU */
#define resINV_ADDR	102	/* Invalid indirect address */
#define resSTACK_OV	103	/* Stack overflow */
#define resBREAKPOINT	104	/* Breakpoint */
#define resUSER		105	/* Stopped by user */
#define resINV_INST	106	/* Invalid instruction */
#define resBITADDR	107	/* Bit address is uninterpretable */
#define resERROR	108	/* Error happened during instruction exec */

#define BIT_MASK(bitaddr) (1 << (bitaddr & 0x07))

//#define IRAM_SIZE 256	  /* Size of Internal RAM */
//#define SFR_SIZE  256     /* Size of SFR area */
//#define SFR_START 128     /* Start address of SFR area */
//#define ERAM_SIZE 256     /* Size of ERAM in 51R */
//#define XRAM_SIZE 0x10000 /* Size of External RAM */
//#define IROM_SIZE 0x1000  /* Size of Internal ROM */
//#define EROM_SIZE 0x10000 /* Size of External ROM */


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

//struct event_rec
//{
//  t_addr wx; /* write to XRAM at this address, else -1 */
//  t_addr rx; /* read from XRAM at this address, else -1 */
//  t_addr wi; /* write to IRAM at this address, else -1 */
//  t_addr ri; /* read from IRAM at this address, else -1 */
//  t_addr ws; /* write to SFR at this address, else -1 */
//  t_addr rs; /* read from SFR at this address, else -1 */
//  t_addr rc; /* read from ROM at this address, else -1 */
//};

/* Interrupt levels */
//#define IT_NO		-1 /* not in interroupt service */
#define IT_LOW		1 /* low level interrupt service */
#define IT_HIGH		2 /* service of high priority interrupt */

/* cathegories of hw elements (peripherials) */
enum hw_cath {
  HW_DUMMY	= 0x0000,
  HW_TIMER	= 0x0002,
  HW_UART	= 0x0004,
  HW_PORT	= 0x0008,
  HW_PCA	= 0x0010,
  HW_INTERRUPT	= 0x0020,
  HW_WDT	= 0x0040
};

// Events that can happen in peripherals
enum hw_event {
  EV_OVERFLOW,
  EV_PORT_CHANGED,
  EV_T2_MODE_CHANGED
};

// flags of hw units
#define HWF_NONE	0
#define HWF_INSIDE	0x0001
#define HWF_OUTSIDE	0x0002
#define HWF_MISC	0x0004


/* Letter cases */
enum letter_case {
  case_upper,  /* all is upper case */
  case_lower,  /* all is lower case */
  case_case    /* first letter is upper, others are lower case */
};


#endif

/* End of stypes.h */
