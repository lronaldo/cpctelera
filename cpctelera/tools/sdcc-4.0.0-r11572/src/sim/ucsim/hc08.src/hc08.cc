/*
 * Simulator of microcontrollers (hc08.cc)
 *
 * some hc08 code base from Karl Bongers karl@turbobit.com
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

//#include "ddconfig.h"

//#include <stdarg.h> /* for va_list */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
//#include "i_string.h"

// prj
//#include "pobjcl.h"

// sim
#include "simcl.h"

// local
#include "hc08cl.h"
#include "glob.h"
//#include "regshc08.h"
#include "hc08mac.h"

#define uint32 t_addr
#define uint8 unsigned char
#define int8 char

//const bool TRUE = 1;
//const bool FALSE = 0;

/*******************************************************************/


/*
 * Base type of HC08 controllers
 */

cl_hc08::cl_hc08(struct cpu_entry *Itype, class cl_sim *asim):
  cl_uc(asim)
{
  type= Itype;
}

int
cl_hc08::init(void)
{
  cl_uc::init(); /* Memories now exist */

  xtal = 8000000;

  //rom= address_space(MEM_ROM_ID);
//  ram= mem(MEM_XRAM);
  //ram= rom;

  // zero out ram(this is assumed in regression tests)
  for (int i=0x80; i<0x8000; i++) {
    ram->set((t_addr) i, 0);
  }

  sp_limit= 0x7000;
  return(0);
}


void
cl_hc08::reset(void)
{
  cl_uc::reset();

  regs.SP = 0xff;
  regs.A = 0;
  regs.X = 0;
  regs.H = 0;
  regs.P = 0x60;
  regs.VECTOR = 1;

}


char *
cl_hc08::id_string(void)
{
  return((char*)"unspecified HC08");
}


/*
 * Making elements of the controller
 */
/*
t_addr
cl_hc08::get_mem_size(enum mem_class type)
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
cl_hc08::mk_hw_elements(void)
{
  //class cl_base *o;
  class cl_hw *h;
  cl_uc::mk_hw_elements();

  add_hw(h= new cl_hc08_cpu(this));
  h->init();
}

void
cl_hc08::make_memories(void)
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


  regs8= new cl_address_space("regs8", 0, 4, 8);
  regs8->init();
  regs8->get_cell(0)->decode((t_mem*)&regs.A);
  regs8->get_cell(1)->decode((t_mem*)&regs.P);
  regs8->get_cell(2)->decode((t_mem*)&regs.H);
  regs8->get_cell(3)->decode((t_mem*)&regs.X);

  regs16= new cl_address_space("regs16", 0, 1, 16);
  regs16->init();

  regs16->get_cell(0)->decode((t_mem*)&regs.SP);

  address_spaces->add(regs8);
  address_spaces->add(regs16);

  class cl_var *v;
  vars->add(v= new cl_var(cchars("A"), regs8, 0, ""));
  v->init();
  vars->add(v= new cl_var(cchars("P"), regs8, 1, ""));
  v->init();
  vars->add(v= new cl_var(cchars("H"), regs8, 2, ""));
  v->init();
  vars->add(v= new cl_var(cchars("X"), regs8, 3, ""));
  v->init();

  vars->add(v= new cl_var(cchars("SP"), regs16, 0, ""));
  v->init();
}


/*
 * Help command interpreter
 */

struct dis_entry *
cl_hc08::dis_tbl(void)
{
  return(disass_hc08);
}

/*struct name_entry *
cl_hc08::sfr_tbl(void)
{
  return(0);
}*/

/*struct name_entry *
cl_hc08::bit_tbl(void)
{
  //FIXME
  return(0);
}*/

int
cl_hc08::inst_length(t_addr addr)
{
  int len = 0;
  /*char *s;

    s =*/ get_disasm_info(addr, &len, NULL, NULL, NULL);

  return len;
}

int
cl_hc08::inst_branch(t_addr addr)
{
  int b;
  /*char *s;

    s =*/ get_disasm_info(addr, NULL, &b, NULL, NULL);

  return b;
}


bool
cl_hc08::is_call(t_addr addr)
{
  struct dis_entry *e;

  get_disasm_info(addr, NULL, NULL, NULL, &e);
  
  return e?(e->is_call):false;
}

int
cl_hc08::longest_inst(void)
{
  return 4;
}


