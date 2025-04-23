/*
 * Simulator of microcontrollers (fpga.cc)
 *
 * Copyright (C) 2020 Drotos Daniel
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

#include "globals.h"

#include "p1516cl.h"

#include "fpgacl.h"


/*
                                                                        LED
  -------------------------------------------------------------------------
*/

cl_led::cl_led(class cl_fpga *the_fpga, int ax, int ay, u32_t amask):
  cl_base()
{
  fpga= the_fpga;
  x= ax;
  y= ay;
  mask= amask;
  last= 0;
}

void
cl_led::refresh(bool force)
{
  class cl_hw_io *io= fpga->get_io();
  u32_t a= fpga->pb->get() & mask;
  u32_t l= last & mask;
  if (force || (a != l))
    {
      io->tu_go(x, y);
      if (a)
	{
	  io->dd_cprintf("led_on", "@");
	  last|= mask;
	}
      else
	{
	  io->dd_cprintf("led_off", ".");
	  last&= ~mask;
	}
    }
}


/*
                                                                    RGB LED
  -------------------------------------------------------------------------
*/

cl_rgb::cl_rgb(class cl_fpga *the_fpga, int ax, int ay, u32_t amask):
  cl_led(the_fpga, ax, ay, amask)
{
  last= 0;
}

void
cl_rgb::refresh(bool force)
{
  u32_t gm= mask, rm= mask<<8, bm= mask<<16;
  u32_t m= gm|rm|bm;
  class cl_hw_io *io= fpga->get_io();
  u32_t a= fpga->pb->get() & m;
  u32_t l= last & m;
  if (force || (a != l))
    {
      int c= 0;
      io->tu_go(x, y);
      if (a&gm) c|= 2;
      if (a&rm) c|= 1;
      if (a&bm) c|= 4;
      if (a)
	{
	  io->tu_fgcolor(c);
	  io->dd_printf("\033[1m");
	}
      else
	io->dd_color("answer");
      io->dd_printf("%c", a?'@':'.');
      last&= ~m;
      last|= a;
      if (a) io->dd_color("answer");
    }
}


/*
                                                          7 SEGMENT DISPLAY
  -------------------------------------------------------------------------
*/

cl_seg::cl_seg(class cl_fpga *the_fpga, int ax, int ay, int adigit):
  cl_led(the_fpga, ax, ay, 0)
{
  digit= adigit;
}

static const char *dsp[3]= {
  //         1         2         3         4
  //1234567890123456789012345678901234567890123456789
  " _         _    _         _    _    _    _    _    _         _         _    _ ",
  "| |    |   _|   _|  |_|  |_   |_     |  |_|  |_|  |_|  |_   |     _|  |_   |_ ",
  "|_|    |  |_    _|    |   _|  |_|    |  |_|   _|  | |  |_|  |_   |_|  |_   |  "
};

