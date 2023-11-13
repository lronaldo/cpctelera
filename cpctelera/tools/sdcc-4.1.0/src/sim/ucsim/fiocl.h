/*
 * Simulator of microcontrollers (fiocl.h)
 *
 * Copyright (C) 1997,16 Drotos Daniel, Talker Bt.
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

#ifndef FIOCL_HEADER
#define FIOCL_HEADER

#include <stdio.h>
#include <stdarg.h>

#include "charscl.h"
#include "pobjcl.h"


enum file_type {
  F_UNKNOWN,
  F_FILE,
  F_CHAR,
  F_SOCKET,
  F_LISTENER,
  F_PIPE,
  F_CONSOLE, // win only
  F_SERIAL // win only
};

enum tu_special_keys {
  TU_UP		= -10,
  TU_DOWN	= -11,
  TU_LEFT	= -12,
  TU_RIGHT	= -13,
  TU_HOME	= -14,
  TU_END	= -15,
  TU_PGUP	= -16,
  TU_PGDOWN	= -17,
  TU_DEL	= -18,
  TU_F1		= -19,
  TU_F2		= -20,
  TU_F3		= -21,
  TU_F4		= -22,
  TU_F5		= -23,
  TU_F6		= -24,
  TU_F7		= -25,
  TU_F8		= -26,
  TU_F9		= -27,
  TU_F10	= -28,
  TU_F11	= -29,
  TU_F12	= -30,
  TU_INS	= -31,

  // mouse reports in 4 bytes: FF X Y Code
  TU_BTN1	= -50, // button1
  TU_BTN2	= -51, // button2
  TU_BTN3	= -52, // button3
  TU_CBTN1	= -53, // CTRL-button1
  TU_CBTN2	= -54, // CTRL-button2
  TU_CBTN3	= -55, // CTRL-button3
  TU_ABTN1	= -56, // ALT-button1
  TU_ABTN2	= -57, // ALT-button2
  TU_ABTN3	= -58, // ALT-button3
  TU_SUP	= -59, // Scroll-UP
  TU_SDOWN	= -60, // Scroll-DOWN
  TU_CSUP	= -61, // CTRL-Scroll-UP
  TU_CSDOWN	= -62, // CTRL-Scroll-DOWN
};


/* History */

class cl_history: public cl_ustrings
{
 protected:
  int nr;
  //chars actual_line;
 public:
  cl_history(char *aname);
  cl_history(const char *aname);
  virtual ~cl_history(void);

 public:
  const char *up(chars line);
  const char *down(chars line);
  void enter(chars line);
  void replace(chars line);
};


/* General file */

class cl_f: public cl_base
{
 public:
  int file_id;
  bool tty;
  enum file_type type;
 protected:
  chars file_name, file_mode;
  bool cooking;
  class cl_f *echo_to, *echo_of;
  chars echo_color;
  bool own;
  int at_end;
  char line[1024];
  int cursor;
  char esc_buffer[100];
  char last_ln;
  int buffer[1024];
  int last_used, first_free;
  bool attributes_saved;
  class cl_history *hist;
  bool proc_telnet; // in raw mode
  bool proc_escape; // in raw mode
 public:
  cl_f(void);
  cl_f(chars fn, chars mode);
  cl_f(int the_server_port);
  virtual ~cl_f(void);
  virtual class cl_f *copy(chars mode);
  virtual int init(void);
  //virtual int open(void) { return init(); }
  virtual int open(const char *fn);
  virtual int open(const char *fn, const char *mode);
  virtual int use_opened(int opened_file_id, const char *mode);
  virtual int own_opened(int opened_file_id, const char *mode);
  virtual int use_opened(FILE *f, const char *mode);
  virtual int own_opened(FILE *f, const char *mode);
  virtual enum file_type determine_type(void)= 0;
  virtual void changed(void);
  virtual int close(void);
  virtual int stop_use(void);
  virtual bool opened(void) { return file_id >= 0; }
  
  virtual const char *get_file_name() { return file_name; };
  virtual const char *get_fname() { return file_name; };
  virtual class cl_f *get_echo_to() { return echo_to; }
 protected:
  virtual int put(int c);
  virtual int get(void);
  virtual int free_place(void);
  virtual int finish_esc(int k);
  virtual int process_telnet(unsigned char ci);
  virtual int process_esc(char c);
  virtual int process_csi(void);
  virtual int process(char c);
  virtual int pick(void);
  virtual int pick(char c);
  virtual int pick(const char *s);
 public:
  virtual int input_avail(void);
  virtual int read(int *buf, int max);
  virtual int get_c(void);
  virtual chars get_s(void);
  
 public:
  //FILE *f(void) { return file_f; };
  int id(void) { return file_id; };

  virtual int check_dev(void)= 0;
  virtual int read_dev(int *buf, int max);
  virtual int write(const char *buf, int count);
  virtual int write_str(const char *s);
  virtual int vprintf(const char *format, va_list ap);
  virtual int prntf(const char *format, ...);
  virtual bool eof(void);
  //virtual void flush(void);

  virtual void echo_cursor_save();
  virtual void echo_cursor_restore();
  virtual void echo_cursor_go_left(int n);
  virtual void echo_cursor_go_right(int n);
  virtual void echo_write(const char *b, int l);
  virtual void echo_write_str(const char *s);
  virtual void set_echo_color(chars col);
  virtual chars get_echo_color() { return echo_color; }
  
  virtual void prepare_terminal();
  virtual void save_attributes();
  virtual void restore_attributes();
  virtual int raw(void);
  virtual int cooked(void);
  virtual void check(void) { return; }
  virtual int echo(class cl_f *out);
  virtual void interactive(class cl_f *echo_out);
  virtual int get_cooking() { return cooking; }
  virtual void set_telnet(bool val);
  virtual void set_escape(bool val);
  virtual bool get_telnet() { return proc_telnet; }
  virtual bool get_escape() { return proc_escape; }
 public:
  int server_port;

 public:
  //virtual int listen(int on_port);
  //virtual class cl_f *accept();
  //virtual int connect(chars host, int to_port);
};

//extern void deb(const char *format, ...);

extern int mk_srv_socket(int port);

extern class cl_f *mk_io(const char *fn, const char *mode);
extern class cl_f *cp_io(/*FILE *f*/int file_id, const char *mode);
extern class cl_f *mk_srv(int server_port);
extern int srv_accept(class cl_f *listen_io,
		      class cl_f **fin, class cl_f **fout);

extern bool check_inputs(class cl_list *active, class cl_list *avail);

extern void msleep(int msec);
extern void loop_delay();

extern const char *fio_type_name(enum file_type t);
extern void  sigpipe_off();


#endif

/* End of fiocl.h */
