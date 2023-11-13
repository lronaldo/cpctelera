/*
 * Simulator of microcontrollers (arith.cc)
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

//#include <stdio.h>

// local
#include "uc51cl.h"
#include "regs51.h"
#include "types51.h"


/*
 * 0x03 1 12 RR A
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_03/*inst_rr*/(t_mem/*uchar*/ code)
{
  uchar ac;

  ac= acc->read(); 
  if (ac & 0x01)
    acc->write((ac >> 1) | 0x80);
  else
    acc->write(ac >> 1);
  //vc.rd++;
  //vc.wr++;
  return(resGO);
}


/*
 * 0x13 1 12 RRC A
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_13/*inst_rrc*/(t_mem/*uchar*/ code)
{
  bool cy;
  uchar ac;

  cy= /*SFR_GET_C*/bits->get(0xd7);
  /*SFR_SET_C(*/bits->set(0xd7, (ac= acc->read()) & 0x01);
  ac>>= 1;
  if (cy)
    ac|= 0x80;
  sfr->write(ACC, ac);
  //vc.rd++;
  //vc.wr++;
  return(resGO);
}


/*
 * 0x23 1 12 RL A
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_23/*inst_rl*/(t_mem/*uchar*/ code)
{
  uchar ac;

  ac= acc->read();
  if (ac & 0x80)
    acc->write((ac << 1 ) | 0x01);
  else
    acc->write(ac << 1);
  //vc.rd++;
  //vc.wr++;
  return(resGO);
}


/*
 * 0x24 2 12 ADD A,#data
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_24/*inst_add_a_Sdata*/(t_mem/*uchar*/ code)
{
  uchar data, ac;
  u8_t newC, newA, c6;

  data= fetch();
  ac  = acc->read();
  newC= (((uint)ac+(uint)(data)) > 255)?0x80:0;
  newA= ((ac&0x0f)+(data&0x0f)) & 0xf0;
  c6  = ((ac&0x7f)+(data&0x7f)) & 0x80;
  acc->write(ac+data);
  /*SFR_SET_C(*/bits->set(0xd7, newC);
  SFR_SET_BIT(newC ^ c6, PSW, bmOV);
  SFR_SET_BIT(newA, PSW, bmAC);
  //vc.rd++;
  //vc.wr++;
  return(resGO);
}


/*
 * 0x25 2 12 ADD A,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_25/*inst_add_a_addr*/(t_mem/*uchar*/ code)
{
  uchar data, ac;
  u8_t newC, newA, c6;
  class cl_memory_cell *cell;
  t_addr a;

  cell= get_direct(a= fetch());
  data= cell->read();
  ac  = acc->get();
  newC= (((uint)ac+(uint)(data)) > 255)?0x80:0;
  newA= ((ac&0x0f)+(data&0x0f)) & 0xf0;
  c6  = ((ac&0x7f)+(data&0x7f)) & 0x80;
  acc->write(ac+data);
  /*SFR_SET_C(*/bits->set(0xd7, newC);
  SFR_SET_BIT(newC ^ c6, PSW, bmOV);
  SFR_SET_BIT(newA, PSW, bmAC);
  vc.rd++;//= 2;
  //vc.wr++;
  return(resGO);
}


/*
 * 0x26-0x27 1 12 ADD A,@Ri
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_26/*inst_add_a_Sri*/(t_mem/*uchar*/ code)
{
  uchar data, ac;
  u8_t newC, newA, c6;
  class cl_memory_cell *cell;

  cell= iram->get_cell(R[code & 0x01]->read());
  ac  = acc->get();
  data= cell->read();
  newC= (((uint)ac+(uint)data) > 255)?0x80:0;
  newA= ((ac&0x0f)+(data&0x0f)) & 0xf0;
  c6  = ((ac&0x7f)+(data&0x7f)) & 0x80;
  acc->write(ac+data);
  /*SFR_SET_C(*/bits->set(0xd7, newC);
  SFR_SET_BIT(newC ^ c6, PSW, bmOV);
  SFR_SET_BIT(newA, PSW, bmAC);
  vc.rd++;//= 3;
  //vc.wr++;
  return(resGO);
}


