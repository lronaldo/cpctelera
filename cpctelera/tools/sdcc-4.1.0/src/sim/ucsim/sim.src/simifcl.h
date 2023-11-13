/*
 * Simulator of microcontrollers (sim.src/simifcl.h)
 *
 * Copyright (C) 2016,16 Drotos Daniel, Talker Bt.
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

#ifndef SIMIFCL_HEADER
#define SIMIFCL_HEADER

// prj
#include "fiocl.h"

// local
#include "uccl.h"
#include "hwcl.h"


#define SIMIF_VERSION	1

enum sif_command {
  DETECT_SIGN	        = '!',	// answer to detect command
  
  SIFCM_DETECT		= '_',	// detect the interface
  // -> _
  // <- !
  SIFCM_COMMANDS	= 'i',	// get info about commands
  // -> i
  // <- nr list of command chars
  SIFCM_IFVER		= 'v',	// interface version
  // -> v
  // <- 1 SIMIF_VERSION
  SIFCM_SIMVER		= 'V',	// simulator version
  // -> V
  // <- len VERSIONSTR
  SIFCM_IFRESET		= '@',	// reset the interface
  // ?
  // ?
  SIFCM_CMDINFO		= 'I',	// info about a command
  // -> I cmdchar
  // <- 2 params_needed answer_type
  SIFCM_CMDHELP		= 'h',	// help about a command
  // -> h cmdchar
  // <- string_length+1 string_of_help 0
  SIFCM_STOP		= 's',	// stop simulation
  // -> s
  // -> s
  SIFCM_PRINT		= 'p',	// print out a character
  // -> p char
  // <-
  SIFCM_HEX		= 'x',	// print out a character in hex
  // -> x char
  // <-
  SIFCM_FIN_CHECK	= 'f',	// check input file for input
  // -> f
  // <- 0|1
  SIFCM_READ		= 'r',	// read from input file
  // -> r
  // <- char|0
  SIFCM_WRITE		= 'w',	// write to output file
  // -> w char
  // <-
  SIFCM_RESET		= 'R',	// reset CPU
  // -> R
  // <-
};

enum sif_answer_type {
  SIFAT_UNKNOWN		= 0x00,	// we don't know...
  SIFAT_BYTE		= 0x01,	// just a byte
  SIFAT_ARRAY		= 0x02,	// array of some bytes
  SIFAT_STRING		= 0x03,	// a string
  SIFAT_NONE		= 0x04	// no answer at all
};

enum simif_cfg {
  simif_on		= 0,  // RW
  simif_run		= 1,  // RW
  simif_start		= 2,  // RW
  simif_stop		= 3,  // RW
  simif_quit		= 4,  // W
  simif_reason		= 5,  // R
  simif_xtal		= 6,  // RW
  simif_ticks		= 7,  // R
  simif_isr_ticks	= 8,  // R
  simif_idle_ticks	= 9,  // R
  simif_real_time	= 10, // R
  simif_vclk		= 11, // R
  simif_pc		= 12, // RW
  simif_print		= 13, // W
  simif_write		= 14, // W
  
  simif_nuof		= 15
};

class cl_simulator_interface;

/* Base of commands */

class cl_sif_command: public cl_base
{
protected:
  enum sif_command command;
  char *description;
  enum sif_answer_type answer_type;
  class cl_simulator_interface *sif;
  t_mem *parameters;
  int params_needed, nuof_params, params_received;
  t_mem *answer;
  int answer_length, answered_bytes;
  bool answering;
public:
  cl_sif_command(enum sif_command cmd,
		 const char *the_name,
		 const char *the_description,
		 enum sif_answer_type the_answer_type,
		 int the_params_needed,
		 class cl_simulator_interface *the_sif);
  virtual ~cl_sif_command(void);
  virtual int init(void);
  virtual void clear_params(void);
  virtual void clear_answer(void);
  virtual void clear(void);

  enum sif_command get_command(void) { return(command); }
  char *get_description(void) { return(description); }
  int get_nuof_params(void) { return(nuof_params); }
  int get_params_received(void) { return(params_received); }
  int get_answer_length(void) { return(answer_length); }
  int get_answered_bytes(void) { return(answered_bytes); }
  bool get_answering(void) { return(answering); }
  enum sif_answer_type get_answer_type(void) { return(answer_type); }
  int get_params_needed(void) { return(params_needed); }
  bool get_parameter(int nr, t_mem *into);

  virtual t_mem read(class cl_memory_cell *cel);
  virtual void write(class cl_memory_cell *cel, t_mem *val);

  virtual void start(void);
  virtual void produce_answer(void);

private:
  virtual void need_params(int nr);
public:
  virtual void set_answer(t_mem ans);
  virtual void set_answer(int nr, t_mem ans[]);
  virtual void set_answer(const char *ans);
  virtual void start_answer(void);
};

/* Command: detect */
class cl_sif_detect: public cl_sif_command
{
public:
  cl_sif_detect(class cl_simulator_interface *the_sif):
    cl_sif_command(SIFCM_DETECT, "if_detect",
		   "Detect existence of interface",
		   SIFAT_BYTE, 0, the_sif)
  {}
  virtual void produce_answer(void) { set_answer(t_mem(DETECT_SIGN)); }
};

/* Command: interface version */
class cl_sif_ifver: public cl_sif_command
{
public:
  cl_sif_ifver(class cl_simulator_interface *the_sif):
    cl_sif_command(SIFCM_IFVER, "if_ver",
		   "Get version of simulator interface",
		   SIFAT_BYTE, 0, the_sif)
  {}
  virtual void produce_answer(void) { set_answer(t_mem(SIMIF_VERSION)); }
};