void
cl_seg::refresh(bool force)
{
  class cl_hw_io *io= fpga->get_io();
  u32_t sw= fpga->pj->read(), act, act_what;
  u32_t l, mask, a, lw;
  class cl_p1516 *uc= (class cl_p1516 *)(fpga->uc);
  chars w= "non ";
  bool direct= false;
  int c= -digit + fpga->d2c_b;
  u32_t b_c= fpga->bc->read(), ctr;
  ctr= (b_c&1)?b_c:sw;
  if (last_ctrl != ctr)
    force= true;
  last_ctrl= ctr;
  switch ((ctr>>4)&0xf)
    {
    case 0: act= fpga->pa->get(); w="PA= "; break;
    case 1: act= fpga->pb->get(); w="PB= "; break;
    case 2: act= fpga->pc->get(); w="PC= "; break;
    case 3: act= fpga->pd->get(); w="PD= "; break;
    case 9:
      act= uc->R[sw&0xf];
      if ((sw&0xf)>9)
	w.format("R%d=", sw&0xf);
      else
	w.format("R%d= ", sw&0xf);
      break;
    case 10:
      {
	t_addr a;
	//act= (digit<=3)?(fpga->pd->get()):(fpga->pc->get());
	a= 0xff02; // ODR of PC
	a+= c/4;
	act= fpga->uc->rom->read(a);
	w="CD->";
	direct= true;
	break;
      }
    default: act= 0;
    }
  act_what= sw & 0xff;
  if (force || (act_what != last_what))
    {
      io->tu_go(1,y+2);
      io->dd_cprintf("ui_label", "%s", w.c_str());
    }
  if (!direct)
    {
      mask= 0xf << (digit*4);
      act&= mask;
      l= last & mask;
      if (force || (act != l) || (act_what != last_what))
	{
	  a= act >> (digit*4);
	  a&= 0xf;
	  int s= a*5;
	  io->tu_fgcolor(1); io->dd_printf("\033[1m");
	  io->tu_go(x, y);
	  io->dd_printf("%c%c%c", dsp[0][s], dsp[0][s+1], dsp[0][s+2]);
	  io->tu_go(x, y+1);
	  io->dd_printf("%c%c%c", dsp[1][s], dsp[1][s+1], dsp[1][s+2]);
	  io->tu_go(x, y+2);
	  io->dd_printf("%c%c%c", dsp[2][s], dsp[2][s+1], dsp[2][s+2]);
	  io->dd_printf("\033[0m");
	  io->dd_color("answer");
	  last= (last & ~mask) | act;
	  last_what= act_what;
	}
    }
  else
    {
      // direct
      int sby= 0;
      switch (c % 4)
	{
	case 3: mask= 0xff000000; sby= 3*8; break;
	case 2: mask= 0x00ff0000; sby= 2*8; break;
	case 1: mask= 0x0000ff00; sby= 1*8; break;
	default: mask= 0x000000ff; sby= 0*8; break;
	}
      act&= mask;
      l= last & mask;
      if (force || (act != l) || (act_what != last_what))
	{
	  int bits= (act >> sby) & 0xff;
	  io->tu_fgcolor(1); io->dd_printf("\033[1m");
	  io->tu_go(x, y);
	  io->dd_printf(" %c ", (bits&1)?'_':' ');
	  io->tu_go(x, y+1);
	  io->dd_printf("%c%c%c", (bits&0x20)?'|':' ', (bits&0x40)?'_':' ', (bits&2)?'|':' ');
	  io->tu_go(x, y+2);
	  io->dd_printf("%c%c%c", (bits&0x10)?'|':' ', (bits&8)?'_':' ', (bits&4)?'|':' ');
	  io->dd_printf("\033[0m");
	  io->dd_color("answer");
	  last= (last & ~mask) | act;
	  last_what= act_what;
	}
    }
}

void
cl_seg::draw(void)
{
  class cl_hw_io *io= fpga->get_io();
  io->tu_go(x+1,y+4);
  io->dd_cprintf("ui_label", "%d", digit);
}


/*
                                                                  Input bit
  -------------------------------------------------------------------------
*/

cl_ibit::cl_ibit(class cl_fpga *the_fpga, int ax, int ay, int amask, char akey):
  cl_base()
{
  fpga= the_fpga;
  x= ax;
  y= ay;
  mask= amask;
  key= akey;
}


/*
                                                                     Switch
  -------------------------------------------------------------------------
*/

cl_sw::cl_sw(class cl_fpga *the_fpga, int ax, int ay, int amask, char akey):
  cl_ibit(the_fpga, ax, ay, amask, akey)
{
}

void
cl_sw::refresh(bool force)
{
  char c= ' ';
  u32_t v= 0, act, a, l;
  class cl_hw_io *io= fpga->get_io();
  if (!io) return;
  class cl_memory_cell *p= fpga->pjp;
  if (!p)
    {
      c= '?';
      act= 0;
    }
  else
    {
      v= p->R();
      act= v;
      v&= mask;
      c= '#';
    }
  a= act & mask;
  l= last & mask;
  if (force || (a != l))
    {
      const char *d0= "_ #";
      const char *d1= "_#_";
      const char *p= v?d1:d0;
      if (!p) io->dd_color("answer");
      else io->dd_color(v?"sw_on":"sw_off");
      io->tu_go(x,y+0); io->dd_printf("%c", p[0]);
      io->tu_go(x,y+1); io->dd_printf("%c", p[1]);
      io->tu_go(x,y+2); io->dd_printf("%c", p[2]);
      io->dd_color("answer");
      last= act;
    }
}

void
cl_sw::draw(void)
{
  class cl_hw_io *io= fpga->get_io();
  if (!io) return;
  io->tu_go(x,y+3);
  io->dd_cprintf("ui_mkey", "%c", key);
}