/*
 * 0x28-0x2f 1 12 ADD A,Rn
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_28/*inst_add_a_rn*/(t_mem/*uchar*/ code)
{
  uchar data, ac;
  u8_t newC, newA, c6;

  data= R[code & 0x07]->read();
  ac  = acc->get();
  newC= (((uint)ac+(uint)data) > 255)?0x80:0;
  newA= ((ac&0x0f)+(data&0x0f)) & 0xf0;
  c6  = ((ac&0x7f)+(data&0x7f)) & 0x80;
  acc->write(ac+data);
  /*SFR_SET_C(*/bits->set(0xd7, newC);
  SFR_SET_BIT(newC ^ c6, PSW, bmOV);
  SFR_SET_BIT(newA, PSW, bmAC);
  //vc.rd+= 2;
  //vc.wr++;
  return(resGO);
}


/*
 * 0x33 1 12 RLC A
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_33/*inst_rlc*/(t_mem/*uchar*/ code)
{
  bool cy;
  uchar ac;

  cy= /*SFR_GET_C*/bits->get(0xd7);
  /*SFR_SET_C(*/bits->set(0xd7, (ac= acc->get()) & 0x80);
  ac<<= 1;
  if (cy)
    ac|= 0x01;
  acc->write(ac);
  //vc.rd++;
  //vc.wr++;
  return(resGO);
}


/*
 * 0x34 2 12 ADDC A,#data
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_34/*inst_addc_a_Sdata*/(t_mem/*uchar*/ code)
{
  uchar data, ac;
  u8_t orgC, newC, newA, c6;

  data= fetch();
  ac  = acc->get();
  newC= (((uint)ac+(uint)data+((orgC= /*SFR_GET_C*/bits->get(0xd7))?1:0)) > 255)?0x80:0;
  newA= ((ac&0x0f)+(data&0x0f)+(orgC?1:0)) & 0xf0;
  c6  = ((ac&0x7f)+(data&0x7f)+(orgC?1:0)) & 0x80;
  acc->write(ac + data + (orgC?1:0));
  /*SFR_SET_C(*/bits->set(0xd7, newC);
  SFR_SET_BIT(newC ^ c6, PSW, bmOV);
  SFR_SET_BIT(newA, PSW, bmAC);
  //vc.rd++;
  //vc.wr++;
  return(resGO);
}


/*
 * 0x35 2 12 ADDC A,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_35/*inst_addc_a_addr*/(t_mem/*uchar*/ code)
{
  uchar data, ac;
  u8_t orgC, newC, newA, c6;
  class cl_memory_cell *cell;
  t_addr a;

  cell= get_direct(a= fetch());
  data= cell->read();
  ac  = acc->get();
  newC= (((uint)ac+(uint)data+((orgC= /*SFR_GET_C*/bits->get(0xd7))?1:0)) > 255)?0x80:0;
  newA= ((ac&0x0f)+(data&0x0f)+(orgC?1:0)) & 0xf0;
  c6  = ((ac&0x7f)+(data&0x7f)+(orgC?1:0)) & 0x80;
  acc->write(ac + data + (orgC?1:0));
  /*SFR_SET_C(*/bits->set(0xd7, newC);
  SFR_SET_BIT(newC ^ c6, PSW, bmOV);
  SFR_SET_BIT(newA, PSW, bmAC);
  vc.rd++;//= 2;
  //vc.wr++;
  return(resGO);
}


