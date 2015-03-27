/*
 * Simulator of microcontrollers (serialcl.h)
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

#ifndef SERIALCL_HEADER
#define SERIALCL_HEADER

#include "stypes.h"
#include "pobjcl.h"
#include "uccl.h"

//#include "newcmdcl.h"


class cl_serial: public cl_hw
{
protected:
  class cl_address_space *sfr;
  bool there_is_t2, t2_baud;
  class cl_memory_cell *sbuf, *pcon, *scon;
#ifdef HAVE_TERMIOS_H
  struct termios saved_attributes_in; // Attributes of serial interface
  struct termios saved_attributes_out;
#endif
  class cl_optref *serial_in_file_option;
  class cl_optref *serial_out_file_option;
  FILE *serial_in;      // Serial line input
  FILE *serial_out;     // Serial line output
  uchar s_in;           // Serial channel input reg
  uchar s_out;          // Serial channel output reg
  bool  s_sending;      // Transmitter is working
  bool  s_receiving;    // Receiver is working
  int   s_rec_bit;      // Bit counter of receiver
  int   s_tr_bit;       // Bit counter of transmitter
  int   s_rec_t1;       // T1 overflows for receiving
  int   s_tr_t1;        // T1 overflows for sending
  int   s_rec_tick;     // Machine cycles for receiving
  int   s_tr_tick;      // Machine cycles for sending
  uchar _mode;
  uchar _bmREN;
  uchar _bmSMOD;
  uchar _bits;
  uchar _divby;
public:
  cl_serial(class cl_uc *auc);
  virtual ~cl_serial(void);
  virtual int init(void);

  virtual void new_hw_added(class cl_hw *new_hw);
  virtual void added_to_uc(void);
  virtual t_mem read(class cl_memory_cell *cell);
  virtual void write(class cl_memory_cell *cell, t_mem *val);

  //virtual void mem_cell_changed(class cl_m *mem, t_addr addr);

  virtual int  serial_bit_cnt(void);
  virtual void received(int c);

  virtual int tick(int cycles);
  virtual void reset(void);
  virtual void happen(class cl_hw *where, enum hw_event he, void *params);

  virtual void print_info(class cl_console_base *con);
};


#endif

/* End of s51.src/serialcl.h */
