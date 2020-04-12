/*
 * Simulator of microcontrollers (xa.cc)
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

//#include "ddconfig.h"

#include <stdio.h>
#include <stdlib.h>
//#include <ctype.h>
#include <string.h>

// prj
//#include "pobjcl.h"

// sim
#include "simcl.h"

// local
#include "xacl.h"
#include "glob.h"
//#include "regsxa.h"


/*
 * Names of SFRs
 */

static struct name_entry sfr_tabXA51[]=
{
  //#include "xa_sfr.cc"
  {CPU_XA, 0x400, "PSW"}, /* Program status word */
  {CPU_XA, 0x400, "PSWL"}, /* Program status word (low byte) */
  {CPU_XA, 0x401, "PSWH"}, /* Program status word (high byte) */
  {CPU_XA, 0x402, "PSW51"}, /* 80C51 compatible PSW */
  {CPU_XA, 0x403, "SSEL"}, /* Segment selection register */
  {CPU_XA, 0x404, "PCON"}, /* Power control register */
  {CPU_XA, 0x410, "TCON"}, /* Timer 0 and 1 control register */
  {CPU_XA, 0x411, "TSTAT"}, /* Timer 0 and 1 extended status */
  {CPU_XA, 0x418, "T2CON"}, /* Timer 2 control register */
  {CPU_XA, 0x419, "T2MOD"}, /* Timer 2 mode control */
  {CPU_XA, 0x41F, "WDCON"}, /* Watchdog control register */
  {CPU_XA, 0x420, "S0CON"}, /* Serial port 0 control register */
  {CPU_XA, 0x421, "S0STAT"}, /* Serial port 0 extended status */
  {CPU_XA, 0x424, "S1CON"}, /* Serial port 1 control register */
  {CPU_XA, 0x425, "S1STAT"}, /* Serial port 1 extended status */
  {CPU_XA, 0x426, "IEL"}, /* Interrupt enable low byte */
  {CPU_XA, 0x427, "IEH"}, /* Interrupt enable high byte */
  {CPU_XA, 0x42A, "SWR"}, /* Software Interrupt Request */
  {CPU_XA, 0x430, "P0"}, /* Port 0 */
  {CPU_XA, 0x431, "P1"}, /* Port 1 */
  {CPU_XA, 0x432, "P2"}, /* Port 2 */
  {CPU_XA, 0x433, "P3"}, /* Port3 */
  {CPU_XA, 0x440, "SCR"}, /* System configuration register */
  {CPU_XA, 0x441, "DS"}, /* Data segment */
  {CPU_XA, 0x442, "ES"}, /* Extra segment */
  {CPU_XA, 0x443, "CS"}, /* Code segment */
  {CPU_XA, 0x450, "TL0"}, /* Timer 0 low byte */
  {CPU_XA, 0x451, "TH0"}, /* Timer 0 high byte */
  {CPU_XA, 0x452, "TL1"}, /* Timer 1 low byte */
  {CPU_XA, 0x453, "TH1"}, /* Timer 1 high byte */
  {CPU_XA, 0x454, "RTL0"}, /* Timer 0 extended reload, low byte */
  {CPU_XA, 0x455, "RTH0"}, /* Timer 0 extended reload, high byte */
  {CPU_XA, 0x456, "RTL1"}, /* Timer 1 extended reload, low byte */
  {CPU_XA, 0x457, "RTH1"}, /* Timer 1 extended reload, high byte */
  {CPU_XA, 0x458, "TL2"}, /* Timer 2 low byte */
  {CPU_XA, 0x459, "TH2"}, /* Timer 2 high byte */
  {CPU_XA, 0x45A, "T2CAPL"}, /* Timer 2 capture register, low byte */
  {CPU_XA, 0x45B, "T2CAPH"}, /* Timer 2 capture register, high byte */
  {CPU_XA, 0x45C, "TMOD"}, /* Timer 0 and 1 mode register */
  {CPU_XA, 0x45D, "WFEED1"}, /* Watchdog feed 1 */
  {CPU_XA, 0x45E, "WFEED2"}, /* Watchdog feed 2 */
  {CPU_XA, 0x45F, "WDL"}, /* Watchdog timer reload */
  {CPU_XA, 0x460, "S0BUF"}, /* Serial port 0 buffer register */
  {CPU_XA, 0x461, "S0ADDR"}, /* Serial port 0 address register */
  {CPU_XA, 0x462, "S0ADEN"}, /* Serial port 0 address enable register */
  {CPU_XA, 0x464, "S1BUF"}, /* Serial port 1 buffer register */
  {CPU_XA, 0x465, "S1ADDR"}, /* Serial port 1 address register */
  {CPU_XA, 0x466, "S1ADEN"}, /* Serial port 1 address enable register */
  {CPU_XA, 0x468, "BTRL"}, /* Bus timing register high byte */
  {CPU_XA, 0x469, "BTRH"}, /* Bus timing register low byte */
  {CPU_XA, 0x46A, "BCR"}, /* Bus configuration register */
  {CPU_XA, 0x470, "P0CFGA"}, /* Port 0 configuration A */
  {CPU_XA, 0x471, "P1CFGA"}, /* Port 1 configuration A */
  {CPU_XA, 0x472, "P2CFGA"}, /* Port 2 configuration A */
  {CPU_XA, 0x473, "P3CFGA"}, /* Port 3 configuration A */
  {CPU_XA, 0x47A, "SWE"}, /* Software Interrupt Enable */
  {CPU_XA, 0x4A0, "IPA0"}, /* Interrupt priority 0 */
  {CPU_XA, 0x4A1, "IPA1"}, /* Interrupt priority 1 */
  {CPU_XA, 0x4A2, "IPA2"}, /* Interrupt priority 2 */
  {CPU_XA, 0x4A4, "IPA4"}, /* Interrupt priority 4 */
  {CPU_XA, 0x4A5, "IPA5"}, /* Interrupt priority 5 */
  {CPU_XA, 0x4F0, "P0CFGB"}, /* Port 0 configuration B */
  {CPU_XA, 0x4F1, "P1CFGB"}, /* Port 1 configuration B */
  {CPU_XA, 0x4F2, "P2CFGB"}, /* Port 2 configuration B */
  {CPU_XA, 0x4F3, "P3CFGB"}, /* Port 3 configuration B */
  { 0, 0, NULL }
};