bool
cl_sw::handle_input(int c)
{
  cl_memory_cell *p= fpga->pjp;
  if ((c == key) || ((key=='y')&&(c=='z')))
    {
      if (p)
	{
	  t_mem v= p->R();
	  v^= mask;
	  p->W(v);
	}
      return true;
    }
  return false;
}


/*
                                                                Push button
  -------------------------------------------------------------------------
*/

cl_btn::cl_btn(class cl_fpga *the_fpga, int ax, int ay, int amask, char akey):
  cl_ibit(the_fpga, ax, ay, amask, akey)
{
}

void
cl_btn::refresh(bool force)
{
  char c= ' ';
  u32_t act, a, l, v= 0;
  class cl_hw_io *io= fpga->get_io();
  if (!io) return;
  class cl_memory_cell *p= fpga->pip;
  if (!p)
    {
      c= '?';
      act= last;
    }
  else
    {
      v= p->R();
      act= v;
      v&= mask;
      c= v?'-':'T';
    }
  a= act&mask;
  l= last&mask;
  if (force || (a != l))
    {
      if (!p) io->dd_color("answer");
      else io->dd_color(v?"btn_on":"btn_off");
      io->tu_go(x,y);
      io->dd_printf("_%c_", c);
      io->dd_color("answer");
      last= act;
    }
}

void
cl_btn::draw(void)
{
  class cl_hw_io *io= fpga->get_io();
  if (!io) return;
  io->tu_go(x+1,y+1);
  io->dd_cprintf("ui_mkey", "%c", key);
}

bool
cl_btn::handle_input(int c)
{
  cl_memory_cell *p= fpga->pip;
  if (c == key)
    {
      if (p)
	{
	  class cl_p1516 *u= (class cl_p1516 *)(fpga->uc);
	  t_mem v= p->R();
	  v^= mask;
	  p->W(v);
	  if (mask==1)
	    u->btn_edge(0, v&mask);
	  if (mask==2)
	    u->btn_edge(1, v&mask);
	}
      return true;
    }
  return false;
}


/*
                                                                       FPGA
  -------------------------------------------------------------------------
*/

cl_fpga::cl_fpga(class cl_uc *auc, int aid, chars aid_string):
  cl_hw(auc, HW_DUMMY, aid, aid_string)
{
  int i;
  for (i= 0; i<16; i++)
    leds[i]= NULL;
  for (i= 0; i<8; i++)
    segs[i]= NULL;
  for (i= 0; i<16; i++)
    sws[i]= NULL;
  for (i= 0; i<8; i++)
    btns[i]= NULL;
  pa= (class cl_cell32 *)register_cell(uc->rom, 0xff00);
  pb= (class cl_cell32 *)register_cell(uc->rom, 0xff01);
  pc= (class cl_cell32 *)register_cell(uc->rom, 0xff02);
  pd= (class cl_cell32 *)register_cell(uc->rom, 0xff03);
  pi= (class cl_cell32 *)register_cell(uc->rom, 0xff20);
  pj= (class cl_cell32 *)register_cell(uc->rom, 0xff10);
  bc= (class cl_cell32 *)register_cell(uc->rom, 0xfff0);
  if ((uc->symbol2cell((char*)"pi_pins", &pip)))
    {
      register_cell(pip);
    }
  if ((uc->symbol2cell((char*)"pj_pins", &pjp)))
    {
      register_cell(pjp);
    }
  basey= 13; // row of leds
  d2c_b= 7;
}


int
cl_fpga::init(void)
{
  cl_hw::init();
  mk_leds();
  mk_segs();
  mk_sws();
  mk_btns();
  return 0;
}


void
cl_fpga::make_io()
{
  if (!io)
    {
      io= new cl_hw_io(this);
      io->init();
      application->get_commander()->add_console(io);
    }
}


void
cl_fpga::new_io(class cl_f *f_in, class cl_f *f_out)
{
  cl_hw::new_io(f_in, f_out);
  io->tu_mouse_on();
  io->dd_printf("\033[2 q");
  if (f_in)
    f_in->set_escape(true);
}