/*
 * 0x36-0x37 1 12 ADDC A,@Ri
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_36/*inst_addc_a_Sri*/(t_mem/*uchar*/ code)
{
  uchar data, ac;
  u8_t orgC, newC, newA, c6;
  class cl_memory_cell *cell;
  
  cell= iram->get_cell(R[code & 0x01]->read());
  ac  = acc->get();
  data= cell->read();
  newC= (((uint)ac+(uint)data+((orgC= /*SFR_GET_C*/bits->get(0xd7))?1:0)) > 255)?0x80:0;
  newA= ((ac&0x0f)+(data&0x0f)+(orgC?1:0)) & 0xf0;
  c6  = ((ac&0x7f)+(data&0x7f)+(orgC?1:0)) & 0x80;
  acc->write(ac + data + (orgC?1:0));
  /*SFR_SET_C(*/bits->set(0xd7, newC);
  SFR_SET_BIT(newC ^ c6, PSW, bmOV);
  SFR_SET_BIT(newA, PSW, bmAC);
  vc.rd++;//= 3;
  //vc.rd++;
  return(resGO);
}


/*
 * 0x38-0x3f 1 12 ADDC A,Rn
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_38/*inst_addc_a_rn*/(t_mem/*uchar*/ code)
{
  uchar data, ac;
  u8_t orgC, newC, newA, c6;

  data= R[code & 0x07]->read();
  ac  = acc->get();
  newC= (((uint)ac+(uint)data+((orgC= /*SFR_GET_C*/bits->get(0xd7))?1:0)) > 255)?0x80:0;
  newA= ((ac&0x0f)+(data&0x0f)+(orgC?1:0)) & 0xf0;
  c6  = ((ac&0x7f)+(data&0x7f)+(orgC?1:0)) & 0x80;
  acc->write(ac + data + (orgC?1:0));
  /*SFR_SET_C(*/bits->set(0xd7, newC);
  SFR_SET_BIT(newC ^ c6, PSW, bmOV);
  SFR_SET_BIT(newA, PSW, bmAC);
  //vc.rd+= 2;
  //vc.wr++;
  return(resGO);
}


/*
 * 0x84 1 48 DIV AB
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_84/*inst_div_ab*/(t_mem/*uchar*/ code)
{
  uchar temp, pw, b, ac;

  pw= psw->get();
  pw&= ~bmCY;
  ac= acc->get();
  if (!(b= sfr->get(B)))
    pw|= bmOV;
  else
    {
      pw&= ~bmOV;
      temp= (ac) / b;
      sfr->write(B, ac % b);
      ac= temp;
    }
  psw->write(pw);
  acc->write(ac);
  tick(3);
  vc.rd++;//= 2;
  vc.wr++;//= 2;
  return(resGO);
}


/*
 * 0x94 2 12 SUBB A,#data
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_94/*inst_subb_a_Sdata*/(t_mem/*uchar*/ code)
{
  uchar data, ac, result, pw, c;

  data= fetch();
  ac  = acc->get();
  result= ac-data;
  pw= psw->get();
  if ((c= (pw & bmCY)?1:0))
    result--;
  psw->set((pw & ~(bmCY|bmOV|bmAC)) |
	   (((unsigned int)ac < (unsigned int)(data+c))?bmCY:0) |
	   (((ac<0x80 && data>0x7f && result>0x7f) ||
	     (ac>0x7f && data<0x80 && result<0x80))?bmOV:0) |
	   (((ac&0x0f) < ((data+c)&0x0f) ||
	     (c && ((data&0x0f)==0x0f)))?bmAC:0));
  acc->write(result);
  //vc.rd++;
  //vc.wr++;
  return(resGO);
}


/*
 * 0x95 2 12 SUBB A,addr
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_95/*inst_subb_a_addr*/(t_mem/*uchar*/ code)
{
  uchar data, ac, result, pw, c;
  class cl_memory_cell *cell;
  
  cell= get_direct(fetch());
  ac  = acc->get();
  data= cell->read();
  result= ac-data;
  pw= psw->get();
  if ((c= (pw & bmCY)?1:0))
    result--;
  psw->set((pw & ~(bmCY|bmOV|bmAC)) |
	   (((unsigned int)ac < (unsigned int)(data+c))?bmCY:0) |
	   (((ac<0x80 && data>0x7f && result>0x7f) ||
	     (ac>0x7f && data<0x80 && result<0x80))?bmOV:0) |
	   (((ac&0x0f) < ((data+c)&0x0f) ||
	     (c && ((data&0x0f)==0x0f)))?bmAC:0));
  acc->write(result);
  vc.rd++;//= 2;
  //vc.rd++;
  return(resGO);
}


