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

#include "ddconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "i_string.h"

// prj
#include "pobjcl.h"

// sim
#include "simcl.h"

// local
#include "xacl.h"
#include "glob.h"
#include "regsxa.h"

/*
 * Base type of xa controllers
 */

cl_xa::cl_xa(class cl_sim *asim):
  cl_uc(asim)
{
  type= CPU_XA;
}

int
cl_xa::init(void)
{
  cl_uc::init(); /* Memories now exist */
  ram= address_space(MEM_XRAM_ID);
  rom= address_space(MEM_ROM_ID);

  /* set SCR to osc/4, native XA mode, flat 24 */
  set_scr(0);
  /* initialize SP to 100H */
  set_reg2(7, 0x100);
  /* set PSW from reset vector */
  set_psw(getcode2(0));
  /* set PC from reset vector */
  PC = getcode2(2);

  printf("The XA Simulator is in development, UNSTABLE, DEVELOPERS ONLY!\n");

  return(0);
}

/*
class cl_m *
cl_xa::mk_mem(enum mem_class type, char *class_name)
{
  class cl_m *m= cl_uc::mk_mem(type, class_name);
  if (type == MEM_SFR)
    sfr= m;
  if (type == MEM_IRAM)
    iram= m;
  return(m);
}
*/

char *
cl_xa::id_string(void)
{
  return("unspecified XA");
}


/*
 * Making elements of the controller
 */
/*
t_addr
cl_xa::get_mem_size(enum mem_class type)
{
  switch(type)
    {
    case MEM_IRAM: return(0x2000);
    case MEM_SFR:  return(0x2000);
    case MEM_ROM:  return(0x10000);
    case MEM_XRAM: return(0x10000);
    default: return(0);
    }
 return(cl_uc::get_mem_size(type));
}
*/

void
cl_xa::mk_hw_elements(void)
{
  //class cl_base *o;
  /* t_uc::mk_hw() does nothing */
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
  if (!get_name(addr, sfr_tbl(), dir_name)) {
    sprintf (dir_name, "0x%03x", addr);
  }
  return dir_name;
}