bool
cl_fpga::handle_input(int c)
{
  int i;
  int ret;
  for (i=0; i<8; i++)
    if (btns[i])
      if (btns[i]->handle_input(c))
	return true;
  for (i=0; i<16; i++)
    if (sws[i])
      if (sws[i]->handle_input(c))
	return true;
  if (pjp)
    {
      u32_t sw= pjp->R();
      u32_t rx= sw&0xf;
      //printf("c=%8x\n",c);
      switch (c)
	{
	case 'A': sw&= ~0xf0; sw|= 0x00; pjp->W(sw); break;
	case 'B': sw&= ~0xf0; sw|= 0x10; pjp->W(sw); break;
	case 'C': sw&= ~0xf0; sw|= 0x20; pjp->W(sw); break;
	case 'D': sw&= ~0xf0; sw|= 0x30; pjp->W(sw); break;
	case 'R': sw&= ~0xf0; sw|= 0x90; pjp->W(sw); break;
	case 'S': sw&= ~0xf0; sw|= 0xa0; pjp->W(sw); break;
	case TU_UP  : sw&= ~0xf; sw|= ((rx+1)&0xf); pjp->W(sw); break;
	case TU_DOWN: sw&= ~0xf; sw|= ((rx-1)&0xf); pjp->W(sw); break;
	}
      
    }
  ret= cl_hw::handle_input(c); // handle default keys
  return ret;
}


void
cl_fpga::refresh_leds(bool force)
{
  int i;
  if (!io) return;
  for (i=0; i<16; i++)
    {
      if (leds[i])
	leds[i]->refresh(force);
    }
}


void
cl_fpga::refresh_segs(bool force)
{
  int i;
  if (!io) return;
  for (i=0; i<8; i++)
    {
      if (segs[i])
	segs[i]->refresh(force);
    }
}


void
cl_fpga::refresh_sws(bool force)
{
  int i;
  if (!io) return;
  for (i=0; i<16; i++)
    {
      if (sws[i])
	sws[i]->refresh(force);
    }
}


void
cl_fpga::refresh_btns(bool force)
{
  int i;
  if (!io) return;
  for (i=0; i<8; i++)
    {
      if (btns[i])
	btns[i]->refresh(force);
    }
}


void
cl_fpga::refresh_display(bool force)
{
  int i;
  
  if (!io) return;
  //io->tu_hide();
  refresh_leds(force);
  refresh_segs(force);
  refresh_sws(force);
  refresh_btns(force);
}


void
cl_fpga::draw_display(void)
{
  int i;
  if (!io) return;
  io->tu_hide();
  io->dd_color("led_on");
  io->tu_cls();
  cl_hw::draw_display();
  draw_fpga(); // board specific
  for (i=0; i<16; i++)
    if (leds[i])
      leds[i]->draw();
  for (i=0; i<8; i++)
    if (segs[i])
      segs[i]->draw();
  for (i=0; i<16; i++)
    if (sws[i])
      sws[i]->draw();
  for (i=0; i<8; i++)
    if (btns[i])
      btns[i]->draw();
  io->tu_go(2+8*5-9-7+3-16-6,basey-7);
  io->dd_cprintf("ui_mkey" , "[S]");
  io->dd_cprintf("ui_mitem", "eg ");
  io->dd_cprintf("ui_mkey" , "[ABCD] ");
  io->dd_cprintf("ui_mitem", "PX ");
  io->dd_cprintf("ui_mkey" , "[R] ");
  io->dd_cprintf("ui_mitem", "RX ");
  io->dd_cprintf("ui_mkey" , "[up] ");
  io->dd_cprintf("ui_mitem", "R+ ");
  io->dd_cprintf("ui_mkey" , "[dn] ");
  io->dd_cprintf("ui_mitem", "R- ");
  refresh_display(true);
}


t_mem
cl_fpga::read(class cl_memory_cell *cell)
{
  if (cell == pi)
    {
    }
  conf(cell, NULL);
  return cell->get();
}

void
cl_fpga::write(class cl_memory_cell *cell, t_mem *val)
{
  if (conf(cell, val))
    return;
  /*
  if (cell == pa)
    {
    }
  else if (cell == pb)
    {
      refresh_leds(false);
    }
  */
  cell->set(*val);
}


/*
                                                                 Nexys4 DDR
  -------------------------------------------------------------------------
*/

cl_n4::cl_n4(class cl_uc *auc, int aid, chars aid_string):
  cl_fpga(auc, aid, aid_string)
{
  board= "Nexys4DDR";
}


void
cl_n4::mk_leds(void)
{
  int i, m;
  for (i=0, m=1; i<16; i++, m<<=1)
    leds[i]= new cl_led(this, 2+16*3-i*3,basey, m);
}

void
cl_n4::mk_segs(void)
{
  int i, d;
  for (i=0, d=0; i<8; i++, d++)
    segs[i]= new cl_seg(this, 2+8*5-i*5,basey-6, d);
}