/*
 * Names of SBITs
 */

static struct name_entry bit_tabXA51[]=
{
  //#include "xa_bit.cc"
  {CPU_XA, 0x33B, "ETI1"}, /* TX interrupt enable 1 */
  {CPU_XA, 0x33A, "ERI1"}, /* RX interrupt enable 1 */
  {CPU_XA, 0x339, "ETI0"}, /* TX interrupt enable 0 */
  {CPU_XA, 0x338, "ERI0"}, /* RX interrupt enable 0 */
  {CPU_XA, 0x337, "EA"}, /* global int. enable */
  {CPU_XA, 0x334, "ET2"}, /* timer 2 interrupt */
  {CPU_XA, 0x333, "ET1"}, /* timer 1 interrupt */
  {CPU_XA, 0x332, "EX1"}, /* external interrupt 1 */
  {CPU_XA, 0x331, "ET0"}, /* timer 0 interrupt */
  {CPU_XA, 0x330, "EX0"}, /* external interrupt 0 */
  {CPU_XA, 0x221, "PD"}, /* power down */
  {CPU_XA, 0x220, "IDL"},
  {CPU_XA, 0x20F, "SM"},
  {CPU_XA, 0x20E, "TM"},
  {CPU_XA, 0x20D, "RS1"},
  {CPU_XA, 0x20C, "RS0"},
  {CPU_XA, 0x20B, "IM3"},
  {CPU_XA, 0x20A, "IM2"},
  {CPU_XA, 0x209, "IM1"},
  {CPU_XA, 0x208, "IM0"},
  {CPU_XA, 0x307, "S0M0"},
  {CPU_XA, 0x306, "S0M1"},
  {CPU_XA, 0x305, "S0M2"},
  {CPU_XA, 0x304, "R0EN"},
  {CPU_XA, 0x303, "T0B8"},
  {CPU_XA, 0x302, "R0B8"},
  {CPU_XA, 0x301, "TI0"}, /* serial port 0 tx ready */
  {CPU_XA, 0x300, "RI0"}, /* serial port 0 rx ready */
  {CPU_XA, 0x30B, "FE0"},
  {CPU_XA, 0x30A, "BR0"},
  {CPU_XA, 0x309, "OE0"},
  {CPU_XA, 0x308, "STINT0"},
  {CPU_XA, 0x327, "S1M0"},
  {CPU_XA, 0x326, "S1M1"},
  {CPU_XA, 0x325, "S1M2"},
  {CPU_XA, 0x324, "R1EN"},
  {CPU_XA, 0x323, "T1B8"},
  {CPU_XA, 0x322, "R1B8"},
  {CPU_XA, 0x321, "TI1"}, /* serial port 0 tx ready */
  {CPU_XA, 0x320, "RI1"}, /* serial port 0 rx ready */
  {CPU_XA, 0x32B, "FE1"},
  {CPU_XA, 0x32A, "BR1"},
  {CPU_XA, 0x329, "OE1"},
  {CPU_XA, 0x328, "STINT1"},
  {CPU_XA, 0x356, "SWR7"},
  {CPU_XA, 0x355, "SWR6"},
  {CPU_XA, 0x354, "SWR5"},
  {CPU_XA, 0x353, "SWR4"},
  {CPU_XA, 0x352, "SWR3"},
  {CPU_XA, 0x351, "SWR2"},
  {CPU_XA, 0x350, "SWR1"},
  {CPU_XA, 0x2C7, "TF2"},
  {CPU_XA, 0x2C6, "EXF2"},
  {CPU_XA, 0x2C5, "RCLK0"},
  {CPU_XA, 0x2C4, "TCLK0"},
  {CPU_XA, 0x2CD, "RCLK1"},
  {CPU_XA, 0x2CC, "TCLK1"},
  {CPU_XA, 0x2C3, "EXEN2"},
  {CPU_XA, 0x2C2, "TR2"},
  {CPU_XA, 0x2C1, "CT2"},
  {CPU_XA, 0x2C0, "CPRL2"},
  {CPU_XA, 0x2C9, "T2OE"},
  {CPU_XA, 0x2C8, "DCEN"},
  {CPU_XA, 0x287, "TF1"},
  {CPU_XA, 0x286, "TR1"},
  {CPU_XA, 0x285, "TF0"},
  {CPU_XA, 0x284, "TR0"},
  {CPU_XA, 0x283, "IE1"},
  {CPU_XA, 0x282, "IT1"},
  {CPU_XA, 0x281, "IE0"},
  {CPU_XA, 0x280, "IT0"},
  {CPU_XA, 0x28A, "T1OE"},
  {CPU_XA, 0x288, "T0OE"},
  {CPU_XA, 0x2FF, "PRE2"},
  {CPU_XA, 0x2FE, "PRE1"},
  {CPU_XA, 0x2FD, "PRE0"},
  {CPU_XA, 0x2FA, "WDRUN"},
  {CPU_XA, 0x2F9, "WDTOF"},
  {CPU_XA, 0x2F8, "WDMOD"},
  {CPU_XA, 0x388, "WR1"},
  {CPU_XA, 0x38F, "T2EX"},
  {CPU_XA, 0x38C, "RXD1"},
  {CPU_XA, 0x38D, "TXD1"},
  {CPU_XA, 0x398, "RXD0"},
  {CPU_XA, 0x399, "TXD0"},
  {CPU_XA, 0x39A, "INT0"},
  {CPU_XA, 0x39B, "INT1"},
  {CPU_XA, 0x39C, "T0"},
  {CPU_XA, 0x39D, "T1"},
  {CPU_XA, 0x39E, "WR"},
  {CPU_XA, 0x39F, "RD"},
  { 0, 0, NULL }
};


