/*
 * Simulator of microcontrollers (z80.cc)
 *
 * some z80 code base from Karl Bongers karl@turbobit.com
 *
 * Copyright (C) 1999 Drotos Daniel
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

//#include "ddconfig.h"

//#include <stdarg.h> /* for va_list */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// prj
//#include "pobjcl.h"
#include "globals.h"
#include "utils.h"

// sim
//#include "simcl.h"
#include "dregcl.h"
#include "pc16550cl.h"

// local
#include "z80cl.h"
#include "glob.h"
//#include "regsz80.h"
#include "z80mac.h"

//#define uint32 t_addr
//#define uint8 unsigned char

/*******************************************************************/


/*
 * Base type of Z80 controllers
 */

cl_z80::cl_z80(struct cpu_entry *Itype, class cl_sim *asim):
  cl_uc(asim)
{
  type= Itype;
  BIT_C= 0x01;  // carry status(out of bit 7)
  BIT_N= 0x02,  // Not addition: subtract status(1 after subtract).
  BIT_P= 0x04,  // parity/overflow, 1=even, 0=odd parity.  arith:1=overflow
  BIT_A= 0x10,  // aux carry status(out of bit 3)
  BIT_Z= 0x40,  // zero status, 1=zero, 0=nonzero
  BIT_S= 0x80,  // sign status(value of bit 7)
  BIT_ALL= (BIT_C |BIT_N |BIT_P |BIT_A |BIT_Z |BIT_S);
}

int
cl_z80::init(void)
{

  cl_uc::init(); /* Memories now exist */

  //rom= address_space(MEM_ROM_ID);
//  ram= mem(MEM_XRAM);
  //ram= rom;

  // zero out ram(this is assumed in regression tests)
  for (int i=0x8000; i<0x10000; i++) {
    ram->set((t_addr) i, 0);
  }
  sp_limit= 0xf000;
  iblock= false;

  ttab_00= z80_ttab_00;
  ttab_dd= z80_ttab_dd;
  ttab_cb= z80_ttab_cb;
  ttab_ed= z80_ttab_ed;
  ttab_fd= z80_ttab_fd;
  ttab_ddcb= z80_ttab_ddcb;
  ttab_fdcb= z80_ttab_fdcb;
  
  return(0);
}

void
cl_z80::reset(void)
{
  cl_uc::reset();
  regs.SP= 0xffff;
  regs.AF= 0xffff;
  IFF1= false;
  IFF2= false;
  imode= 0;
}

const char *
cl_z80::id_string(void)
{
  switch (type->type)
    {
    case CPU_Z80: return "Z80";
    case CPU_Z180: return "Z180";
    case CPU_Z80N: return "Z80N";
    default:
      return "unspecified Z80";
    }
  return "Z80";
}


/*
 * Making elements of the controller
 */
/*
t_addr
cl_z80::get_mem_size(enum mem_class type)
{
  switch(type)
    {
    case MEM_ROM: return(0x10000);
    case MEM_XRAM: return(0x10000);
    default: return(0);
    }
 return(cl_uc::get_mem_size(type));
}
*/

void
cl_z80::make_cpu_hw(void)
{
  add_hw(cpu= new cl_z80_cpu(this));
  cpu->init();
}

void
cl_z80::mk_hw_elements(void)
{
  //class cl_base *o;
  class cl_hw *h;
  cl_uc::mk_hw_elements();

  add_hw(h= new cl_dreg(this, 0, "dreg"));
  h->init();

  add_hw(h= new cl_pc16550(this, 0));
  h->init();
}