static char bit_name[64];
char *cl_xa::get_bit_name(short addr) {
  if (!get_name(addr, bit_tbl(), bit_name)) {
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
  int len = 0;
  int immed_n = 0;
  int i;
  int start_addr = addr;

  code= get_mem(MEM_ROM_ID, addr++);
  if (code == 0x00) {
    i= 0;
    while (disass_xa[i].mnemonic != NOP)
      i++;
  } else {
    len = 2;
    code = (code << 8) | get_mem(MEM_ROM_ID, addr++);
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

static char *w_reg_strs[] = {
 "R0", "R1",
 "R2", "R3",
 "R4", "R5",
 "R6", "R7",
 "R8", "R9",
 "R10", "R11",
 "R12", "R13",
 "R14", "R15"};

static char *b_reg_strs[] = {
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
const char *
cl_xa::disass(t_addr addr, const char *sep)
{
  char work[256], parm_str[40];
  char *buf, *p, *b;
  int code;
  int len = 0;
  int immed_offset = 0;
  int operands;
  int mnemonic;
  char **reg_strs;

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
              get_mem(MEM_ROM_ID, addr+immed_offset));
      ++immed_offset;
    break;
    case IREGOFF8_REG :
      sprintf(parm_str, "[%s+%02x],%s",
              w_reg_strs[(code & 0x7)],
              get_mem(MEM_ROM_ID, addr+immed_offset),
              reg_strs[((code >> 4) & 0xf)] );
      ++immed_offset;
    break;
    case REG_IREGOFF16 :
      sprintf(parm_str, "%s,[%s+%04x]",
              reg_strs[((code >> 4) & 0xf)],
              w_reg_strs[(code & 0x7)],
              (short)((get_mem(MEM_ROM_ID, addr+immed_offset+1)) |
                     (get_mem(MEM_ROM_ID, addr+immed_offset)<<8)) );
      ++immed_offset;
      ++immed_offset;
    break;
    case IREGOFF16_REG :
      sprintf(parm_str, "[%s+%04x],%s",
              w_reg_strs[(code & 0x7)],
              (short)((get_mem(MEM_ROM_ID, addr+immed_offset+1)) |
                     (get_mem(MEM_ROM_ID, addr+immed_offset)<<8)),
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
                           get_mem(MEM_ROM_ID, addr+immed_offset)),
              reg_strs[((code >> 4) & 0xf)] );
      ++immed_offset;
    break;
    case REG_DIRECT :
      sprintf(parm_str, "%s,%s",
              reg_strs[((code >> 4) & 0xf)],
              get_dir_name(((code & 0x7) << 8) |
                           get_mem(MEM_ROM_ID, addr+immed_offset)));
      ++immed_offset;
    break;
    case REG_DATA8 :
      sprintf(parm_str, "%s,#0x%02x",
              b_reg_strs[((code >> 4) & 0xf)],
              get_mem(MEM_ROM_ID, addr+immed_offset) );
      ++immed_offset;
    break;
    case REG_DATA16 :
      sprintf(parm_str, "%s,#0x%04x",
              reg_strs[((code >> 4) & 0xf)],
              (short)((get_mem(MEM_ROM_ID, addr+immed_offset+1)) |
                     (get_mem(MEM_ROM_ID, addr+immed_offset)<<8)) );
      ++immed_offset;
      ++immed_offset;
    break;
    case IREG_DATA8 :
      sprintf(parm_str, "[%s], 0x%02x",
              w_reg_strs[((code >> 4) & 0x7)],
              get_mem(MEM_ROM_ID, addr+immed_offset) );
      ++immed_offset;
    break;
    case IREG_DATA16 :
      sprintf(parm_str, "[%s], 0x%04x",
              w_reg_strs[((code >> 4) & 0x7)],
              (short)((get_mem(MEM_ROM_ID, addr+immed_offset+1)) |
                     (get_mem(MEM_ROM_ID, addr+immed_offset)<<8)) );
      ++immed_offset;
      ++immed_offset;
    break;
    case IREGINC_DATA8 :
      sprintf(parm_str, "[%s+], 0x%02x",
              w_reg_strs[((code >> 4) & 0x7)],
              get_mem(MEM_ROM_ID, addr+immed_offset) );
      ++immed_offset;
    break;
    case IREGINC_DATA16 :
      sprintf(parm_str, "[%s+], 0x%04x",
              w_reg_strs[((code >> 4) & 0x7)],
              (short)((get_mem(MEM_ROM_ID, addr+immed_offset+1)) |
                     (get_mem(MEM_ROM_ID, addr+immed_offset)<<8)) );
      ++immed_offset;
      ++immed_offset;
    break;
    case IREGOFF8_DATA8 :
      sprintf(parm_str, "[%s+%02x], 0x%02x",
              w_reg_strs[((code >> 4) & 0x7)],
              get_mem(MEM_ROM_ID, addr+immed_offset),
              get_mem(MEM_ROM_ID, addr+immed_offset+1) );
      immed_offset += 2;
    break;
    case IREGOFF8_DATA16 :
      sprintf(parm_str, "[%s+%02x], 0x%04x",
              w_reg_strs[((code >> 4) & 0x7)],
              get_mem(MEM_ROM_ID, addr+immed_offset),
              (short)((get_mem(MEM_ROM_ID, addr+immed_offset+2)) |
                     (get_mem(MEM_ROM_ID, addr+immed_offset+1)<<8)) );
      immed_offset += 3;
    break;
    case IREGOFF16_DATA8 :
      sprintf(parm_str, "[%s+%04x], 0x%02x",
              w_reg_strs[((code >> 4) & 0x7)],
              (short)((get_mem(MEM_ROM_ID, addr+immed_offset+1)) |
                     (get_mem(MEM_ROM_ID, addr+immed_offset+0)<<8)),
              get_mem(MEM_ROM_ID, addr+immed_offset+2) );
      immed_offset += 3;
    break;
    case IREGOFF16_DATA16 :
      sprintf(parm_str, "[%s+%04x], 0x%04x",
              w_reg_strs[((code >> 4) & 0x7)],
              (short)((get_mem(MEM_ROM_ID, addr+immed_offset+1)) |
                     (get_mem(MEM_ROM_ID, addr+immed_offset+0)<<8)),
              (short)((get_mem(MEM_ROM_ID, addr+immed_offset+3)) |
                     (get_mem(MEM_ROM_ID, addr+immed_offset+2)<<8)) );
      immed_offset += 4;
    break;
    case DIRECT_DATA8 :
      sprintf(parm_str, "%s,#0x%02x",
              get_dir_name(((code & 0x0070) << 4) |
                           get_mem(MEM_ROM_ID, addr+immed_offset)),
              get_mem(MEM_ROM_ID, addr+immed_offset+1));
      immed_offset += 3;
    break;
    case DIRECT_DATA16 :
      sprintf(parm_str, "%s,#0x%04x",
              get_dir_name(((code & 0x0070) << 4) |
                           get_mem(MEM_ROM_ID, addr+immed_offset)),
              get_mem(MEM_ROM_ID, addr+immed_offset+2) +
              (get_mem(MEM_ROM_ID, addr+immed_offset+1)<<8));
      immed_offset += 3;
    break;

// odd-ball ones
    case NO_OPERANDS :  // for NOP
      strcpy(parm_str, "");
    break;
    case CY_BIT :
      sprintf(parm_str, "C,%s",
             get_bit_name(((code&0x0003)<<8) + get_mem(MEM_ROM_ID, addr+2)));
    break;
    case BIT_CY :
      sprintf(parm_str, "%s,C",
              get_bit_name(((code&0x0003)<<8) + get_mem(MEM_ROM_ID, addr+2)));
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
                           get_mem(MEM_ROM_ID, addr+2)),
              code&0x0f);
    break;
    case DIRECT :
      sprintf(parm_str, "%s",
              get_dir_name(((code & 0x007) << 4) +
                           get_mem(MEM_ROM_ID, addr+2)));
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
              get_bit_name(((code&0x0003)<<8) + get_mem(MEM_ROM_ID, addr+2)));
    break;
    case BIT_REL8 :
      sprintf(parm_str, "%s,0x%04x",
              get_bit_name((code&0x0003)<<8) + get_mem(MEM_ROM_ID, addr+2),
              ((signed char)get_mem(MEM_ROM_ID, addr+3)*2+addr+len)&0xfffe);
    break;
    case DATA4:
      sprintf(parm_str, "#0x%02x", code&0x0f);
      break;
    case ADDR24 :
      sprintf(parm_str, "0x%06x",
             (get_mem(MEM_ROM_ID, addr+3)<<16) +
             (get_mem(MEM_ROM_ID, addr+1)<<8) +
             get_mem(MEM_ROM_ID, addr+2));
      break;
    break;
    case REG_REL8 :
      sprintf(parm_str, "%s,0x%04x",
              reg_strs[(code>>4) & 0xf],
              ((signed char)get_mem(MEM_ROM_ID, addr+2)*2+addr+len)&0xfffe);
    break;
    case DIRECT_REL8 :
      sprintf(parm_str, "%s,0x%04x",
              get_dir_name(((code&0x07)<<8) +
                           get_mem(MEM_ROM_ID, addr+2)),
              ((signed char)get_mem(MEM_ROM_ID, addr+2)*2+addr+len)&0xfffe);
    break;
    case REG_USP:
      sprintf(parm_str, "REG_USP");
    break;
    case USP_REG:
      sprintf(parm_str, "USP_REG");
    break;
    case REL8 :
      sprintf(parm_str, "0x%04x",
              ((signed char)get_mem(MEM_ROM_ID, addr+1)*2+addr+len)&0xfffe);
    break;
    case REL16 :
      sprintf(parm_str, "0x%04x",
              ((signed short)((get_mem(MEM_ROM_ID, addr+1)<<8) + get_mem(MEM_ROM_ID, addr+2))*2+addr+len)&0xfffe);
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
                           get_mem(MEM_ROM_ID, addr+immed_offset)),
              ((signed char) get_mem(MEM_ROM_ID, addr+immed_offset+1) * 2) & 0xfffe );
    break;
    case REG_DATA8_REL8 :
      sprintf(parm_str, "%s,#0x%02x,0x%02x",
              reg_strs[((code >> 4) & 0xf)],
              get_mem(MEM_ROM_ID, addr+immed_offset+1),
              ((signed char)get_mem(MEM_ROM_ID, addr+immed_offset) * 2) & 0xfffe );
    break;
    case REG_DATA16_REL8 :
      sprintf(parm_str, "%s,#0x%04x,0x%02x",
              w_reg_strs[(code >> 4) & 0xf],
              get_mem(MEM_ROM_ID, addr+immed_offset+2) +
                 (get_mem(MEM_ROM_ID, addr+immed_offset+1)<<8),
              ((signed char)get_mem(MEM_ROM_ID, addr+immed_offset) * 2) & 0xfffe );
    break;
    case IREG_DATA8_REL8 :
      sprintf(parm_str, "[%s],#0x%02x,0x%02x",
              reg_strs[((code >> 4) & 0x7)],
              get_mem(MEM_ROM_ID, addr+immed_offset+1),
              ((signed char)get_mem(MEM_ROM_ID, addr+immed_offset) * 2) & 0xfffe );
    break;
    case IREG_DATA16_REL8 :
      sprintf(parm_str, "[%s],#0x%04x,0x%02x",
              w_reg_strs[(code >> 4) & 0x7],
              get_mem(MEM_ROM_ID, addr+immed_offset+2) +
                (get_mem(MEM_ROM_ID, addr+immed_offset+1)<<8),
              ((signed char)get_mem(MEM_ROM_ID, addr+immed_offset) * 2) & 0xfffe );
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
              get_mem(MEM_ROM_ID, addr+immed_offset));
      break;

    case REG_REGOFF16 :
      sprintf(parm_str, "%s,%s+0x%02x",
              w_reg_strs[(code >> 4) & 0x7],
              w_reg_strs[code & 0x7],
              get_mem(MEM_ROM_ID, addr+immed_offset+1) +
                (get_mem(MEM_ROM_ID, addr+immed_offset+0)<<8));
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