/*
 * Base type of xa controllers
 */

cl_xa::cl_xa(class cl_sim *asim):
  cl_uc(asim)
{
  type= (struct cpu_entry *)malloc(sizeof(struct cpu_entry));
  type->type= CPU_XA;
}

int
cl_xa::init(void)
{
  cl_uc::init(); /* Memories now exist */

  /* set SCR to osc/4, native XA mode, flat 24 */
  set_scr(0);
  /* initialize SP to 100H */
  set_reg2(7, 0x100);
  /* set PSW from reset vector */
  set_psw(getcode2(0));
  /* set PC from reset vector */
  PC = getcode2(2);

  printf("The XA Simulator is in development, UNSTABLE, DEVELOPERS ONLY!\n");

  int i;
  for (i= 0; sfr_tabXA51[i].name != NULL; i++)
    {
      if (type->type & sfr_tabXA51[i].cpu_type)
	{
	  class cl_var *v;
	  vars->add(v= new cl_var(chars(sfr_tabXA51[i].name),
				  sfr,
				  sfr_tabXA51[i].addr, ""));
	  v->init();
	}
    }
  for (i= 0; bit_tabXA51[i].name != NULL; i++)
    {
      if (type->type & bit_tabXA51[i].cpu_type)
	{
	  class cl_var *v;
	  t_addr a= bit_tabXA51[i].addr;
	  int bitnr, offset= 0;
	  if (a >= 0x200)
	    {
	      a-= 0x200;
	      offset= 0x400;
	    }
	  bitnr= a%8;
	  a= offset + a/8;
	  vars->add(v= new cl_var(chars(bit_tabXA51[i].name),
				  sfr,
				  a, "", bitnr));
	  v->init();
	}
    }
  return(0);
}


char *
cl_xa::id_string(void)
{
  return((char*)"unspecified XA");
}


/*
 * Making elements of the controller
 */

void
cl_xa::mk_hw_elements(void)
{
  //class cl_base *o;
  cl_uc::mk_hw_elements();
}