void
cl_z80::make_memories(void)
{
  class cl_address_space *as;

  rom= ram= as= new cl_address_space("rom", 0, 0x10000, 8);
  as->init();
  address_spaces->add(as);

  class cl_address_decoder *ad;
  class cl_memory_chip *chip;

  chip= new cl_chip8("rom_chip", 0x10000, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as= address_space("rom"), chip, 0, 0xffff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  inputs= new cl_address_space("inputs", 0, 0x10000, 8);
  inputs->init();
  chip= new cl_chip8("in_chip", 0x10000, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(inputs, chip, 0, 0xffff, 0);
  ad->init();
  inputs->decoders->add(ad);
  address_spaces->add(inputs);
  ad->activate(0);
  outputs= new cl_address_space("outputs", 0, 0x10000, 8);
  outputs->init();
  chip= new cl_chip8("out_chip", 0x10000, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(outputs, chip, 0, 0xffff, 0);
  ad->init();
  outputs->decoders->add(ad);
  address_spaces->add(outputs);
  ad->activate(0);
  
  regs8= new cl_address_space("regs8", 0, 18, 8);
  regs8->init();
  regs8->get_cell(0)->decode((t_mem*)&regs.raf.A);
  regs8->get_cell(1)->decode((t_mem*)&regs.raf.F);
  regs8->get_cell(2)->decode((t_mem*)&regs.bc.h);
  regs8->get_cell(3)->decode((t_mem*)&regs.bc.l);
  regs8->get_cell(4)->decode((t_mem*)&regs.de.h);
  regs8->get_cell(5)->decode((t_mem*)&regs.de.l);
  regs8->get_cell(6)->decode((t_mem*)&regs.hl.h);
  regs8->get_cell(7)->decode((t_mem*)&regs.hl.l);

  regs8->get_cell(8)->decode((t_mem*)&regs.ralt_af.aA);
  regs8->get_cell(9)->decode((t_mem*)&regs.ralt_af.aF);
  regs8->get_cell(10)->decode((t_mem*)&regs.a_bc.h);
  regs8->get_cell(11)->decode((t_mem*)&regs.a_bc.l);
  regs8->get_cell(12)->decode((t_mem*)&regs.a_de.h);
  regs8->get_cell(13)->decode((t_mem*)&regs.a_de.l);
  regs8->get_cell(14)->decode((t_mem*)&regs.a_hl.h);
  regs8->get_cell(15)->decode((t_mem*)&regs.a_hl.l);

  regs8->get_cell(16)->decode((t_mem*)&regs.R);
  regs8->get_cell(17)->decode((t_mem*)&regs.iv);
  
  regs16= new cl_address_space("regs16", 0, 11, 16);
  regs16->init();

  regs16->get_cell(0)->decode((t_mem*)&regs.AF);
  regs16->get_cell(1)->decode((t_mem*)&regs.BC);
  regs16->get_cell(2)->decode((t_mem*)&regs.DE);
  regs16->get_cell(3)->decode((t_mem*)&regs.HL);
  regs16->get_cell(4)->decode((t_mem*)&regs.IX);
  regs16->get_cell(5)->decode((t_mem*)&regs.IY);
  regs16->get_cell(6)->decode((t_mem*)&regs.SP);
  regs16->get_cell(7)->decode((t_mem*)&regs.aAF);
  regs16->get_cell(8)->decode((t_mem*)&regs.aBC);
  regs16->get_cell(9)->decode((t_mem*)&regs.aDE);
  regs16->get_cell(10)->decode((t_mem*)&regs.aHL);

  address_spaces->add(regs8);
  address_spaces->add(regs16);

  vars->add("A", regs8, 0, 7, 0, "Accumulator");
  vars->add("F", regs8, 1, 7, 0, "Flags");
  /*
  flag bit positions are family dependent
  vars->add("F_C", regs8, 1, BITPOS_C, BITPOS_C, "Carry");
  vars->add("F_SUB", regs8, 1, BITPOS_SUB, BITPOS_SUB, "");
  vars->add("F_P", regs8, 1, BITPOS_P, BITPOS_P, "");
  vars->add("F_A", regs8, 1, BITPOS_A, BITPOS_A, "");
  vars->add("F_Z", regs8, 1, BITPOS_Z, BITPOS_Z, "Zero");
  vars->add("F_S", regs8, 1, BITPOS_S, BITPOS_S, "");
  */
  vars->add("B", regs8, 2, 7, 0, "B register");
  vars->add("C", regs8, 3, 7, 0, "C register");
  vars->add("D", regs8, 4, 7, 0, "D register");
  vars->add("E", regs8, 5, 7, 0, "E register");
  vars->add("H", regs8, 6, 7, 0, "H register");
  vars->add("L", regs8, 7, 7, 0, "L register");

  vars->add("ALT_A", regs8, 8, 7, 0, "Alt Accumulator");
  vars->add("ALT_F", regs8, 9, 7, 0, "Alt Flags");
  /*
  flag bit positions are family dependent
  vars->add("ALT_F_C", regs8, 9, BITPOS_C, BITPOS_C, "Carry");
  vars->add("ALT_F_SUB", regs8, 9, BITPOS_SUB, BITPOS_SUB, "");
  vars->add("ALT_F_P", regs8, 9, BITPOS_P, BITPOS_P, "");
  vars->add("ALT_F_A", regs8, 9, BITPOS_A, BITPOS_A, "");
  vars->add("ALT_F_Z", regs8, 9, BITPOS_Z, BITPOS_Z, "Zero");
  vars->add("ALT_F_S", regs8, 9, BITPOS_S, BITPOS_S, "");
  */
  vars->add("ALT_B", regs8, 10, 7, 0, "Alt B register");
  vars->add("ALT_C", regs8, 11, 7, 0, "Alt C register");
  vars->add("ALT_D", regs8, 12, 7, 0, "Alt D register");
  vars->add("ALT_E", regs8, 13, 7, 0, "Alt E register");
  vars->add("ALT_H", regs8, 14, 7, 0, "Alt H register");
  vars->add("ALT_L", regs8, 15, 7, 0, "Alt L register");

  vars->add("AF", regs16, 0, 15, 0, "Accumulator/Flags");
  vars->add("BC", regs16, 1, 15, 0, "BC register pair");
  vars->add("DE", regs16, 2, 15, 0, "DE register pair");
  vars->add("HL", regs16, 3, 15, 0, "HL register pair");
  vars->add("IX", regs16, 4, 15, 0, "IX register");
  vars->add("IY", regs16, 5, 15, 0, "IY register");
  vars->add("SP", regs16, 6, 15, 0, "Stack Pointer");
  vars->add("ALT_AF", regs16, 7, 15, 0, "Alt Accumulator/Flags");
  vars->add("ALT_BC", regs16, 8, 15, 0, "Alt BC register pair");
  vars->add("ALT_DE", regs16, 9, 15, 0, "Alt DE register pair");
  vars->add("ALT_HL", regs16, 10, 15, 0, "Alt HL register pair");

  vars->add("R", regs8, 16, "R register");
  vars->add("I", regs8, 17, "I register");
}


/*
 * Help command interpreter
 */

struct dis_entry *
cl_z80::dis_tbl(void)
{
  return(disass_z80);
}

/*struct name_entry *
cl_z80::sfr_tbl(void)
{
  return(0);
}*/

/*struct name_entry *
cl_z80::bit_tbl(void)
{
  //FIXME
  return(0);
}*/

int
cl_z80::inst_length(t_addr addr)
{
  int len = 0;

  get_disasm_info(addr, &len, NULL, NULL, NULL);

  return len;
}

int
cl_z80::inst_branch(t_addr addr)
{
  int b;

  get_disasm_info(addr, NULL, &b, NULL, NULL);

  return b;
}

bool
cl_z80::is_call(t_addr addr)
{
  struct dis_entry *e;

  get_disasm_info(addr, NULL, NULL, NULL, &e);

  return e?(e->is_call):false;
}

int
cl_z80::longest_inst(void)
{
  return 4;
}


const char *
cl_z80::get_disasm_info(t_addr addr,
                        int *ret_len,
                        int *ret_branch,
                        int *immed_offset,
			struct dis_entry **dentry)
{
  const char *b = NULL;
  uint code;
  int len = 0;
  int immed_n = 0;
  int i;
  int start_addr = addr;
  struct dis_entry *dis_e;

  code= rom->get(addr++);
  dis_e = NULL;

  switch(code) {
    case 0xcb:  /* ESC code to lots of op-codes, all 2-byte */
      code= rom->get(addr++);
      i= 0;
      while ((code & disass_z80_cb[i].mask) != disass_z80_cb[i].code &&
        disass_z80_cb[i].mnemonic)
        i++;
      dis_e = &disass_z80_cb[i];
      b= disass_z80_cb[i].mnemonic;
      if (b != NULL)
        len += (disass_z80_cb[i].length + 1);
    break;

    case 0xed: /* ESC code to about 80 opcodes of various lengths */
      code= rom->get(addr++);
      if (type->type == CPU_Z80N)
	{
	  i= 0;
	  while ((code & disass_z80n_ed[i].mask) != disass_z80n_ed[i].code &&
		 disass_z80n_ed[i].mnemonic)
	    i++;
	  b= disass_z80n_ed[i].mnemonic;
	  if (b != NULL)
	    {
	      len += (disass_z80n_ed[i].length + 1);
	      break;
	    }
	}
      i= 0;
      while ((code & disass_z80_ed[i].mask) != disass_z80_ed[i].code &&
        disass_z80_ed[i].mnemonic)
        i++;
      dis_e = &disass_z80_ed[i];
      b= disass_z80_ed[i].mnemonic;
      if (b != NULL)
        len += (disass_z80_ed[i].length + 1);
    break;

    case 0xdd: /* ESC codes,about 284, vary lengths, IX centric */
      code= rom->get(addr++);
      if (code == 0xcb) {
        immed_n = 2;
        addr++;  // pass up immed data
        code= rom->get(addr++);
        i= 0;
        while ((code & disass_z80_ddcb[i].mask) != disass_z80_ddcb[i].code &&
          disass_z80_ddcb[i].mnemonic)
          i++;
        dis_e = &disass_z80_ddcb[i];
        b= disass_z80_ddcb[i].mnemonic;
        if (b != NULL)
          len += (disass_z80_ddcb[i].length + 2);
      } else {
        i= 0;
        while ((code & disass_z80_dd[i].mask) != disass_z80_dd[i].code &&
          disass_z80_dd[i].mnemonic)
          i++;
        dis_e = &disass_z80_dd[i];
        b= disass_z80_dd[i].mnemonic;
        if (b != NULL)
          len += (disass_z80_dd[i].length + 1);
      }
    break;

    case 0xfd: /* ESC codes,sme as dd but IY centric */
      code= rom->get(addr++);
      if (code == 0xcb) {
        immed_n = 2;
        addr++;  // pass up immed data
        code= rom->get(addr++);
        i= 0;
        while ((code & disass_z80_fdcb[i].mask) != disass_z80_fdcb[i].code &&
          disass_z80_fdcb[i].mnemonic)
          i++;
        dis_e = &disass_z80_fdcb[i];
        b= disass_z80_fdcb[i].mnemonic;
        if (b != NULL)
          len += (disass_z80_fdcb[i].length + 2);
      } else {
        i= 0;
        while ((code & disass_z80_fd[i].mask) != disass_z80_fd[i].code &&
          disass_z80_fd[i].mnemonic)
          i++;
        dis_e = &disass_z80_fd[i];
        b= disass_z80_fd[i].mnemonic;
        if (b != NULL)
          len += (disass_z80_fd[i].length + 1);
      }
    break;

    default:
      i= 0;
      while ((code & disass_z80[i].mask) != disass_z80[i].code &&
             disass_z80[i].mnemonic)
        i++;
      dis_e = &disass_z80[i];
      b= disass_z80[i].mnemonic;
      if (b != NULL)
        len += (disass_z80[i].length);
    break;
  }


  if (ret_branch) {
    *ret_branch = dis_e->branch;
  }

  if (immed_offset) {
    if (immed_n > 0)
         *immed_offset = immed_n;
    else *immed_offset = (addr - start_addr);
  }

  if (len == 0)
    len = 1;

  if (ret_len)
    *ret_len = len;

  if (dentry)
    *dentry= dis_e;
  
  return b;
}

char *
cl_z80::disass(t_addr addr)
{
  chars work, temp;
  const char *b;
  int len = 0;
  int immed_offset = 0;
  bool first= true;
  
  work= "";

  b = get_disasm_info(addr, &len, NULL, &immed_offset, NULL);

  if (b == NULL)
    {
      return strdup("UNKNOWN/INVALID");
    }

  while (*b)
    {
      if ((*b == ' ') && first)
	{
	  first= false;
	  while (work.len() < 6) work.append(' ');
	}
      if (*b == '%')
        {
          b++;
	  temp= "";
          switch (*(b++))
            {
            case 'd': // jump relative target, signed? byte immediate operand
              temp.format("%d", (signed char)(rom->get(addr+immed_offset)));
              ++immed_offset;
              break;
            case 'w': // word immediate operand, little endian
              temp.format("0x%04x",
			  (uint)((rom->get(addr+immed_offset)) |
				 (rom->get(addr+immed_offset+1)<<8)) );
              ++immed_offset;
              ++immed_offset;
              break;
            case 'W': // word immediate operand, big endian
              temp.format("0x%04x",
			  (uint)((rom->get(addr+immed_offset)<<8) |
				 (rom->get(addr+immed_offset+1))) );
              ++immed_offset;
              ++immed_offset;
              break;
            case 'b': // byte immediate operand
              temp.format("0x%02x", (uint)rom->get(addr+immed_offset));
              ++immed_offset;
              break;
            default:
              temp= "?";
              break;
            }
          work+= temp;
        }
      else
        work+= *(b++);
    }

  return strdup(work.c_str());
}


void
cl_z80::print_regs(class cl_console_base *con)
{
  con->dd_color("answer");
  con->dd_printf("SZ-A-PNC  Flags= 0x%02x %3d %c  ",
                 regs.raf.F, regs.raf.F, isprint(regs.raf.F)?regs.raf.F:'.');
  con->dd_printf("A= 0x%02x %3d %c\n",
                 regs.raf.A, regs.raf.A, isprint(regs.raf.A)?regs.raf.A:'.');
  con->dd_printf("%s\n", cbin(regs.raf.F, 8).c_str());
  con->dd_printf("BC= 0x%04x [BC]= %02x %3d %c  ",
                 regs.BC, ram->get(regs.BC), ram->get(regs.BC),
                 isprint(ram->get(regs.BC))?ram->get(regs.BC):'.');
  con->dd_printf("DE= 0x%04x [DE]= %02x %3d %c  ",
                 regs.DE, ram->get(regs.DE), ram->get(regs.DE),
                 isprint(ram->get(regs.DE))?ram->get(regs.DE):'.');
  con->dd_printf("HL= 0x%04x [HL]= %02x %3d %c\n",
                 regs.HL, ram->get(regs.HL), ram->get(regs.HL),
                 isprint(ram->get(regs.HL))?ram->get(regs.HL):'.');
  con->dd_printf("IX= 0x%04x [IX]= %02x %3d %c  ",
                 regs.IX, ram->get(regs.IX), ram->get(regs.IX),
                 isprint(ram->get(regs.IX))?ram->get(regs.IX):'.');
  con->dd_printf("IY= 0x%04x [IY]= %02x %3d %c  ",
                 regs.IY, ram->get(regs.IY), ram->get(regs.IY),
                 isprint(ram->get(regs.IY))?ram->get(regs.IY):'.');
  con->dd_printf("AF= 0x%04x [AF]= %02x %3d %c\n",
                 regs.AF, ram->get(regs.AF), ram->get(regs.AF),
                 isprint(ram->get(regs.AF))?ram->get(regs.AF):'.');
  con->dd_printf("SP limit= 0x%04x\n", AU(sp_limit));
  
  /*con->dd_printf("SP= 0x%04x ",
                 regs.SP, ram->get(regs.SP), ram->get(regs.SP),
                 isprint(ram->get(regs.SP))?ram->get(regs.SP):'.');
  */
  int i;
  con->dd_cprintf("answer", "SP= ");
  con->dd_cprintf("dump_address", "0x%04x ->", regs.SP);
  for (i= 0; i < 2*12; i+= 2)
    {
      t_addr al, ah;
      al= (regs.SP+i)&0xffff;
      ah= (al+1)&0xffff;
      con->dd_cprintf("dump_number", " %02x%02x",
		      (u8_t)(ram->read(al)),
		      (u8_t)(ram->read(ah)));
    }
  con->dd_printf("\n");
  
  print_disass(PC, con);
}

/*
 * Execution
 */

int
cl_z80::exec_inst(void)
{
  t_mem code;
  int res= -1;
  
  instPC= PC;

  if (fetch(&code))
    return(resBREAKPOINT);
  tick(1);
  inc_R();
  iblock= false;
  cond_true= false;
  
  switch (code)
    {
    case 0x00: res= (inst_nop(code)); break;
    case 0x01: case 0x02: case 0x06: res= (inst_ld(code)); break;
    case 0x03: case 0x04: res= (inst_inc(code)); break;
    case 0x05: res= (inst_dec(code)); break;
    case 0x07: res= (inst_rlca(code)); break;

    case 0x08: res= (inst_ex(code)); break;
    case 0x09: res= (inst_add(code)); break;
    case 0x0a: case 0x0e: res= (inst_ld(code)); break;
    case 0x0b: case 0x0d: res= (inst_dec(code)); break;
    case 0x0c: res= (inst_inc(code)); break;
    case 0x0f: res= (inst_rrca(code)); break;


    case 0x10: res= (inst_djnz(code)); break;
    case 0x11: case 0x12: case 0x16: res= (inst_ld(code)); break;
    case 0x13: case 0x14: res= (inst_inc(code)); break;
    case 0x15: res= (inst_dec(code)); break;
    case 0x17: res= (inst_rla(code)); break;

    case 0x18: res= (inst_jr(code)); break;
    case 0x19: res= (inst_add(code)); break;
    case 0x1a: case 0x1e: res= (inst_ld(code)); break;
    case 0x1b: case 0x1d: res= (inst_dec(code)); break;
    case 0x1c: res= (inst_inc(code)); break;
    case 0x1f: res= (inst_rra(code)); break;


    case 0x20: res= (inst_jr(code)); break;
    case 0x21: case 0x22: case 0x26: res= (inst_ld(code)); break;
    case 0x23: case 0x24: res= (inst_inc(code)); break;
    case 0x25: res= (inst_dec(code)); break;
    case 0x27: res= (inst_daa(code)); break;

    case 0x28: res= (inst_jr(code)); break;
    case 0x29: res= (inst_add(code)); break;
    case 0x2a: case 0x2e: res= (inst_ld(code)); break;
    case 0x2b: case 0x2d: res= (inst_dec(code)); break;
    case 0x2c: res= (inst_inc(code)); break;
    case 0x2f: res= (inst_cpl(code)); break;


    case 0x30: res= (inst_jr(code)); break;
    case 0x31: case 0x32: case 0x36: res= (inst_ld(code)); break;
    case 0x33: case 0x34: res= (inst_inc(code)); break;
    case 0x35: res= (inst_dec(code)); break;
    case 0x37: res= (inst_scf(code)); break;

    case 0x38: res= (inst_jr(code)); break;
    case 0x39: res= (inst_add(code)); break;
    case 0x3a: case 0x3e: res= (inst_ld(code)); break;
    case 0x3b: case 0x3d: res= (inst_dec(code)); break;
    case 0x3c: res= (inst_inc(code)); break;
    case 0x3f: res= (inst_ccf(code)); break;

    case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
    case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
      res= (inst_ld(code)); break;

    case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
    case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
      res= (inst_ld(code)); break;

    case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
    case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
      res= (inst_ld(code)); break;

    case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77:
    case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
      res= (inst_ld(code)); break;
    case 0x76:
      res= (inst_halt(code)); break;

    case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
      res= (inst_add(code)); break;
    case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
      res= (inst_adc(code)); break;

    case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
      res= (inst_sub(code)); break;
    case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
      res= (inst_sbc(code)); break;

    case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
      res= (inst_and(code)); break;
    case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
      res= (inst_xor(code)); break;

    case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
      res= (inst_or(code)); break;
    case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
      res= (inst_cp(code)); break;

    case 0xc0: res= (inst_ret(code)); break;
    case 0xc1: res= (inst_pop(code)); break;
    case 0xc2: case 0xc3: res= (inst_jp(code)); break;
    case 0xc4: res= (inst_call(code)); break;
    case 0xc5: res= (inst_push(code)); break;
    case 0xc6: res= (inst_add(code)); break;
    case 0xc7: res= (inst_rst(code)); break;

    case 0xc8: case 0xc9: res= (inst_ret(code)); break;
    case 0xca: res= (inst_jp(code)); break;

      /* CB escapes out to 2 byte opcodes(CB include), opcodes
         to do register bit manipulations */
    case 0xcb: res= (inst_cb()); break;
    case 0xcc: case 0xcd: res= (inst_call(code)); break;
    case 0xce: res= (inst_adc(code)); break;
    case 0xcf: res= (inst_rst(code)); break;


    case 0xd0: res= (inst_ret(code)); break;
    case 0xd1: res= (inst_pop(code)); break;
    case 0xd2: res= (inst_jp(code)); break;
    case 0xd3: res= (inst_out(code)); break;
    case 0xd4: res= (inst_call(code)); break;
    case 0xd5: res= (inst_push(code)); break;
    case 0xd6: res= (inst_sub(code)); break;
    case 0xd7: res= (inst_rst(code)); break;

    case 0xd8: res= (inst_ret(code)); break;
    case 0xd9: res= (inst_exx(code)); break;
    case 0xda: res= (inst_jp(code)); break;
    case 0xdb: res= (inst_in(code)); break;
    case 0xdc: res= (inst_call(code)); break;
      /* DD escapes out to 2 to 4 byte opcodes(DD included)
        with a variety of uses.  It can precede the CB escape
        sequence to extend CB codes with IX+immed_byte */
    case 0xdd: res= (inst_dd(0xdd)); break;
    case 0xde: res= (inst_sbc(code)); break;
    case 0xdf: res= (inst_rst(code)); break;


    case 0xe0: res= (inst_ret(code)); break;
    case 0xe1: res= (inst_pop(code)); break;
    case 0xe2: res= (inst_jp(code)); break;
    case 0xe3: res= (inst_ex(code)); break;
    case 0xe4: res= (inst_call(code)); break;
    case 0xe5: res= (inst_push(code)); break;
    case 0xe6: res= (inst_and(code)); break;
    case 0xe7: res= (inst_rst(code)); break;

    case 0xe8: res= (inst_ret(code)); break;
    case 0xe9: res= (inst_jp(code)); break;
    case 0xea: res= (inst_jp(code)); break;
    case 0xeb: res= (inst_ex(code)); break;
    case 0xec: res= (inst_call(code)); break;
      /* ED escapes out to misc IN, OUT and other oddball opcodes */
    case 0xed: res= (inst_ed(0xed)); break;
    case 0xee: res= (inst_xor(code)); break;
    case 0xef: res= (inst_rst(code)); break;


    case 0xf0: res= (inst_ret(code)); break;
    case 0xf1: res= (inst_pop(code)); break;
    case 0xf2: res= (inst_jp(code)); break;
    case 0xf3: res= (inst_di(code)); break;
    case 0xf4: res= (inst_call(code)); break;
    case 0xf5: res= (inst_push(code)); break;
    case 0xf6: res= (inst_or(code)); break;
    case 0xf7: res= (inst_rst(code)); break;

    case 0xf8: res= (inst_ret(code)); break;
    case 0xf9: res= (inst_ld(code)); break;
    case 0xfa: res= (inst_jp(code)); break;
    case 0xfb: res= (inst_ei(code)); break;
    case 0xfc: res= (inst_call(code)); break;
      /* DD escapes out to 2 to 4 byte opcodes(DD included)
        with a variety of uses.  It can precede the CB escape
        sequence to extend CB codes with IX+immed_byte */
    case 0xfd: res= (inst_fd(0xfd)); break;
    case 0xfe: res= (inst_cp(code)); break;
    case 0xff: res= (inst_rst(code)); break;
    }

  tickt(code);
  
  if (res >= 0)
    return res;
  return resINV_INST;
}

int
cl_z80::tickt(t_mem code)
{
  // special cases: dd, cb, ed, ddcb, fd, fdcb
  u16_t t= 0;
  u8_t c2, c3, c4;
  switch (code)
    {
    case 0xdd:
      c2= rom->read(instPC+1);
      if (c2 == 0xcb)
	{
	  c4= rom->read(instPC+3);
	  t= ttab_ddcb[c4];
	  break;
	}
      t= ttab_dd[c2];
      break;
    case 0xfd:
      c2= rom->read(instPC+1);
      if (c2==0xcb)
	{
	  c4= rom->read(instPC+3);
	  t= ttab_fdcb[c4];
	  break;
	}
      t= ttab_fd[c2];
      break;
    case 0xcb:
      c2= rom->read(instPC+1);
      t= ttab_cb[c2];
      break;
    case 0xed:
      c2= rom->read(instPC+1);
      t= ttab_ed[c2];
      break;
    default:
      t= ttab_00[code];
      break;
    }
  if (cond_true)
    t>>= 8;
  else
    t&= 0xff;
  tick(t-1);
  return t;
}

bool cl_z80::inst_z80n(t_mem code, int *ret)
{
  int r= resGO;
  switch (code)
    {
    case 0xa4: r= inst_ldix(code); break;
    case 0xa5: // ldws
      {
	this->store1(regs.DE, this->get1(regs.HL));
	vc.rd++;
	vc.wr++;
	inc(regs.hl.l);
	inc(regs.de.h);
	break;
      }
    case 0xb4: do r= inst_ldix(code); while (regs.BC); break;
    case 0xac: r= inst_lddx(code); break;
    case 0xbc: do r= inst_lddx(code); while (regs.BC); break;
    case 0xb7:  // ldpirx
      {
	u8_t t;
	do {
	  t= this->get1((regs.HL & 0xfff8)+(regs.de.l & 7));
	  vc.rd++;
	  if (t != regs.raf.A)
	    {
	      this->store1(regs.DE, t);
	      vc.wr++;
	    }
	  regs.DE++;
	  regs.BC--;
	}
	while (regs.BC);
	break;
      }
    case 0x90: // outinb
      outputs->write(regs.BC, this->get1(regs.HL));
      vc.wr++;
      vc.rd++;
      SET_Z(regs.bc.h);
      regs.raf.F|= BIT_N;
      regs.HL++;
      break;
    case 0x30: // mul
      regs.DE= regs.de.h * regs.de.l;
      break;
    case 0x31: // add hl,a
      regs.HL+= regs.raf.A;
      break;
    case 0x32: // add de,a
      regs.DE+= regs.raf.A;
      break;
    case 0x33: // add bc,a
      regs.BC+= regs.raf.A;
      break;
    case 0x34: // add hl,$nnnn
      {
	u16_t w= fetch2();
	regs.HL+= w;
	break;
      }
    case 0x35: // add de,$nnnn
      {
	u16_t w= fetch2();
	regs.DE+= w;
	break;
      }

    case 0x36: // add bc,$nnnn
      {
	u16_t w= fetch2();
	regs.BC+= w;
	break;
      }
    case 0x23: // swapnib
      regs.raf.A= (regs.raf.A >> 4) | (regs.raf.A << 4);
      break;
    case 0x24: // mirror a
      regs.raf.A=
	((regs.raf.A&0x01)?0x80:0) |
	((regs.raf.A&0x02)?0x40:0) |
	((regs.raf.A&0x04)?0x20:0) |
	((regs.raf.A&0x08)?0x10:0) |
	((regs.raf.A&0x10)?0x08:0) |
	((regs.raf.A&0x20)?0x04:0) |
	((regs.raf.A&0x40)?0x02:0) |
	((regs.raf.A&0x80)?0x01:0);
      break;	
    case 0x8a: // push $nnnn
      {
	u16_t w= fetch() * 256;
	w+= fetch();
	push2(w);
	vc.wr+= 2;
	break;
      }
    case 0x91: // nextreg $rr,$nn
      outputs->write(0x243b, fetch());
      outputs->write(0x253b, fetch());
      vc.wr+= 2;
      break;
    case 0x92: // nextreg $rr,a
      outputs->write(0x243b, fetch());
      outputs->write(0x253b, regs.raf.A);
      vc.wr+= 2;
      break;
    case 0x93: // pixeldn
      if (regs.HL!=0x0700)
	regs.HL+= 256;
      else if ((regs.HL&0xe0)!=0xe0)
	regs.HL= (regs.HL&0xf800)+0x20;
      else
	regs.HL= (regs.HL&0xf81f)+0x0800;
      break;
    case 0x94: // pixelad
      regs.HL= 0x4000 + ((regs.de.h&0xc0)<<5) + ((regs.de.h&0x07)<<8) +
	((regs.de.h&0x38)<<2) + (regs.de.l>>3);
      break;
    case 0x95: // setae
      regs.raf.A= 0x80 >> (regs.de.l & 0x7);
      break;
    case 0x27: // test $nn
      {
	u8_t d= fetch();
	d&= regs.raf.A;
	regs.raf.F &= ~(BIT_ALL);
	SET_Z(d);
	SET_S(d);
	if (parity(d))
	  regs.raf.F|= BIT_P;
	break;
      }
      // core version 2.00.22+
    case 0x28: // bsla de,b
      regs.DE= regs.DE << (regs.bc.h&31);
      break;
    case 0x29: // bsra de,b
      {
	i16_t w= regs.DE;
	w= w >> (regs.bc.h&31);
	regs.DE= w;
	break;
      }
    case 0x2a: // bsrl de,b
      regs.DE= regs.DE >> (regs.bc.h&31);
      break;
    case 0x2b: // bsrf de,b
      {
	u16_t w= ~regs.DE;
	w>>= (regs.bc.h&31);
	regs.DE= ~w;
	break;
      }
    case 0x2c: // brlc de,b
      regs.DE= (regs.DE << (regs.bc.h&15)) | (regs.DE >> (16-(regs.bc.h&15)));
      break;
    case 0x98: // jp (c)
      PC= (PC&0xc000) + (inputs->read(regs.BC)<<6);
      break;
    default: return false;
    }
  if (ret)
    *ret= r;
  return true;
}

// Z80N
int
cl_z80::inst_ldix(t_mem code)
{
  // ldix, -, {if HL*!=A DE*:=HL*;} DE++; HL++; BC--
  u8_t at_hl;
  at_hl= this->get1(regs.HL);
  vc.rd++;
  if (at_hl == regs.raf.A)
    {
      this->store1(regs.DE, at_hl);
      vc.wr++;
    }
  regs.DE++;
  regs.HL++;
  regs.BC--;
  return resGO;
}

void
cl_z80::inc_R()
{
  u8_t r7= regs.R&0x7f;
  r7= (r7+1)&0x7f;
  regs.R= (regs.R&0x80)|r7;
}

// set undocumented flags 3 (x), and 5 (y)
void
cl_z80::xy(u8_t v)
{
  regs.raf.F= (regs.raf.F & ~(0x28)) | (v & 0x28);
}

// Z80N
int
cl_z80::inst_lddx(t_mem code)
{
  // lddx, -, {if HL*!=A DE*:=HL*;} DE++; HL--; BC--
  u8_t at_hl;
  at_hl= this->get1(regs.HL);
  vc.rd++;
  if (at_hl == regs.raf.A)
    {
      this->store1(regs.DE, at_hl);
      vc.wr++;
    }
  regs.DE++;
  regs.HL--;
  regs.BC--;
  return resGO;
}

void cl_z80::store1( u16_t addr, t_mem val ) {
  ram->write(addr, val);
}

void cl_z80::store2( u16_t addr, u16_t val ) {
  ram->write(addr,   val & 0xff);
  ram->write(addr+1, (val >> 8) & 0xff);
}

u8_t  cl_z80::get1( u16_t addr ) {
  return ram->read(addr);
}

u16_t  cl_z80::get2( u16_t addr ) {
  u16_t  l, h;
  
  l = ram->read(addr  );
  h = ram->read(addr+1);
  
  return (h << 8) | l;
}

t_mem       cl_z80::fetch1( void ) {
  return fetch( );
}

u16_t  cl_z80::fetch2( void ) {
  u16_t  c1, c2;
  
  c1 = fetch( );
  c2 = fetch( );
  return (c2 << 8) | c1;
}

t_mem       cl_z80::peek1 ( void ) {
  return rom->read(PC);
}

u8_t  cl_z80:: in_byte( u16_t ioaddr )
{
  return inputs->read(ioaddr);
}

void        cl_z80::out_byte( u16_t ioaddr, u8_t io_val )
{
  outputs->write(ioaddr, io_val);
  return;
}

u8_t  cl_z80::reg_g_read ( t_mem g )
{
  switch( g )
    {
    case 0:  return regs.bc.h;
    case 1:  return regs.bc.l;
    case 2:  return regs.de.h;
    case 3:  return regs.de.l;
    case 4:  return regs.hl.h;
    case 5:  return regs.hl.l;
    case 6:  return get1( regs.HL );
    case 7:  return regs.raf.A;
    default:
      return 0xffU;
    }
}

void        cl_z80::reg_g_store( t_mem g, u8_t new_val )
{
  switch( g )
    {
    case 0:  regs.bc.h = new_val;  break;  /* write to b */
    case 1:  regs.bc.l = new_val;  break;  /* write to c */
    case 2:  regs.de.h = new_val;  break;  /* write to d */
    case 3:  regs.de.l = new_val;  break;  /* write to e */
    case 4:  regs.hl.h = new_val;  break;  /* write to h */
    case 5:  regs.hl.l = new_val;  break;  /* write to l */
    case 6:  /* write to (hl) */
      store1( regs.HL, new_val );
      break;

    case 7:  regs.raf.A    = new_val;  break;  /* write to a */
    }
}

u16_t
cl_z80::regrp_ss_read(u8_t ss)
{
  switch (ss)
    {
    case 0: return regs.BC;
    case 1: return regs.DE;
    case 2: return regs.HL;
      //case 3:
    }
  return regs.SP;
}

void
cl_z80::stack_check_overflow(class cl_stack_op *op)
{
  if (op)
    {
      if (op->get_op() & stack_write_operation)
	{
	  t_addr a= op->get_after();
	  if (a < sp_limit)
	    {
	      class cl_error_stack_overflow *e=
		new cl_error_stack_overflow(op);
	      e->init();
	      error(e);
	    }
	}
    }
}


cl_z80_cpu::cl_z80_cpu(class cl_uc *auc):
  cl_hw(auc, HW_CPU, 0, "cpu")
{
  zuc= (class cl_z80 *)auc;
}

int
cl_z80_cpu::init(void)
{
  cl_hw::init();

  cl_var *v;
  uc->vars->add(v= new cl_var("sp_limit", cfg, z80cpu_sp_limit,
			      cfg_help(z80cpu_sp_limit)));
  v->init();
  return 0;
}

const char *
cl_z80_cpu::cfg_help(t_addr addr)
{
  switch (addr)
    {
    case z80cpu_sp_limit:
      return "Stack overflows when SP is below this limit";
    }
  return "Not used";
}

t_mem
cl_z80_cpu::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  class cl_z80 *u= (class cl_z80 *)uc;
  if (val)
    cell->set(*val);
  switch ((enum z80cpu_confs)addr)
    {
    case z80cpu_sp_limit:
      if (val)
	u->sp_limit= *val & 0xffff;
      else
	cell->set(u->sp_limit);
      break;
    case z80cpu_nuof: break;
    }
  return cell->get();
}

void
cl_z80_cpu::print_info(class cl_console_base *con)
{
  con->dd_printf("R= 0x%02x\n", zuc->regs.R);
  con->dd_printf("I= 0x%02x\n", zuc->regs.iv);
  con->dd_printf("IFF1= %d IFF2= %d\n",
		 (zuc->IFF1)?1:0,
		 (zuc->IFF2)?1:0);
}


/* End of z80.src/z80.cc */
