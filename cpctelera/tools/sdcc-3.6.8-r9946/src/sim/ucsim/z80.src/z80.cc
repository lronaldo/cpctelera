/*
 * Simulator of microcontrollers (z80.cc)
 *
 * some z80 code base from Karl Bongers karl@turbobit.com
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

#include "ddconfig.h"

#include <stdarg.h> /* for va_list */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "i_string.h"

// prj
#include "pobjcl.h"

// sim
#include "simcl.h"

// local
#include "z80cl.h"
#include "glob.h"
#include "regsz80.h"

#define uint32 t_addr
#define uint8 unsigned char

/*******************************************************************/


/*
 * Base type of Z80 controllers
 */

cl_z80::cl_z80(struct cpu_entry *Itype, class cl_sim *asim):
  cl_uc(asim)
{
  type= Itype;
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

  return(0);
}

char *
cl_z80::id_string(void)
{
  return((char*)"unspecified Z80");
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
cl_z80::mk_hw_elements(void)
{
  //class cl_base *o;
  cl_uc::mk_hw_elements();
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

  chip= new cl_memory_chip("rom_chip", 0x10000, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as= address_space("rom"), chip, 0, 0xffff, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  inputs= new cl_address_space("inputs", 0, 0x10000, 8);
  inputs->init();
  chip= new cl_memory_chip("in_chip", 0x10000, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(inputs, chip, 0, 0xffff, 0);
  ad->init();
  inputs->decoders->add(ad);
  address_spaces->add(inputs);
  outputs= new cl_address_space("outputs", 0, 0x10000, 8);
  outputs->init();
  chip= new cl_memory_chip("out_chip", 0x10000, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(outputs, chip, 0, 0xffff, 0);
  ad->init();
  outputs->decoders->add(ad);
  address_spaces->add(outputs);
  
  regs8= new cl_address_space("regs8", 0, 16, 8);
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

  class cl_var *v;
  vars->add(v= new cl_var(cchars("A"), regs8, 0, ""));
  v->init();
  vars->add(v= new cl_var(cchars("F"), regs8, 1, ""));
  v->init();
  vars->add(v= new cl_var(cchars("B"), regs8, 2, ""));
  v->init();
  vars->add(v= new cl_var(cchars("C"), regs8, 3, ""));
  v->init();
  vars->add(v= new cl_var(cchars("D"), regs8, 4, ""));
  v->init();
  vars->add(v= new cl_var(cchars("E"), regs8, 5, ""));
  v->init();
  vars->add(v= new cl_var(cchars("H"), regs8, 6, ""));
  v->init();
  vars->add(v= new cl_var(cchars("L"), regs8, 7, ""));
  v->init();

  vars->add(v= new cl_var(cchars("ALT_A"), regs8, 8, ""));
  v->init();
  vars->add(v= new cl_var(cchars("ALT_F"), regs8, 9, ""));
  v->init();
  vars->add(v= new cl_var(cchars("ALT_B"), regs8, 10, ""));
  v->init();
  vars->add(v= new cl_var(cchars("ALT_C"), regs8, 11, ""));
  v->init();
  vars->add(v= new cl_var(cchars("ALT_D"), regs8, 12, ""));
  v->init();
  vars->add(v= new cl_var(cchars("ALT_E"), regs8, 13, ""));
  v->init();
  vars->add(v= new cl_var(cchars("ALT_H"), regs8, 14, ""));
  v->init();
  vars->add(v= new cl_var(cchars("ALT_L"), regs8, 15, ""));
  v->init();

  vars->add(v= new cl_var(cchars("AF"), regs16, 0, ""));
  v->init();
  vars->add(v= new cl_var(cchars("BC"), regs16, 1, ""));
  v->init();
  vars->add(v= new cl_var(cchars("DE"), regs16, 2, ""));
  v->init();
  vars->add(v= new cl_var(cchars("HL"), regs16, 3, ""));
  v->init();
  vars->add(v= new cl_var(cchars("IX"), regs16, 4, ""));
  v->init();
  vars->add(v= new cl_var(cchars("IY"), regs16, 5, ""));
  v->init();
  vars->add(v= new cl_var(cchars("SP"), regs16, 6, ""));
  v->init();
  vars->add(v= new cl_var(cchars("ALT_AF"), regs16, 7, ""));
  v->init();
  vars->add(v= new cl_var(cchars("ALT_BC"), regs16, 8, ""));
  v->init();
  vars->add(v= new cl_var(cchars("ALT_DE"), regs16, 9, ""));
  v->init();
  vars->add(v= new cl_var(cchars("ALT_HL"), regs16, 10, ""));
  v->init();
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
cl_z80::disass(t_addr addr, const char *sep)
{
  char work[256], temp[20];
  const char *b;
  char *buf, *p, *t;
  int len = 0;
  int immed_offset = 0;

  p= work;

  b = get_disasm_info(addr, &len, NULL, &immed_offset, NULL);

  if (b == NULL) {
    buf= (char*)malloc(30);
    strcpy(buf, "UNKNOWN/INVALID");
    return(buf);
  }

  while (*b)
    {
      if (*b == '%')
        {
          b++;
          switch (*(b++))
            {
            case 'd': // d    jump relative target, signed? byte immediate operand
              sprintf(temp, "#%d", (char)rom->get(addr+immed_offset));
              ++immed_offset;
              break;
            case 'w': // w    word immediate operand
              sprintf(temp, "#0x%04x",
                 (uint)((rom->get(addr+immed_offset)) |
                        (rom->get(addr+immed_offset+1)<<8)) );
              ++immed_offset;
              ++immed_offset;
              break;
            case 'b': // b    byte immediate operand
              sprintf(temp, "#0x%02x", (uint)rom->get(addr+immed_offset));
              ++immed_offset;
              break;
            default:
              strcpy(temp, "?");
              break;
            }
          t= temp;
          while (*t)
            *(p++)= *(t++);
        }
      else
        *(p++)= *(b++);
    }
  *p= '\0';

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
  for (p= work, t= buf; *p != ' '; p++, t++)
    *t= *p;
  p++;
  *t= '\0';
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


void
cl_z80::print_regs(class cl_console_base *con)
{
  con->dd_printf("SZ-A-PNC  Flags= 0x%02x %3d %c  ",
                 regs.raf.F, regs.raf.F, isprint(regs.raf.F)?regs.raf.F:'.');
  con->dd_printf("A= 0x%02x %3d %c\n",
                 regs.raf.A, regs.raf.A, isprint(regs.raf.A)?regs.raf.A:'.');
  con->dd_printf("%c%c-%c-%c%c%c\n",
                 (regs.raf.F&BIT_S)?'1':'0',
                 (regs.raf.F&BIT_Z)?'1':'0',
                 (regs.raf.F&BIT_A)?'1':'0',
                 (regs.raf.F&BIT_P)?'1':'0',
                 (regs.raf.F&BIT_N)?'1':'0',
                 (regs.raf.F&BIT_C)?'1':'0');
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
  con->dd_printf("SP= 0x%04x [SP]= %02x %3d %c\n",
                 regs.SP, ram->get(regs.SP), ram->get(regs.SP),
                 isprint(ram->get(regs.SP))?ram->get(regs.SP):'.');

  print_disass(PC, con);
}

/*
 * Execution
 */

int
cl_z80::exec_inst(void)
{
  t_mem code;

  if (fetch(&code))
    return(resBREAKPOINT);
  tick(1);
  switch (code)
    {
    case 0x00: return(inst_nop(code));
    case 0x01: case 0x02: case 0x06: return(inst_ld(code));
    case 0x03: case 0x04: return(inst_inc(code));
    case 0x05: return(inst_dec(code));
    case 0x07: return(inst_rlca(code));

    case 0x08: return(inst_ex(code));
    case 0x09: return(inst_add(code));
    case 0x0a: case 0x0e: return(inst_ld(code));
    case 0x0b: case 0x0d: return(inst_dec(code));
    case 0x0c: return(inst_inc(code));
    case 0x0f: return(inst_rrca(code));


    case 0x10: return(inst_djnz(code));
    case 0x11: case 0x12: case 0x16: return(inst_ld(code));
    case 0x13: case 0x14: return(inst_inc(code));
    case 0x15: return(inst_dec(code));
    case 0x17: return(inst_rla(code));

    case 0x18: return(inst_jr(code));
    case 0x19: return(inst_add(code));
    case 0x1a: case 0x1e: return(inst_ld(code));
    case 0x1b: case 0x1d: return(inst_dec(code));
    case 0x1c: return(inst_inc(code));
    case 0x1f: return(inst_rra(code));


    case 0x20: return(inst_jr(code));
    case 0x21: case 0x22: case 0x26: return(inst_ld(code));
    case 0x23: case 0x24: return(inst_inc(code));
    case 0x25: return(inst_dec(code));
    case 0x27: return(inst_daa(code));

    case 0x28: return(inst_jr(code));
    case 0x29: return(inst_add(code));
    case 0x2a: case 0x2e: return(inst_ld(code));
    case 0x2b: case 0x2d: return(inst_dec(code));
    case 0x2c: return(inst_inc(code));
    case 0x2f: return(inst_cpl(code));


    case 0x30: return(inst_jr(code));
    case 0x31: case 0x32: case 0x36: return(inst_ld(code));
    case 0x33: case 0x34: return(inst_inc(code));
    case 0x35: return(inst_dec(code));
    case 0x37: return(inst_scf(code));

    case 0x38: return(inst_jr(code));
    case 0x39: return(inst_add(code));
    case 0x3a: case 0x3e: return(inst_ld(code));
    case 0x3b: case 0x3d: return(inst_dec(code));
    case 0x3c: return(inst_inc(code));
    case 0x3f: return(inst_ccf(code));

    case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
    case 0x48: case 0x49: case 0x4a: case 0x4b: case 0x4c: case 0x4d: case 0x4e: case 0x4f:
      return(inst_ld(code));

    case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
    case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
      return(inst_ld(code));

    case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
    case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
      return(inst_ld(code));

    case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77:
    case 0x78: case 0x79: case 0x7a: case 0x7b: case 0x7c: case 0x7d: case 0x7e: case 0x7f:
      return(inst_ld(code));
    case 0x76:
      return(inst_halt(code));

    case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
      return(inst_add(code));
    case 0x88: case 0x89: case 0x8a: case 0x8b: case 0x8c: case 0x8d: case 0x8e: case 0x8f:
      return(inst_adc(code));

    case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
      return(inst_sub(code));
    case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
      return(inst_sbc(code));

    case 0xa0: case 0xa1: case 0xa2: case 0xa3: case 0xa4: case 0xa5: case 0xa6: case 0xa7:
      return(inst_and(code));
    case 0xa8: case 0xa9: case 0xaa: case 0xab: case 0xac: case 0xad: case 0xae: case 0xaf:
      return(inst_xor(code));

    case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
      return(inst_or(code));
    case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
      return(inst_cp(code));

    case 0xc0: return(inst_ret(code));
    case 0xc1: return(inst_pop(code));
    case 0xc2: case 0xc3: return(inst_jp(code));
    case 0xc4: return(inst_call(code));
    case 0xc5: return(inst_push(code));
    case 0xc6: return(inst_add(code));
    case 0xc7: return(inst_rst(code));

    case 0xc8: case 0xc9: return(inst_ret(code));
    case 0xca: return(inst_jp(code));

      /* CB escapes out to 2 byte opcodes(CB include), opcodes
         to do register bit manipulations */
    case 0xcb: return(inst_cb());
    case 0xcc: case 0xcd: return(inst_call(code));
    case 0xce: return(inst_adc(code));
    case 0xcf: return(inst_rst(code));


    case 0xd0: return(inst_ret(code));
    case 0xd1: return(inst_pop(code));
    case 0xd2: return(inst_jp(code));
    case 0xd3: return(inst_out(code));
    case 0xd4: return(inst_call(code));
    case 0xd5: return(inst_push(code));
    case 0xd6: return(inst_sub(code));
    case 0xd7: return(inst_rst(code));

    case 0xd8: return(inst_ret(code));
    case 0xd9: return(inst_exx(code));
    case 0xda: return(inst_jp(code));
    case 0xdb: return(inst_in(code));
    case 0xdc: return(inst_call(code));
      /* DD escapes out to 2 to 4 byte opcodes(DD included)
        with a variety of uses.  It can precede the CB escape
        sequence to extend CB codes with IX+immed_byte */
    case 0xdd: return(inst_dd());
    case 0xde: return(inst_sbc(code));
    case 0xdf: return(inst_rst(code));


    case 0xe0: return(inst_ret(code));
    case 0xe1: return(inst_pop(code));
    case 0xe2: return(inst_jp(code));
    case 0xe3: return(inst_ex(code));
    case 0xe4: return(inst_call(code));
    case 0xe5: return(inst_push(code));
    case 0xe6: return(inst_and(code));
    case 0xe7: return(inst_rst(code));

    case 0xe8: return(inst_ret(code));
    case 0xe9: return(inst_jp(code));
    case 0xea: return(inst_jp(code));
    case 0xeb: return(inst_ex(code));
    case 0xec: return(inst_call(code));
      /* ED escapes out to misc IN, OUT and other oddball opcodes */
    case 0xed: return(inst_ed());
    case 0xee: return(inst_xor(code));
    case 0xef: return(inst_rst(code));


    case 0xf0: return(inst_ret(code));
    case 0xf1: return(inst_pop(code));
    case 0xf2: return(inst_jp(code));
    case 0xf3: return(inst_di(code));
    case 0xf4: return(inst_call(code));
    case 0xf5: return(inst_push(code));
    case 0xf6: return(inst_or(code));
    case 0xf7: return(inst_rst(code));

    case 0xf8: return(inst_ret(code));
    case 0xf9: return(inst_ld(code));
    case 0xfa: return(inst_jp(code));
    case 0xfb: return(inst_ei(code));
    case 0xfc: return(inst_call(code));
      /* DD escapes out to 2 to 4 byte opcodes(DD included)
        with a variety of uses.  It can precede the CB escape
        sequence to extend CB codes with IX+immed_byte */
    case 0xfd: return(inst_fd());
    case 0xfe: return(inst_cp(code));
    case 0xff: return(inst_rst(code));
    }

  /*if (PC)
    PC--;
  else
  PC= get_mem_size(MEM_ROM_ID)-1;*/
  PC= rom->inc_address(PC, -1);

  sim->stop(resINV_INST);
  return(resINV_INST);
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

/* End of z80.src/z80.cc */