void
cl_xa::make_memories(void)
{
  class cl_address_space *as;

  as= rom= new cl_address_space("rom", 0, 0x10000, 8);
  as->init();
  address_spaces->add(as);
  as= iram= new cl_address_space("iram", 0, 0x2000, 8);
  as->init();
  address_spaces->add(as);
  as= sfr= new cl_address_space("sfr", 0x0, 0x2000, 8);
  as->init();
  address_spaces->add(as);
  as= ram= new cl_address_space("xram", 0, 0x10000, 8);
  as->init();
  address_spaces->add(as);

  class cl_address_decoder *ad;
  class cl_memory_chip *chip;

  chip= new cl_memory_chip("rom_chip", 0x10000, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as= address_space("rom"), chip, 0, 0xffff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  chip= new cl_memory_chip("iram_chip", 0x2000, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as= address_space("iram"), chip, 0, 0x1fff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  chip= new cl_memory_chip("xram_chip", 0x10000, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as= address_space("xram"), chip, 0, 0xffff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  chip= new cl_memory_chip("sfr_chip", 0x2000, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as= address_space("sfr"), chip, 0x0, 0x1fff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);
}


/*
 * Help command interpreter
 */

struct dis_entry *
cl_xa::dis_tbl(void)
{
  // this should be unused, we need to make main prog code
  // independent of any array thing.
  printf("ERROR - Using disass[] table in XA sim code!\n");
  return(glob_disass_xa);
}

struct name_entry *cl_xa::sfr_tbl(void)
{
  return(sfr_tabXA51);
}

struct name_entry *cl_xa::bit_tbl(void)
{
  return(bit_tabXA51);
}

int
cl_xa::inst_length(t_addr addr)
{
  int len = 0;

  get_disasm_info(addr, &len, NULL, NULL, NULL, NULL);

  return len;
}

int
cl_xa::inst_branch(t_addr addr)
{
  int b;

  get_disasm_info(addr, NULL, &b, NULL, NULL, NULL);

  return b;
}

int
cl_xa::longest_inst(void)
{
  return 6;
}

static char dir_name[64];
char *cl_xa::get_dir_name(short addr) {
  if (!/*get*/addr_name(addr, sfr/*_tbl()*/, dir_name)) {
    sprintf (dir_name, "0x%03x", addr);
  }
  return dir_name;
}

static char bit_name[64];
char *cl_xa::get_bit_name(short addr) {
  t_addr a= addr; int offset= 0, bitnr= addr%8;
  if (a >= 0x200) { a-= 0x200; offset= 0x400; }
  a= offset+a/8;
  if (!/*get*/addr_name(a/*ddr*/, sfr/*bit_tbl()*/, bitnr, bit_name)) {
    sprintf (bit_name, "0x%03x", addr);
  }
  return bit_name;
}

/*--------------------------------------------------------------------
get_disasm_info - Given an address, return information about the opcode
  which resides there.
  addr - address of opcode we want information on.
  ret_len - return length of opcode.
  ret_branch - return a character which indicates if we are
    a branching opcode.  Used by main app to implement "Next"
    function which steps over functions.
  immed_offset - return a number which represents the number of bytes
    offset to where any immediate data is(tail end of opcode).  Used
    for printing disassembly.
  operands - return a key indicating the form of the operands,
    used for printing the disassembly.
  mnemonic - return a key indicating the mnemonic of the instruction.

  Return value: Return the operand code formed by either the single
  byte opcode or 2 bytes of the opcode for multi-byte opcodes.

  Note: Any of the return pointer parameters can be set to NULL to
    indicate the caller does not want the information.
|--------------------------------------------------------------------*/
int
cl_xa::get_disasm_info(t_addr addr,
                       int *ret_len,
                       int *ret_branch,
                       int *immed_offset,
                       int *parms,
                       int *mnemonic)
{
  uint code;
  //int len = 0;
  int immed_n = 0;
  int i;
  int start_addr = addr;

  code= rom->get(addr++);
  if (code == 0x00) {
    i= 0;
    while (disass_xa[i].mnemonic != NOP)
      i++;
  } else {
    //len = 2;
    code = (code << 8) | rom->get(addr++);
    i= 0;
    while ((code & disass_xa[i].mask) != disass_xa[i].code &&
           disass_xa[i].mnemonic != BAD_OPCODE)
      i++;
  }

  if (ret_len)
    *ret_len = disass_xa[i].length;
  if (ret_branch)
   *ret_branch = disass_xa[i].branch;
  if (immed_offset) {
    if (immed_n > 0)
         *immed_offset = immed_n;
    else *immed_offset = (addr - start_addr);
  }
  if (parms) {
    *parms = disass_xa[i].operands;
  }
  if (mnemonic) {
    *mnemonic = disass_xa[i].mnemonic;
  }

  return code;
}

static const char *w_reg_strs[] = {
 "R0", "R1",
 "R2", "R3",
 "R4", "R5",
 "R6", "R7",
 "R8", "R9",
 "R10", "R11",
 "R12", "R13",
 "R14", "R15"};

static const char *b_reg_strs[] = {
 "R0l", "R0h",
 "R1l", "R1h",
 "R2l", "R2h",
 "R3l", "R3h",
 "R4l", "R4h",
 "R5l", "R5h",
 "R6l", "R6h",
 "R7l", "R7h"};

/*--------------------------------------------------------------------
disass - Disassemble an opcode.
    addr - address of opcode to disassemble/print.
    sep - optionally points to string(tab) to use as separator.
|--------------------------------------------------------------------*/
char *
cl_xa::disass(t_addr addr, const char *sep)
{
  char work[256], parm_str[40];
  char *buf, *p, *b;
  int code;
  int len = 0;
  int immed_offset = 0;
  int operands;
  int mnemonic;
  const char **reg_strs;

  p= work;

  code = get_disasm_info(addr, &len, NULL, &immed_offset, &operands, &mnemonic);

  if (mnemonic == BAD_OPCODE) {
    buf= (char*)malloc(30);
    strcpy(buf, "UNKNOWN/INVALID");
    return(buf);
  }

  if (code & 0x0800)
    reg_strs = w_reg_strs;
  else
    reg_strs = b_reg_strs;

  switch(operands) {
     // the repeating common parameter encoding for ADD, ADDC, SUB, AND...
    case REG_REG :
      sprintf(parm_str, "%s,%s",
              reg_strs[((code >> 4) & 0xf)],
              reg_strs[(code & 0xf)]);
    break;
    case REG_IREG :
      sprintf(parm_str, "%s,[%s]",
              reg_strs[((code >> 4) & 0xf)],
              w_reg_strs[(code & 0xf)]);
    break;
    case IREG_REG :
      sprintf(parm_str, "[%s],%s",
              w_reg_strs[(code & 0x7)],
              reg_strs[((code >> 4) & 0xf)] );
    break;
    case REG_IREGOFF8 :
      sprintf(parm_str, "%s,[%s+%02x]",
              reg_strs[((code >> 4) & 0xf)],
              w_reg_strs[(code & 0x7)],
              rom->get(addr+immed_offset));
      ++immed_offset;
    break;
    case IREGOFF8_REG :
      sprintf(parm_str, "[%s+%02x],%s",
              w_reg_strs[(code & 0x7)],
              rom->get(addr+immed_offset),
              reg_strs[((code >> 4) & 0xf)] );
      ++immed_offset;
    break;
    case REG_IREGOFF16 :
      sprintf(parm_str, "%s,[%s+%04x]",
              reg_strs[((code >> 4) & 0xf)],
              w_reg_strs[(code & 0x7)],
              (short)((rom->get(addr+immed_offset+1)) |
                     (rom->get(addr+immed_offset)<<8)) );
      ++immed_offset;
      ++immed_offset;
    break;
    case IREGOFF16_REG :
      sprintf(parm_str, "[%s+%04x],%s",
              w_reg_strs[(code & 0x7)],
              (short)((rom->get(addr+immed_offset+1)) |
                     (rom->get(addr+immed_offset)<<8)),
              reg_strs[((code >> 4) & 0xf)] );
      ++immed_offset;
      ++immed_offset;
    break;
    case REG_IREGINC :
      sprintf(parm_str, "%s,[%s+]",
              reg_strs[((code >> 4) & 0xf)],
              w_reg_strs[(code & 0xf)]);
    break;
    case IREGINC_REG :
      sprintf(parm_str, "[%s+],%s",
              w_reg_strs[(code & 0x7)],
              reg_strs[((code >> 4) & 0xf)] );
    break;
    case DIRECT_REG :
      sprintf(parm_str, "%s,%s",
              get_dir_name(((code & 0x7) << 8) |
                           rom->get(addr+immed_offset)),
              reg_strs[((code >> 4) & 0xf)] );
      ++immed_offset;
    break;
    case REG_DIRECT :
      sprintf(parm_str, "%s,%s",
              reg_strs[((code >> 4) & 0xf)],
              get_dir_name(((code & 0x7) << 8) |
                           rom->get(addr+immed_offset)));
      ++immed_offset;
    break;
    case REG_DATA8 :
      sprintf(parm_str, "%s,#0x%02x",
              b_reg_strs[((code >> 4) & 0xf)],
              rom->get(addr+immed_offset) );
      ++immed_offset;
    break;
    case REG_DATA16 :
      sprintf(parm_str, "%s,#0x%04x",
              reg_strs[((code >> 4) & 0xf)],
              (short)((rom->get(addr+immed_offset+1)) |
                     (rom->get(addr+immed_offset)<<8)) );
      ++immed_offset;
      ++immed_offset;
    break;
    case IREG_DATA8 :
      sprintf(parm_str, "[%s], 0x%02x",
              w_reg_strs[((code >> 4) & 0x7)],
              rom->get(addr+immed_offset) );
      ++immed_offset;
    break;
    case IREG_DATA16 :
      sprintf(parm_str, "[%s], 0x%04x",
              w_reg_strs[((code >> 4) & 0x7)],
              (short)((rom->get(addr+immed_offset+1)) |
                     (rom->get(addr+immed_offset)<<8)) );
      ++immed_offset;
      ++immed_offset;
    break;
    case IREGINC_DATA8 :
      sprintf(parm_str, "[%s+], 0x%02x",
              w_reg_strs[((code >> 4) & 0x7)],
              rom->get(addr+immed_offset) );
      ++immed_offset;
    break;
    case IREGINC_DATA16 :
      sprintf(parm_str, "[%s+], 0x%04x",
              w_reg_strs[((code >> 4) & 0x7)],
              (short)((rom->get(addr+immed_offset+1)) |
                     (rom->get(addr+immed_offset)<<8)) );
      ++immed_offset;
      ++immed_offset;
    break;
    case IREGOFF8_DATA8 :
      sprintf(parm_str, "[%s+%02x], 0x%02x",
              w_reg_strs[((code >> 4) & 0x7)],
              rom->get(addr+immed_offset),
              rom->get(addr+immed_offset+1) );
      immed_offset += 2;
    break;
    case IREGOFF8_DATA16 :
      sprintf(parm_str, "[%s+%02x], 0x%04x",
              w_reg_strs[((code >> 4) & 0x7)],
              rom->get(addr+immed_offset),
              (short)((rom->get(addr+immed_offset+2)) |
                     (rom->get(addr+immed_offset+1)<<8)) );
      immed_offset += 3;
    break;
    case IREGOFF16_DATA8 :
      sprintf(parm_str, "[%s+%04x], 0x%02x",
              w_reg_strs[((code >> 4) & 0x7)],
              (short)((rom->get(addr+immed_offset+1)) |
                     (rom->get(addr+immed_offset+0)<<8)),
              rom->get(addr+immed_offset+2) );
      immed_offset += 3;
    break;
    case IREGOFF16_DATA16 :
      sprintf(parm_str, "[%s+%04x], 0x%04x",
              w_reg_strs[((code >> 4) & 0x7)],
              (short)((rom->get(addr+immed_offset+1)) |
                     (rom->get(addr+immed_offset+0)<<8)),
              (short)((rom->get(addr+immed_offset+3)) |
                     (rom->get(addr+immed_offset+2)<<8)) );
      immed_offset += 4;
    break;
    case DIRECT_DATA8 :
      sprintf(parm_str, "%s,#0x%02x",
              get_dir_name(((code & 0x0070) << 4) |
                           rom->get(addr+immed_offset)),
              rom->get(addr+immed_offset+1));
      immed_offset += 3;
    break;
    case DIRECT_DATA16 :
      sprintf(parm_str, "%s,#0x%04x",
              get_dir_name(((code & 0x0070) << 4) |
                           rom->get(addr+immed_offset)),
              rom->get(addr+immed_offset+2) +
              (rom->get(addr+immed_offset+1)<<8));
      immed_offset += 3;
    break;

// odd-ball ones
    case NO_OPERANDS :  // for NOP
      strcpy(parm_str, "");
    break;
    case CY_BIT :
      sprintf(parm_str, "C,%s",
             get_bit_name(((code&0x0003)<<8) + rom->get(addr+2)));
    break;
    case BIT_CY :
      sprintf(parm_str, "%s,C",
              get_bit_name(((code&0x0003)<<8) + rom->get(addr+2)));
    break;
    case REG_DATA4 :
      strcpy(parm_str, "REG_DATA4");
    break;
    case REG_DATA5 :
      strcpy(parm_str, "REG_DATA5");
    break;
    case IREG_DATA4 :
      strcpy(parm_str, "IREG_DATA4");
    break;
    case IREGINC_DATA4 :
      strcpy(parm_str, "IREGINC_DATA4");
    break;
    case IREGOFF8_DATA4 :
      strcpy(parm_str, "IREGOFF8_DATA4");
    break;
    case IREGOFF16_DATA4 :
      strcpy(parm_str, "IREGOFF16_DATA4");
    break;
    case DIRECT_DATA4 :
      sprintf(parm_str, "%s,#0x%x",
	      get_dir_name(((code & 0x70)<<4) |
			   rom->get(addr+2)),
	      code&0x0f);
    break;
    case DIRECT :
      sprintf(parm_str, "%s",
              get_dir_name(((code & 0x007) << 4) +
                           rom->get(addr+2)));
    break;
    case REG :
      sprintf(parm_str, "%s",
              reg_strs[((code >> 4) & 0xf)] );
    break;
    case IREG :
      sprintf(parm_str, "[%s]",
              reg_strs[((code >> 4) & 0xf)] );
    break;
    case BIT_ALONE :
      sprintf(parm_str, "%s",
	      get_bit_name(((code&0x0003)<<8) + rom->get(addr+2)));
    break;
    case BIT_REL8 :
      sprintf(parm_str, "%s,0x%04lx",
	      get_bit_name((code&0x0003)<<8) + rom->get(addr+2),
	      (long)(((signed char)rom->get(addr+3)*2+addr+len)&0xfffe));
    break;
    case DATA4:
      sprintf(parm_str, "#0x%02x", code&0x0f);
      break;
    case ADDR24 :
      sprintf(parm_str, "0x%06x",
             (rom->get(addr+3)<<16) +
             (rom->get(addr+1)<<8) +
             rom->get(addr+2));
      break;
    break;
    case REG_REL8 :
      sprintf(parm_str, "%s,0x%04lx",
	      reg_strs[(code>>4) & 0xf],
	      (long)(((signed char)rom->get(addr+2)*2+addr+len)&0xfffe));
    break;
    case DIRECT_REL8 :
      sprintf(parm_str, "%s,0x%04lx",
	      get_dir_name(((code&0x07)<<8) +
			   rom->get(addr+2)),
	      (long)(((signed char)rom->get(addr+2)*2+addr+len)&0xfffe));
    break;
    case REG_USP:
      sprintf(parm_str, "REG_USP");
    break;
    case USP_REG:
      sprintf(parm_str, "USP_REG");
    break;
    case REL8 :
      sprintf(parm_str, "0x%04lx",
	      (long)(((signed char)rom->get(addr+1)*2+addr+len)&0xfffe));
    break;
    case REL16 :
      sprintf(parm_str, "0x%04lx",
	      (long)(((signed short)((rom->get(addr+1)<<8) + rom->get(addr+2))*2+addr+len)&0xfffe));
    break;

    case RLIST : {
      /* TODO: the list should be comma reperated
	 and maybe for POP the list should be reversed */
      unsigned char rlist=code&0xff;
      parm_str[0]='\0';
      if (code&0x0800) { // word list
	if (code&0x4000) { // R8-R15
	  if (rlist&0x80) strcat (parm_str, "R15 ");
	  if (rlist&0x40) strcat (parm_str, "R14");
	  if (rlist&0x20) strcat (parm_str, "R13 ");
	  if (rlist&0x10) strcat (parm_str, "R12 ");
	  if (rlist&0x08) strcat (parm_str, "R11 ");
	  if (rlist&0x04) strcat (parm_str, "R10 ");
	  if (rlist&0x02) strcat (parm_str, "R9 ");
	  if (rlist&0x01) strcat (parm_str, "R8 ");
	} else { // R7-R0
	  if (rlist&0x80) strcat (parm_str, "R7 ");
	  if (rlist&0x40) strcat (parm_str, "R6 ");
	  if (rlist&0x20) strcat (parm_str, "R5 ");
	  if (rlist&0x10) strcat (parm_str, "R4 ");
	  if (rlist&0x08) strcat (parm_str, "R3 ");
	  if (rlist&0x04) strcat (parm_str, "R2 ");
	  if (rlist&0x02) strcat (parm_str, "R1 ");
	  if (rlist&0x01) strcat (parm_str, "R0 ");
	}
      } else { // byte list
	if (code&0x4000) { //R7h-R4l
	  if (rlist&0x80) strcat (parm_str, "R7h ");
	  if (rlist&0x40) strcat (parm_str, "R7l ");
	  if (rlist&0x20) strcat (parm_str, "R6h ");
	  if (rlist&0x10) strcat (parm_str, "R6l ");
	  if (rlist&0x08) strcat (parm_str, "R5h ");
	  if (rlist&0x04) strcat (parm_str, "R5l ");
	  if (rlist&0x02) strcat (parm_str, "R4h ");
	  if (rlist&0x01) strcat (parm_str, "R4l ");
	} else { // R3h-R0l
	  if (rlist&0x80) strcat (parm_str, "R3h ");
	  if (rlist&0x40) strcat (parm_str, "R3l ");
	  if (rlist&0x20) strcat (parm_str, "R2h ");
	  if (rlist&0x10) strcat (parm_str, "R2l ");
	  if (rlist&0x08) strcat (parm_str, "R1h ");
	  if (rlist&0x04) strcat (parm_str, "R1l ");
	  if (rlist&0x02) strcat (parm_str, "R0h ");
	  if (rlist&0x01) strcat (parm_str, "R0l ");
	}
      }
    }
    break;

    case REG_DIRECT_REL8 :
      sprintf(parm_str, "%s,%s,0x%02x",
              reg_strs[((code >> 4) & 0xf)],
              get_dir_name(((code & 0x7) << 8) +
                           rom->get(addr+immed_offset)),
              ((signed char) rom->get(addr+immed_offset+1) * 2) & 0xfffe );
    break;
    case REG_DATA8_REL8 :
      sprintf(parm_str, "%s,#0x%02x,0x%02x",
              reg_strs[((code >> 4) & 0xf)],
              rom->get(addr+immed_offset+1),
              ((signed char)rom->get(addr+immed_offset) * 2) & 0xfffe );
    break;
    case REG_DATA16_REL8 :
      sprintf(parm_str, "%s,#0x%04x,0x%02x",
              w_reg_strs[(code >> 4) & 0xf],
              rom->get(addr+immed_offset+2) +
                 (rom->get(addr+immed_offset+1)<<8),
              ((signed char)rom->get(addr+immed_offset) * 2) & 0xfffe );
    break;
    case IREG_DATA8_REL8 :
      sprintf(parm_str, "[%s],#0x%02x,0x%02x",
              reg_strs[((code >> 4) & 0x7)],
              rom->get(addr+immed_offset+1),
              ((signed char)rom->get(addr+immed_offset) * 2) & 0xfffe );
    break;
    case IREG_DATA16_REL8 :
      sprintf(parm_str, "[%s],#0x%04x,0x%02x",
              w_reg_strs[(code >> 4) & 0x7],
              rom->get(addr+immed_offset+2) +
	        (rom->get(addr+immed_offset+1)<<8),
              ((signed char)rom->get(addr+immed_offset) * 2) & 0xfffe );
    break;

    case A_APLUSDPTR :
      strcpy(parm_str, "A, [A+DPTR]");
    break;

    case A_APLUSPC :
      strcpy(parm_str, "A, [A+PC]");
    break;

    case REG_REGOFF8 :
      sprintf(parm_str, "%s,%s+0x%02x",
              w_reg_strs[(code >> 4) & 0x7],
              w_reg_strs[code & 0x7],
              rom->get(addr+immed_offset));
      break;

    case REG_REGOFF16 :
      sprintf(parm_str, "%s,%s+0x%02x",
              w_reg_strs[(code >> 4) & 0x7],
              w_reg_strs[code & 0x7],
              rom->get(addr+immed_offset+1) +
	        (rom->get(addr+immed_offset+0)<<8));
      break;

    case A_PLUSDPTR :
      strcpy(parm_str, "[A+DPTR]");
      break;

    case IIREG :
      sprintf(parm_str, "[[%s]]",
              w_reg_strs[(code & 0x7)]);
      break;

    default:
      strcpy(parm_str, "???");
    break;
  }

  sprintf(work, "%s %s",
          op_mnemonic_str[ mnemonic ],
          parm_str);

  p= strchr(work, ' ');
  if (!p)
    {
      buf= strdup(work);
      return(buf);
    }
  if (sep == NULL)
    buf= (char *)malloc(6+strlen(p)+1);
  else
    buf= (char *)malloc((p-work)+strlen(sep)+strlen(p)+1);

  for (p= work, b= buf; *p != ' '; p++, b++)
    *b= *p;
  p++;
  *b= '\0';
  if (sep == NULL)
    {
      while (strlen(buf) < 6)
        strcat(buf, " ");
    }
  else
    strcat(buf, sep);
  strcat(buf, p);
  return(buf);
}

/*--------------------------------------------------------------------
 print_regs - Print the registers, flags and other useful information.
   Used to print a status line while stepping through the code.
|--------------------------------------------------------------------*/
void
cl_xa::print_regs(class cl_console_base *con)
{
  unsigned char flags;

  flags = get_psw();
  con->dd_printf("CA---VNZ | ", flags);
  con->dd_printf("R0:%04x R1:%04x R2:%04x R3:%04x\n",
                 reg2(0), reg2(1), reg2(2), reg2(3));

  con->dd_printf("%c%c---%c%c%c | ",
         (flags & BIT_C)?'1':'0',
         (flags & BIT_AC)?'1':'0',
         (flags & BIT_V)?'1':'0',
         (flags & BIT_N)?'1':'0',
         (flags & BIT_Z)?'1':'0');

  con->dd_printf("R4:%04x R5:%04x R6:%04x SP:%04x ES:%04x  DS:%04x\n",
                 reg2(4), reg2(5), reg2(6), reg2(7), 0, 0);

  print_disass(PC, con);
}


/*--------------------------------------------------------------------
 exec_inst - Called to implement simulator execution of 1 instruction
   at the current PC(program counter) address.
|--------------------------------------------------------------------*/
int cl_xa::exec_inst(void)
{
  t_mem code1;
  uint code;
  int i;
  int operands;

  instPC= PC;

  if (fetch(&code1))
    return(resBREAKPOINT);
  tick(1);

/* the following lookups make for a slow simulation, we will
  figure out how to make it fast later... */

  /* scan to see if its a 1 byte-opcode */
  code = (code1 << 8);
  i= 0;
  while ( ((code & disass_xa[i].mask) != disass_xa[i].code ||
          (!disass_xa[i].is1byte)) /* not a one byte op code */
                    &&
         disass_xa[i].mnemonic != BAD_OPCODE)
    i++;

  if (disass_xa[i].mnemonic == BAD_OPCODE) {
    /* hit the end of the list, must be a 2 or more byte opcode */
    /* fetch another code byte and search the list again */
      //if (fetch(&code2))  ?not sure if break allowed in middle of opcode?
      //  return(resBREAKPOINT);
    code |= fetch();  /* add 2nd opcode */

    i= 0;
    while ((code & disass_xa[i].mask) != disass_xa[i].code &&
           disass_xa[i].mnemonic != BAD_OPCODE)
      i++;
    /* we should have found the opcode by now, if not invalid entry at eol */
  }

  operands = (int)(disass_xa[i].operands);
  switch (disass_xa[i].mnemonic)
  {
    case ADD:
    return inst_ADD(code, operands);
    case ADDC:
    return inst_ADDC(code, operands);
    case ADDS:
    return inst_ADDS(code, operands);
    case AND:
    return inst_AND(code, operands);
    case ANL:
    return inst_ANL(code, operands);
    case ASL:
    return inst_ASL(code, operands);
    case ASR:
    return inst_ASR(code, operands);
    case BCC:
    return inst_BCC(code, operands);
    case BCS:
    return inst_BCS(code, operands);
    case BEQ:
    return inst_BEQ(code, operands);
    case BG:
    return inst_BG(code, operands);
    case BGE:
    return inst_BGE(code, operands);
    case BGT:
    return inst_BGT(code, operands);
    case BKPT:
    return inst_BKPT(code, operands);
    case BL:
    return inst_BL(code, operands);
    case BLE:
    return inst_BLE(code, operands);
    case BLT:
    return inst_BLT(code, operands);
    case BMI:
    return inst_BMI(code, operands);
    case BNE:
    return inst_BNE(code, operands);
    case BNV:
    return inst_BNV(code, operands);
    case BOV:
    return inst_BOV(code, operands);
    case BPL:
    return inst_BPL(code, operands);
    case BR:
    return inst_BR(code, operands);
    case CALL:
    return inst_CALL(code, operands);
    case CJNE:
    return inst_CJNE(code, operands);
    case CLR:
    return inst_CLR(code, operands);
    case CMP:
    return inst_CMP(code, operands);
    case CPL:
    return inst_CPL(code, operands);
    case DA:
    return inst_DA(code, operands);
    case DIV_w :
    case DIV_d :
    case DIVU_b:
    case DIVU_w:
    case DIVU_d:
    return inst_DIV(code, operands);
    case DJNZ:
    return inst_DJNZ(code, operands);
    case FCALL:
    return inst_FCALL(code, operands);
    case FJMP:
    return inst_FJMP(code, operands);
    case JB:
    return inst_JB(code, operands);
    case JBC:
    return inst_JBC(code, operands);
    case JMP:
    return inst_JMP(code, operands);
    case JNB:
    return inst_JNB(code, operands);
    case JNZ:
    return inst_JNZ(code, operands);
    case JZ:
    return inst_JZ(code, operands);
    case LEA:
    return inst_LEA(code, operands);
    case LSR:
    return inst_LSR(code, operands);
    case MOV:
    return inst_MOV(code, operands);
    case MOVC:
    return inst_MOVC(code, operands);
    case MOVS:
    return inst_MOVS(code, operands);
    case MOVX:
    return inst_MOVX(code, operands);
    case MUL_w:
    case MULU_b:
    case MULU_w:
    return inst_MUL(code, operands);
    case NEG:
    return inst_NEG(code, operands);
    case NOP:
    return inst_NOP(code, operands);
    case NORM:
    return inst_NORM(code, operands);
    case OR:
    return inst_OR(code, operands);
    case ORL:
    return inst_ORL(code, operands);
    case POP:
    case POPU:
    return inst_POP(code, operands);
    case PUSH:
    case PUSHU:
    return inst_PUSH(code, operands);
    case RESET:
    return inst_RESET(code, operands);
    case RET:
    return inst_RET(code, operands);
    case RETI:
    return inst_RETI(code, operands);
    case RL:
    return inst_RL(code, operands);
    case RLC:
    return inst_RLC(code, operands);
    case RR:
    return inst_RR(code, operands);
    case RRC:
    return inst_RRC(code, operands);
    case SETB:
    return inst_SETB(code, operands);
    case SEXT:
    return inst_SEXT(code, operands);
    case SUB:
    return inst_SUB(code, operands);
    case SUBB:
    return inst_SUBB(code, operands);
    case TRAP:
    return inst_TRAP(code, operands);
    case XCH:
    return inst_XCH(code, operands);
    case XOR:
    return inst_XOR(code, operands);

    case BAD_OPCODE:
    default:
    break;
  }

  /*if (PC)
    PC--;
  else
  PC= get_mem_size(MEM_ROM_ID)-1;*/
  PC= rom->inc_address(PC, -1);
  //tick(-clock_per_cycle());
  sim->stop(resINV_INST);
  return(resINV_INST);
}


/* End of xa.src/xa.cc */