/*
 * 0x96-0x97 1 12 SUBB A,@Ri
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_96/*inst_subb_a_Sri*/(t_mem/*uchar*/ code)
{
  uchar data, ac, result, pw, c;
  class cl_memory_cell *cell;

  cell= iram->get_cell(R[code & 0x01]->read());
  data= cell->read();
  ac  = acc->get();
  result= ac-data;
  pw= psw->get();
  if ((c= (pw & bmCY)?1:0))
    result--;
  psw->write((pw & ~(bmCY|bmOV|bmAC)) |
	     (((unsigned int)ac < (unsigned int)(data+c))?bmCY:0) |
	     (((ac<0x80 && data>0x7f && result>0x7f) ||
	       (ac>0x7f && data<0x80 && result<0x80))?bmOV:0) |
	     (((ac&0x0f) < ((data+c)&0x0f) ||
	       (c && ((data&0x0f)==0x0f)))?bmAC:0));
  acc->write(result);
  vc.rd++;//= 3;
  //vc.wr++;
  return(resGO);
}


/*
 * 0x98-0x9f 1 12 SUBB A,Rn
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_98/*inst_subb_a_rn*/(t_mem/*uchar*/ code)
{
  uchar data, ac, result, pw, c;

  data= R[code & 0x07]->read();
  ac  = acc->get();
  result= ac-data;
  pw= psw->get();
  if ((c= (pw & bmCY)?1:0))
    result--;
  psw->write((pw & ~(bmCY|bmOV|bmAC)) |
	     (((unsigned int)ac < (unsigned int)(data+c))?bmCY:0) |
	     (((ac<0x80 && data>0x7f && result>0x7f) ||
	       (ac>0x7f && data<0x80 && result<0x80))?bmOV:0) |
	     (((ac&0x0f) < ((data+c)&0x0f) ||
	       (c && ((data&0x0f)==0x0f)))?bmAC:0));
  acc->write(result);
  //vc.rd+= 2;
  //vc.wr++;
  return(resGO);
}


/*
 * 0xa4 1 48 MUL AB
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_a4/*inst_mul_ab*/(t_mem/*uchar*/ code)
{
  uint temp, pw, ac, b, x;

  pw= psw->get();
  pw&= ~bmCY;
  temp= (ac= acc->read()) * (b= sfr->get(B));
  acc->write(temp & 0xff);
  x= sfr->write(B, (temp >> 8) & 0xff);
  SFR_SET_BIT(x/*sfr->get(B)*/, PSW, bmOV);
  SFR_SET_BIT(0, PSW, bmCY);
  tick(3);
  vc.rd++;//= 2;
  vc.wr++;//= 2;
  return(resGO);
}


/*
 * 0xd4 1 12 DA A
 *____________________________________________________________________________
 *
 */

int
cl_51core::instruction_d4/*inst_da_a*/(t_mem/*uchar*/ code)
{
  uchar ac, pw;

  ac= acc->get();
  pw= psw->get();
  if ((ac & 0x0f) > 9 ||
      (pw & bmAC))
    {
      if (((uint)ac+(uint)0x06) > 255)
	pw|= bmCY;
      ac+= 0x06;
    }
  if ((ac & 0xf0) > 0x90 ||
      (pw & bmCY))
    {
      if (((uint)ac+(uint)0x60) > 255)
	pw|= bmCY;
      ac+= 0x60;
    }
  psw->write(pw);
  acc->write(ac);
  //vc.rd++;
  //vc.wr++;
  return(resGO);
}


/* End of s51.src/arith.cc */