/* Command: simulator version */
class cl_sif_simver: public cl_sif_command
{
public:
  cl_sif_simver(class cl_simulator_interface *the_sif):
    cl_sif_command(SIFCM_SIMVER, "sim_ver",
		   "Get version of simulator",
		   SIFAT_STRING, 0, the_sif)
  {}
  virtual void produce_answer(void) { set_answer(VERSIONSTR); }
};

/* Command: reset interface */
class cl_sif_ifreset: public cl_sif_command
{
public:
  cl_sif_ifreset(class cl_simulator_interface *the_sif):
    cl_sif_command(SIFCM_IFRESET, "if_reset",
		   "Reset interface to default state",
		   SIFAT_NONE, 0, the_sif)
  {}
};

/* Command: get info about commands */
class cl_sif_commands: public cl_sif_command
{
public:
  cl_sif_commands(class cl_simulator_interface *the_sif):
    cl_sif_command(SIFCM_COMMANDS, "commands",
		   "Get information about known commands",
		   SIFAT_ARRAY, 0, the_sif)
  {}
  virtual void produce_answer(void);
};

/* Command: get info about a command */
class cl_sif_cmdinfo: public cl_sif_command
{
public:
  cl_sif_cmdinfo(class cl_simulator_interface *the_sif):
    cl_sif_command(SIFCM_CMDINFO, "cmdinfo",
		   "Get information about a command",
		   SIFAT_ARRAY, 1, the_sif)
  {}
  virtual void produce_answer(void);
};

/* Command: get info about a command */
class cl_sif_cmdhelp: public cl_sif_command
{
public:
  cl_sif_cmdhelp(class cl_simulator_interface *the_sif):
    cl_sif_command(SIFCM_CMDHELP, "cmdhelp",
		   "Get help about a command",
		   SIFAT_STRING, 1, the_sif)
  {}
  virtual void produce_answer(void);
};

/* Command: stop simulation */
class cl_sif_stop: public cl_sif_command
{
public:
  cl_sif_stop(class cl_simulator_interface *the_sif):
    cl_sif_command(SIFCM_STOP, "stop",
		   "Stop simulation",
		   SIFAT_BYTE, 0, the_sif)
  {}
  virtual void produce_answer(void);
};

/* Command: print character */
class cl_sif_print: public cl_sif_command
{
public:
  cl_sif_print(class cl_simulator_interface *the_sif):
    cl_sif_command(SIFCM_PRINT, "print",
		   "Print character",
		   SIFAT_NONE, 1, the_sif)
  {}
  virtual void produce_answer(void);
};


/* Command: print character */
class cl_sif_hex: public cl_sif_command
{
public:
  cl_sif_hex(class cl_simulator_interface *the_sif):
    cl_sif_command(SIFCM_HEX, "print_hex",
		   "Print character in hex",
		   SIFAT_NONE, 1, the_sif)
  {}
  virtual void produce_answer(void);
};


/* Command: write character to output file */
class cl_sif_write: public cl_sif_command
{
public:
  cl_sif_write(class cl_simulator_interface *the_sif):
    cl_sif_command(SIFCM_WRITE, "write to output file",
		   "Write character to output file",
		   SIFAT_NONE, 1, the_sif)
  {}
  virtual void produce_answer(void);
};


/* Command: check input file */
class cl_sif_fin_check: public cl_sif_command
{
public:
  cl_sif_fin_check(class cl_simulator_interface *the_sif):
    cl_sif_command(SIFCM_FIN_CHECK, "fin_check",
		   "Check input file if input available",
		   SIFAT_BYTE, 0, the_sif)
  {}
  virtual void produce_answer(void);
};


/* Command: read input file */
class cl_sif_read: public cl_sif_command
{
public:
  cl_sif_read(class cl_simulator_interface *the_sif):
    cl_sif_command(SIFCM_READ, "read input file",
		   "Read character from input file",
		   SIFAT_BYTE, 0, the_sif)
  {}
  virtual void produce_answer(void);
};


/* Command: reset */
class cl_sif_reset: public cl_sif_command
{
public:
  cl_sif_reset(class cl_simulator_interface *the_sif):
    cl_sif_command(SIFCM_RESET, "reset cpu",
		   "Reset CPU",
		   SIFAT_NONE, 0, the_sif)
  {}
  virtual void produce_answer(void);
};


/*
 * Virtual hardware: simulator interface
 */

class cl_simulator_interface: public cl_hw
{
 private:
  int version;
  const char *as_name;
  t_addr addr;
  class cl_address_space *as;
  t_addr address;
  class cl_memory_cell *cell;
  class cl_sif_command *active_command;
 public:
  class cl_f *fin, *fout;
 public:
  class cl_list *commands;
 public:
  cl_simulator_interface(class cl_uc *auc);
  virtual ~cl_simulator_interface(void);
  virtual int init(void);
  virtual int cfg_size(void) { return simif_nuof; }
  virtual const char *cfg_help(t_addr addr);
    
  virtual void set_cmd(class cl_cmdline *cmdline, class cl_console_base *con);
  virtual t_mem read(class cl_memory_cell *cel);
  virtual void write(class cl_memory_cell *cel, t_mem *val);
  virtual t_mem conf_op(cl_memory_cell *cell, t_addr addr, t_mem *val);

  virtual void finish_command(void);

  virtual void print_info(class cl_console_base *con);
};


#endif