const char *
cl_hc08::get_disasm_info(t_addr addr,
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
    case 0x9e:  /* ESC code to sp relative op-codes */
      code= rom->get(addr++);
      i= 0;
      while ((code & disass_hc08_9e[i].mask) != disass_hc08_9e[i].code &&
        disass_hc08_9e[i].mnemonic)
        i++;
      dis_e = &disass_hc08_9e[i];
      b= disass_hc08_9e[i].mnemonic;
      if (b != NULL)
        len += (disass_hc08_9e[i].length + 1);
    break;

    default:
      i= 0;
      while ((code & disass_hc08[i].mask) != disass_hc08[i].code &&
             disass_hc08[i].mnemonic)
        i++;
      dis_e = &disass_hc08[i];
      b= disass_hc08[i].mnemonic;
      if (b != NULL)
        len += (disass_hc08[i].length);
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
cl_hc08::disass(t_addr addr, const char *sep)
{
  char work[256], temp[20];
  char *buf, *p, *t, *s;
  const char *b;
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
	    case 's': // s    signed byte immediate
	      sprintf(temp, "#%d", (char)rom->get(addr+immed_offset));
	      ++immed_offset;
	      break;
	    case 'w': // w    word immediate operand
	      sprintf(temp, "#0x%04x",
	         (uint)((rom->get(addr+immed_offset)<<8) |
	                (rom->get(addr+immed_offset+1))) );
	      ++immed_offset;
	      ++immed_offset;
	      break;
	    case 'b': // b    byte immediate operand
	      sprintf(temp, "#0x%02x", (uint)rom->get(addr+immed_offset));
	      ++immed_offset;
	      break;
	    case 'x': // x    extended addressing
	      sprintf(temp, "0x%04x",
	         (uint)((rom->get(addr+immed_offset)<<8) |
	                (rom->get(addr+immed_offset+1))) );
	      ++immed_offset;
	      ++immed_offset;
	      break;
	    case 'd': // d    direct addressing
	      sprintf(temp, "*0x%02x", (uint)rom->get(addr+immed_offset));
	      ++immed_offset;
	      break;
	    case '2': // 2    word index offset
	      {
		int i= (uint)((rom->get(addr+immed_offset)<<8) |
			      (rom->get(addr+immed_offset+1)));
		sprintf(temp, "0x%04x", i & 0xffff);
		++immed_offset;
		++immed_offset;
		break;
	      }		
	    case '1': // b    byte index offset
              sprintf(temp, "0x%02x", (uint)rom->get(addr+immed_offset));
	      ++immed_offset;
	      break;
	    case 'p': // b    byte index offset
	      {
		int i= addr+immed_offset+1
		  +(char)rom->get(addr+immed_offset);
		sprintf(temp, "0x%04x", i & 0xffff);
		++immed_offset;
		break;
	      }
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
  for (p= work, s= buf; *p != ' '; p++, s++)
    *s= *p;
  p++;
  *s= '\0';
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
cl_hc08::print_regs(class cl_console_base *con)
{
  con->dd_printf("V--HINZC  Flags= 0x%02x %3d %c  ",
		 regs.P, regs.P, isprint(regs.P)?regs.P:'.');
  con->dd_printf("A= 0x%02x %3d %c\n",
		 regs.A, regs.A, isprint(regs.A)?regs.A:'.');
  con->dd_printf("%c--%c%c%c%c%c  ",
		 (regs.P&BIT_V)?'1':'0',
		 (regs.P&BIT_H)?'1':'0',
		 (regs.P&BIT_I)?'1':'0',
		 (regs.P&BIT_N)?'1':'0',
		 (regs.P&BIT_Z)?'1':'0',
		 (regs.P&BIT_C)?'1':'0');
  con->dd_printf("    H= 0x%02x %3d %c  ",
		 regs.H, regs.H, isprint(regs.H)?regs.H:'.');
  con->dd_printf("X= 0x%02x %3d %c\n",
		 regs.X, regs.X, isprint(regs.X)?regs.X:'.');
  con->dd_printf("SP= 0x%04x [SP+1]= %02x %3d %c",
                 regs.SP, ram->get(regs.SP+1), ram->get(regs.SP+1),
                 isprint(ram->get(regs.SP+1))?ram->get(regs.SP+1):'.');
  con->dd_printf("  Limit= 0x%04x\n", AU(sp_limit));
  
  print_disass(PC, con);
}

/*
 * Execution
 */

int
cl_hc08::exec_inst(void)
{
  t_mem code;

  if (regs.VECTOR) {
    PC = get2(0xfffe);
    regs.VECTOR = 0;
    return(resGO);
  }

  instPC= PC;

  if (fetch(&code))
    return(resBREAKPOINT);
  tick(1);
  switch ((code >> 4) & 0xf) {
  case 0x0: return(inst_bittestsetclear(code, /*FALSE*/0));
  case 0x1: return(inst_bitsetclear(code, /*FALSE*/0));
    case 0x2: return(inst_condbranch(code, /*FALSE*/0));
    case 0x3:
    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7:
      switch (code & 0xf) {
        case 0x0: return(inst_neg(code, /*FALSE*/0));
        case 0x1: return(inst_cbeq(code, false));
        case 0x2:
          switch (code) {
            case 0x32: return(inst_ldhx(code, false));
            case 0x42: return(inst_mul(code, false));
            case 0x52: return(inst_div(code, false));
            case 0x62: return(inst_nsa(code, false));
            case 0x72: return(inst_daa(code, /*FALSE*/0));
            default: return(resHALT);
          }
        case 0x3: return(inst_com(code, /*FALSE*/0));
        case 0x4: return(inst_lsr(code, /*FALSE*/0));
        case 0x5:
          switch (code) {
            case 0x35: return(inst_sthx(code, /*FALSE*/0));
            case 0x45:
            case 0x55: return(inst_ldhx(code, /*FALSE*/0));
            case 0x65:
            case 0x75: return(inst_cphx(code, /*FALSE*/0));
            default: return(resHALT);
          }
        case 0x6: return(inst_ror(code, /*FALSE*/0));
        case 0x7: return(inst_asr(code, /*FALSE*/0));
        case 0x8: return(inst_lsl(code, /*FALSE*/0));
        case 0x9: return(inst_rol(code, /*FALSE*/0));
        case 0xa: return(inst_dec(code, /*FALSE*/0));
        case 0xb: return(inst_dbnz(code, /*FALSE*/0));
        case 0xc: return(inst_inc(code, /*FALSE*/0));
        case 0xd: return(inst_tst(code, false));
        case 0xe:
          switch (code) {
            case 0x3e: return(inst_cphx(code, false));
            case 0x4e:
            case 0x5e:
            case 0x6e:
            case 0x7e: return(inst_mov(code, /*FALSE*/0));
            default: return(resHALT);
          }
        case 0xf: return(inst_clr(code, /*FALSE*/0));
        default: return(resHALT);
      }
    case 0x8:
      switch (code & 0xf) {
        case 0x0: return(inst_rti(code, /*FALSE*/0));
        case 0x1: return(inst_rts(code, /*FALSE*/0));
        case 0x3: return(inst_swi(code, /*FALSE*/0));
        case 0x4:
        case 0x5: return(inst_transfer(code, /*FALSE*/0));
        case 0x6:
        case 0x7:
        case 0x8:
        case 0x9:
        case 0xa:
        case 0xb: return(inst_pushpull(code, /*FALSE*/0));
        case 0xc: return(inst_clrh(code, false));
        case 0xe: return(inst_stop(code, false));
        case 0xf: return(inst_wait(code, false));
        default: return(resINV_INST); // 0x82 and 0x8d not valid
      }
    case 0x9:
      switch (code & 0xf) {
        case 0x0:
        case 0x1:
        case 0x2:
        case 0x3: return(inst_condbranch(code, false));
        case 0x4:
        case 0x5: return(inst_transfer(code, false));
        case 0x6: return(inst_sthx(code, false));
        case 0x7:
        case 0xf: return(inst_transfer(code, false));
        case 0x8:
        case 0x9:
        case 0xa:
        case 0xb: return(inst_setclearflags(code, false));
        case 0xc: return(inst_rsp(code, false));
        case 0xd: return(inst_nop(code, false));
        case 0xe: // start 0x9e prefix handling
          code = fetch();
          tick(1);
          switch ((code >> 4) & 0xf) {
            case 0x6:
              switch (code & 0xf) {
                case 0x0: return(inst_neg(code, /*TRUE*/1));
                case 0x1: return(inst_cbeq(code, /*TRUE*/1));
                case 0x3: return(inst_com(code, /*TRUE*/1));
                case 0x4: return(inst_lsr(code, /*TRUE*/1));
                case 0x6: return(inst_ror(code, /*TRUE*/1));
                case 0x7: return(inst_asr(code, /*TRUE*/1));
                case 0x8: return(inst_lsl(code, /*TRUE*/1));
                case 0x9: return(inst_rol(code, /*TRUE*/1));
                case 0xa: return(inst_dec(code, /*TRUE*/1));
                case 0xb: return(inst_dbnz(code, /*TRUE*/1));
                case 0xc: return(inst_inc(code, true));
                case 0xd: return(inst_tst(code, true));
                case 0xf: return(inst_clr(code, true));
                default: return(resINV_INST); // 0x9e62, 0x9e65, 0x9e6e not valid
              }
            case 0xa:
              switch (code) {
                case 0xae: return(inst_ldhx(code,true));
                default: return(resINV_INST);
              }
            case 0xb:
              switch (code) {
                case 0xbe: return(inst_ldhx(code,true));
                default: return(resINV_INST);
              }
            case 0xc:
              switch (code) {
                case 0xce: return(inst_ldhx(code,true));
                default: return(resINV_INST);
              }
            case 0xd:
            case 0xe:
              switch (code & 0xf) {
                case 0x0: return(inst_sub(code, /*TRUE*/1));
                case 0x1: return(inst_cmp(code, /*TRUE*/1));
                case 0x2: return(inst_sbc(code, /*TRUE*/1));
                case 0x3: return(inst_cpx(code, /*TRUE*/1));
                case 0x4: return(inst_and(code, /*TRUE*/1));
                case 0x5: return(inst_bit(code, /*TRUE*/1));
                case 0x6: return(inst_lda(code, /*TRUE*/1));
                case 0x7: return(inst_sta(code, /*TRUE*/1));
                case 0x8: return(inst_eor(code, /*TRUE*/1));
                case 0x9: return(inst_adc(code, true));
                case 0xa: return(inst_ora(code, true));
                case 0xb: return(inst_add(code, true));
		case 0xc: return(resHALT); // not real instruction: regression test hack to exit simulation
		case 0xd: putchar(regs.A); fflush(stdout); return(resGO); // not real instruction: regression test hack to output results
                case 0xe: return(inst_ldx(code, true));
                case 0xf: return(inst_stx(code, true));
                default: return(resHALT);
              }
            case 0xf:
              switch (code & 0xf) {
                case 0x3: return(inst_cphx(code, true));
                case 0xe: return(inst_ldhx(code, true));
                case 0xf: return(inst_sthx(code, true));
              }
            default: return(resINV_INST);
	    // end 0x9e prefix handling
          }

      }
    case 0xa:
    case 0xb:
    case 0xc:
    case 0xd:
    case 0xe:
    case 0xf:
      switch (code & 0xf) {
        case 0x0: return(inst_sub(code, /*FALSE*/0));
        case 0x1: return(inst_cmp(code, /*FALSE*/0));
        case 0x2: return(inst_sbc(code, /*FALSE*/0));
        case 0x3: return(inst_cpx(code, /*FALSE*/0));
        case 0x4: return(inst_and(code, /*FALSE*/0));
        case 0x5: return(inst_bit(code, /*FALSE*/0));
        case 0x6: return(inst_lda(code, /*FALSE*/0));
        case 0x7:
          if (code==0xa7)
            return(inst_ais(code, /*FALSE*/0));
          else
            return(inst_sta(code, /*FALSE*/0));
        case 0x8: return(inst_eor(code, /*FALSE*/0));
        case 0x9: return(inst_adc(code, /*FALSE*/0));
        case 0xa: return(inst_ora(code, /*FALSE*/0));
        case 0xb: return(inst_add(code, false));
        case 0xc:
          if (code==0xac)
            return(resINV_INST);
          else
            return(inst_jmp(code, false));
        case 0xd:
          if (code==0xad)
            return(inst_bsr(code, /*FALSE*/0));
          else
            return(inst_jsr(code, /*FALSE*/0));
        case 0xe: return(inst_ldx(code, /*FALSE*/0));
        case 0xf:
          if (code==0xaf)
            return(inst_aix(code, /*FALSE*/0));
          else
            return(inst_stx(code, /*FALSE*/0));
        default: return(resHALT);
      }
    default: return(resHALT);
  }

  /*if (PC)
    PC--;
  else
  PC= get_mem_size(MEM_ROM_ID)-1;*/
  PC= rom->inc_address(PC, -1);

  sim->stop(resINV_INST);
  return(resINV_INST);
}


void
cl_hc08::stack_check_overflow(class cl_stack_op *op)
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

t_mem
cl_hc08::get_1(t_addr addr)
{
  vc.rd++;
  return ram->read(addr & 0xffff);
}

t_mem
cl_hc08::get_2(t_addr addr)
{
  vc.rd+= 2;
  return (ram->read(addr & 0xffff) << 8) | ram->read((addr+1) & 0xffff);
}


cl_hc08_cpu::cl_hc08_cpu(class cl_uc *auc):
  cl_hw(auc, HW_CPU, 0, "cpu")
{
}

int
cl_hc08_cpu::init(void)
{
  cl_hw::init();

  cl_var *v;
  uc->vars->add(v= new cl_var(cchars("sp_limit"), cfg, hc08cpu_sp_limit,
			      cfg_help(hc08cpu_sp_limit)));
  v->init();

  return 0;
}

char *
cl_hc08_cpu::cfg_help(t_addr addr)
{
  switch (addr)
    {
    case hc08cpu_sp_limit:
      return (char*)"Stack overflows when SP is below this limit";
    }
  return (char*)"Not used";
}

t_mem
cl_hc08_cpu::conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val)
{
  class cl_hc08 *u= (class cl_hc08 *)uc;
  if (val)
    cell->set(*val);
  switch ((enum hc08cpu_confs)addr)
    {
    case hc08cpu_sp_limit:
      if (val)
	u->sp_limit= *val & 0xffff;
      else
	cell->set(u->sp_limit);
      break;
    case hc08cpu_nuof: break;
    }
  return cell->get();
}


/* End of hc08.src/hc08.cc */