void
cl_n4::mk_btns(void)
{
  int d;
  // btnc
  btns[0]= new cl_btn(this, 2+16*3+5+5,basey-5, 1, '0');
  // btnd
  btns[1]= new cl_btn(this, 2+16*3+5+5,basey-2, 2, '1');
  // btnu
  btns[2]= new cl_btn(this, 2+16*3+5+5,basey-8, 4, '2');
  // btnr
  btns[3]= new cl_btn(this, 2+16*3+5+5+5,basey-5, 8, '3');
  // btnl
  btns[4]= new cl_btn(this, 2+16*3+5,basey-5, 16, '4');
}

void
cl_n4::mk_sws(void)
{
  const char *k= "asdfghjkqwertyui";
  int i, m;
  for (i=0, m=1; i<16; i++, m<<=1)
    sws[i]= new cl_sw(this, 2+16*3-i*3,basey+2, m, k[15-i]);
}

void
cl_n4::draw_fpga(void)
{
  int i;
  if (!io) return;
  io->tu_go(1,basey);
  io->dd_cprintf("ui_label", "PB=");
  io->tu_go(1,4);
  io->dd_printf("%s", board.c_str());
  for (i=0; i<16; i++)
    {
      io->tu_go(2+16*3-i*3-1,basey+1);
      io->dd_cprintf("ui_label", "%2d", i);
    }
}


/*
                                                                    Boolean
  -------------------------------------------------------------------------
*/

cl_bool::cl_bool(class cl_uc *auc, int aid, chars aid_string):
  cl_n4(auc, aid, aid_string)
{
  board= "Boolean";
}


void
cl_bool::draw_fpga(void)
{
  cl_n4::draw_fpga();
}


void
cl_bool::mk_btns(void)
{
  int d;
  /*
    0 1
    2 3
  */
  btns[0]= new cl_btn(this, 2+16*3+5+5  ,basey-5, 1, '0');
  btns[1]= new cl_btn(this, 2+16*3+5+5+5,basey-5, 2, '1');
  btns[2]= new cl_btn(this, 2+16*3+5+5  ,basey-2, 4, '2');
  btns[3]= new cl_btn(this, 2+16*3+5+5+5,basey-2, 8, '3');
}


/*
                                                                     LogSys
  -------------------------------------------------------------------------
*/

cl_logsys::cl_logsys(class cl_uc *auc, int aid, chars aid_string):
  cl_fpga(auc, aid, aid_string)
{
  board= "LogSys";
  d2c_b= 3;
}

void
cl_logsys::mk_leds(void)
{
  int i, m;
  for (i=0, m=1; i<8; i++, m<<=1)
    leds[i]= new cl_rgb(this, 2+16*3-i*3,basey+7, m);
}

void
cl_logsys::mk_segs(void)
{
  int i, d;
  for (i=0, d=0; i<4; i++, d++)
    segs[i]= new cl_seg(this, 2+8*5-i*5,basey-6, d);
}

void
cl_logsys::draw_fpga(void)
{
  int i;
  if (!io) return;
  io->tu_go(2+16*3-8*3-1,basey+7);
  io->dd_cprintf("ui_label", "PB=");
  io->tu_go(1,4);
  io->dd_printf("%s", board.c_str());
  for (i=0; i<8; i++)
    {
      io->tu_go(2+16*3-i*3-1,basey+1);
      io->dd_cprintf("ui_label", "%2d", i);
    }
}

void
cl_logsys::mk_btns(void)
{
  int d;
  /*
    3 2 1 0
  */
  btns[0]= new cl_btn(this, 2+16*3-3-9*3-0*5,basey+3, 1, '0');
  btns[1]= new cl_btn(this, 2+16*3-3-9*3-1*5,basey+3, 2, '1');
  btns[2]= new cl_btn(this, 2+16*3-3-9*3-2*5,basey+3, 4, '2');
  btns[3]= new cl_btn(this, 2+16*3-3-9*3-3*5,basey+3, 8, '3');
}


void
cl_logsys::mk_sws(void)
{
  const char *k= "qwertyui";
  int i, m;
  for (i=0, m=1; i<8; i++, m<<=1)
    sws[i]= new cl_sw(this, 2+16*3-i*3,basey+2, m, k[7-i]);
}

/* End of p1516.src/fpga.cc */
