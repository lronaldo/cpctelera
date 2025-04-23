/*
 * Simulator of microcontrollers (vcd.cc)
 *
 * Copyright (C) 2017 Drotos Daniel
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

#include "ddconfig.h"

#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#include <errno.h>
#include <fcntl.h>
//#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

// prj
#include "stypes.h"
#include "utils.h"
#include "globals.h"

// sim
#include "appcl.h"
#include "argcl.h"
#include "simcl.h"

// local
#include "vcdcl.h"


static i64_t ULPs(double d1, double d2)
{
  i64_t i1, i2;

  memcpy(&i1, &d1, sizeof(d1));
  memcpy(&i2, &d2, sizeof(d2));

  if ((i1 < 0) != (i2 < 0))
    return
#ifdef INT64_MAX
      INT64_MAX
#else
      ((u64_t)-1)>>1
#endif
      ;

  return (i1 > i2 ? i1 - i2 : i2 - i1);
}


#if defined(HAVE_WORKING_FORK) && defined(HAVE_DUP2) && defined(HAVE_PIPE) && defined(HAVE_WAITPID)

#  if !defined(HAVE__EXIT)
static void _exit(int status) { exit(status); }
#  endif

#  if !defined(O_CLOEXEC)
#    define O_CLOEXEC 0
#  endif

/*
#  if !defined(HAVE_DUP3)
static int dup3(int oldfd, int newfd, int flags) { return dup2(oldfd, newfd); }
#  endif
*/

static int spipe2(int pipefd[2], int flags) { return pipe(pipefd); }

#endif

class cl_vcd_var: public cl_memory_operator
{
 public:
  class cl_vcd_var *next_var;
  class cl_vcd *vcd;
  int bitnr_high, bitnr_low;
  t_mem bitmask;
  char var_id;

  cl_vcd_var(class cl_vcd *vcd, char vcd_id, class cl_memory_cell *cell, int bitnr_high, int bitnr_low);
  ~cl_vcd_var(void);

  t_mem write(t_mem val);
};


cl_vcd_var::cl_vcd_var(class cl_vcd *vcd, char var_id, class cl_memory_cell *cell, int bitnr_high, int bitnr_low):
  cl_memory_operator(cell)
{
  set_name(vcd->get_name());

  this->next_var = NULL;
  this->vcd = vcd;
  this->bitnr_high = bitnr_high;
  this->bitnr_low = bitnr_low;

  bitmask = ((1U << (bitnr_high - bitnr_low + 1)) - 1) << bitnr_low;

  if (!(this->var_id = var_id))
    {
      this->var_id = vcd->get_next_var_id();
      cell->append_operator(this);
    }
}


cl_vcd_var::~cl_vcd_var(void)
{
  cell->remove_operator(this);
}


t_mem
cl_vcd_var::write(t_mem val)
{
  if (vcd->is_running() /*&& (val & bitmask) != (cell->get() & bitmask)*/)
    vcd->report(this, val);

  return cl_memory_operator::write(val);
}


cl_vcd::cl_vcd(class cl_uc *auc, int aid, chars aid_string):
       cl_hw(auc, HW_DUMMY, aid, aid_string)
{
  filename= NULL;
  fd= 0;
  vars= NULL;
  reset_next_var_id();
  starttime= 0;
  timescale= 0;
  event= 0;
  pausetime= -1;
  state= -1;
  started= false;
  paused= false;
  dobreak= false;
  modul= chars("", "ucsim_vcd_%d", id);
}

int
cl_vcd::init(void)
{
  cl_hw::init();
  on= false;

  return 0;
}

void
cl_vcd::clear_vars(void)
{
  while (vars)
    {
      class cl_vcd_var *old = vars;
      vars = vars->next_var;
      delete old;
    }
}

void
cl_vcd::add_var(class cl_console_base *con, char var_id, class cl_memory_cell *cell, int bitnr_high, int bitnr_low)
{
  if (!cell)
    return;

  if (cell->get_flag(CELL_NON_DECODED))
    {
      if (con) con->dd_printf("Cell is not decoded\n");
      return;
    }

  if (started)
    {
      if (con) con->dd_printf("Already started\n");
      return;
    }

  if (bitnr_high == -1)
    bitnr_high = cell->get_width() - 1;

  if (bitnr_low == -1)
    bitnr_low = 0;

  class cl_vcd_var **var_p;

  for (var_p = &vars; *var_p; var_p = &((*var_p)->next_var))
    {
      if ((*var_p)->cell == cell &&
          (*var_p)->bitnr_high == bitnr_high && (*var_p)->bitnr_low == bitnr_low)
        {
          if (con) con->dd_printf("Already exists\n");
          return;
        }
    }

  *var_p = new cl_vcd_var(this, var_id, cell, bitnr_high, bitnr_low);
}

void
cl_vcd::del_var(class cl_console_base *con, class cl_memory_cell *cell, int bitnr_low, int bitnr_high)
{
  for (class cl_vcd_var **var_p = &vars; *var_p; var_p = &((*var_p)->next_var))
    {
      if ((*var_p)->cell == cell)
        {
          if ((*var_p)->bitnr_high == bitnr_high && (*var_p)->bitnr_low == bitnr_low)
            {
              class cl_vcd_var *old = *var_p;
              *var_p = (*var_p)->next_var;
              delete old;

              if (vars == NULL && !started)
                reset_next_var_id();

              return;
            }
        }
    }

  if (con) con->dd_printf("Not found\n");
}

void
cl_vcd::add_var(class cl_console_base *con, class cl_memory *m, t_addr a, int bitnr_high, int bitnr_low)
{
  if (!m->is_address_space())
    {
      if (con) con->dd_printf("Not an address space\n");
      return;
    }
  if (!m->valid_address(a))
    {
      if (con) con->dd_printf("Address must be between 0x%x and 0x%x\n",
                              AU(m->lowest_valid_address()),
                              AU(m->highest_valid_address()));
      return;
    }
  cl_memory_cell *c= ((cl_address_space*)m)->get_cell(a);
  add_var(con, 0, c, bitnr_high, bitnr_low);
}

void
cl_vcd::del_var(class cl_console_base *con, class cl_memory *m, t_addr a, int bitnr_low, int bitnr_high)
{
  if (!m->is_address_space())
    {
      if (con) con->dd_printf("%s is not an address space\n");
      return;
    }
  if (!m->valid_address(a))
    {
      if (con) con->dd_printf("Address must be between 0x%x and 0x%x\n",
                              AU(m->lowest_valid_address()),
                              AU(m->highest_valid_address()));
      return;
    }
  del_var(con, ((cl_address_space*)m)->get_cell(a), bitnr_low, bitnr_high);
}

FILE *
cl_vcd::open_vcd(class cl_console_base *con)
{
  if (state != -1)
    {
      if (filename)
        {
          if ((fd= fopen(filename, "re")) == NULL)
            con->dd_printf("%s: %s\n", filename, strerror(errno));
        }
      else
        con->dd_printf("No file specified\n");
    }
  else if (filename && filename[0] != '|')
    {
      if ((fd= fopen(filename, "we")) == NULL)
        con->dd_printf("%s: %s\n", filename, strerror(errno));
    }
  else
    {
#if defined(HAVE_WORKING_FORK) && defined(HAVE_DUP2) && defined(HAVE_PIPE) && defined(HAVE_WAITPID)
      int p[2];
      pid_t pid;

      if (!spipe2(p, O_CLOEXEC))
        {
          if ((pid = fork()) > 0)
            {
              // Parent
              if (!(fd = fdopen(p[1], "we")))
                {
                  close(p[1]);
                  con->dd_printf("fdopen: %s\n", strerror(errno));
                }
              close(p[0]);

              // Reap the intermediate child. The real child is no longer
              // under our control.
              waitpid(pid, NULL, 0);
            }
          else if (pid == 0)
            {
              // Child
              // Double fork to break the parent-child relationship
              if (fork() != 0)
                _exit(0);

              dup2(p[0], 0);
              close(p[0]);
              close(p[1]);

              const char *cmd = filename;
              if (!cmd)
                cmd = "exec > /dev/null 2>&1; shmidcat | gtkwave -v -I";
              else if (cmd[0] == '|')
                cmd++;

              execl("/bin/sh", "sh", "-c", cmd, (char*)NULL);
              perror("execl");
              _exit(1);
            }
          else
            {
              con->dd_printf("fork: %s\n", strerror(errno));
              close(p[0]);
              close(p[1]);
            }
        }
      else
        con->dd_printf("pipe: %s\n", strerror(errno));
#else
      con->dd_printf("Piped output (e.g. to gtkwave) is not supported on this platform\n");
#endif
    }

  return fd;
}

void
cl_vcd::close_vcd(void)
{
  fclose(fd);
}

bool
cl_vcd::parse_header(cl_console_base *con)
{
  int width = 0;
  char id = 0;

  clear_vars();
  state = 0;
  word[0] = 'X';
  word[1] = ' ';
  char *token = &word[2];

  while (read_word(2U))
    {
      if (!strcmp(token, "$end"))
        state = 0;
      else if (state == 0)
        {
          if (token[0] == '#')
            {
              event = starttime + timescale * strtod(&token[1], NULL);
              //uc->sim->app->debug("vcd[%d]: first event at %.15f\n", cl_hw::id, event);
              break;
            }
          else if (!strcmp(token, "$timescale"))
            state = 2;
          else if (!strcmp(token, "$dumpvars"))
            state = 3;
          else if (!strcmp(token, "$var"))
             state = 4;
        }
      else if (state == 2) // $timescale
        {
          int i = 0;
          if (token[0] >= '0' && token[0] <= '9')
            sscanf(token, "%lf%n", &timescale, &i);
          if (!strcmp(&token[i], "fs"))
            timescale *= 1e-15;
          else if (!strcmp(&token[i], "ps"))
            timescale *= 1e-12;
          else if (!strcmp(&token[i], "ns"))
            timescale *= 1e-09;
          else if (!strcmp(&token[i], "us"))
            timescale *= 1e-06;
          else if (!strcmp(&token[i], "ms"))
            timescale *= 1e-03;
        }
      else if (state == 3) // $dumpvars
        // FIXME: does it make sense to set the initial state of all vars?
        ; // set_mem_from_var(token); // FIXME: not implemented yet
      else if (state == 4) // $var [type]
        state = (!strcmp(token, "wire") ? 5 : 8);
      else if (state == 5) // $var [type] [bitwidth]
        {
          width = strtol(token, NULL, 0);
          state = 6;
        }
      else if (state == 6) // $var [type] [bitwidth] [id]
        {
          id = token[0];
          state = 7;
        }
      else if (state == 7) // $var [type] [bitwidth] [id] [name]
        {
          class cl_cmdline *cmdline = new cl_cmdline(NULL, word, con);
          cmdline->init();

          if (cmdline->params->count != 1)
            {
              con->dd_printf("Unable to interpret %s, ignoring\n", token);
              continue;
            }

          class cl_cmd_arg *param = (class cl_cmd_arg *)(cmdline->params->at(0));

          if (param->as_bit(uc))
            {
              if (param->value.bit.mem->is_address_space())
                add_var(con, id, ((cl_address_space *)param->value.bit.mem)->get_cell(param->value.bit.mem_address), param->value.bit.bitnr_low + width - 1, param->value.bit.bitnr_low);
              else
                con->dd_printf("Unable to interpret %s as a memory/bit reference in an address space\n", token);
            }
          else if (param->as_cell(uc))
            add_var(con, id, param->value.cell, width - 1, 0);
          else
            con->dd_printf("Unable to interpret %s as a memory/bit reference\n", token);

          state = 8;
        }
    }

  return true;
}

bool
cl_vcd::read_word(unsigned int i)
{
  int c;

  do
    {
      c = getc(fd);
    }
  while (c != EOF && strchr(" \n\r\t", c));

  if (c != EOF)
    {
      do
        {
          if (i < sizeof(word) - 1)
            word[i++] = c;
          c = getc(fd);
        }
      while (c > 0 && !strchr(" \n\r\t", c));
    }
  else
    on = false;

  word[i] = '\0';
  return (c != EOF);
}

bool
cl_vcd::set_cmd(class cl_cmdline *cmdline, class cl_console_base *con)
{
  class cl_cmd_arg *params[5]= {
    cmdline->param(0),
    cmdline->param(1),
    cmdline->param(2),
    cmdline->param(3),
    cmdline->param(4)
  };

  /*
  if (cmdline->syntax_match(uc, BIT)) // ADD
    {
      add_var(con, params[0]->value.bit.mem, params[0]->value.bit.mem_address, params[0]->value.bit.bitnr_high, params[0]->value.bit.bitnr_low);
      return;
    }
  else if (cmdline->syntax_match(uc, MEMORY ADDRESS NUMBER NUMBER)) // ADD
    {
      add_var(con, params[0]->value.memory.memory, params[1]->value.address, params[2]->value.number, params[3]->value.number);
      return;
    }
  else if (cmdline->syntax_match(uc, MEMORY ADDRESS NUMBER)) // ADD
    {
      add_var(con, params[0]->value.memory.memory, params[1]->value.address, params[2]->value.number, params[2]->value.number);
      return;
    }
  else if (cmdline->syntax_match(uc, MEMORY ADDRESS)) // ADD
    {
      add_var(con, params[0]->value.memory.memory, params[1]->value.address, -1, -1);
      return;
    }
  else if (cmdline->syntax_match(uc, CELL NUMBER NUMBER)) // ADD
    {
      add_var(con, params[0]->value.cell, params[1]->value.number, params[2]->value.number);
      return;
    }
  else if (cmdline->syntax_match(uc, CELL NUMBER)) // ADD
    {
      add_var(con, params[0]->value.cell, params[1]->value.number, params[1]->value.number);
      return;
    }
  else if (cmdline->syntax_match(uc, CELL)) // ADD
    {
      add_var(con, params[0]->value.cell, -1, -1);
      return;
    }
  else
  */
  if (0)
    ;
  else if (cmdline->syntax_match(uc, STRING CELL)) // DEL|ADD
    {
      char *p1= params[0]->value.string.string;
      if (p1 && *p1)
        {
          if (strcmp(p1, "add") == 0)
            add_var(con, params[1]->value.cell, -1, -1);
          else if (strstr(p1, "del") == p1)
            del_var(con, params[1]->value.cell, -1, -1);
          return true;
        }
      return false;
    }
  else if (cmdline->syntax_match(uc, STRING BIT)) // DEL|ADD
    {
      char *p1= params[0]->value.string.string;
      if (p1 && *p1)
        {
          if (strcmp(p1, "add") == 0)
            {
              add_var(con, params[1]->value.bit.mem, params[1]->value.bit.mem_address, params[1]->value.bit.bitnr_high, params[1]->value.bit.bitnr_low);
              return true;
            }
          else if (strstr(p1, "del") == p1)
            {
              del_var(con, params[1]->value.bit.mem, params[1]->value.bit.mem_address, params[1]->value.bit.bitnr_low, params[1]->value.bit.bitnr_high);
              return true;
            }
          else if (strcmp(p1, "new") == 0)
            {
              int nid= params[1]->as_number();
              if (uc->get_hw(id_string, nid, NULL) != NULL)
                {
                  con->dd_printf("Already exists\n");
                  return true;
                }
              cl_hw *h= new cl_vcd(uc, nid, id_string);
              h->init();
              uc->add_hw(h);
              return true;
            }
        }
      return false;
    }
  else if (cmdline->syntax_match(uc, STRING MEMORY ADDRESS NUMBER NUMBER)) // DEL|ADD
    {
      char *p1= params[0]->value.string.string;
      if (p1 && *p1)
        {
          if (strcmp(p1, "add") == 0)
            add_var(con, params[1]->value.memory.memory, params[2]->value.address, params[3]->value.number, params[4]->value.number);
          else if (strstr(p1, "del") == p1)
            del_var(con, params[1]->value.memory.memory, params[2]->value.address, params[3]->value.number, params[4]->value.number);
          return true;
        }
      return false;
    }
  else if (cmdline->syntax_match(uc, STRING MEMORY ADDRESS NUMBER)) // DEL|ADD
    {
      char *p1= params[0]->value.string.string;
      if (p1 && *p1)
        {
          if (strcmp(p1, "add") == 0)
            add_var(con, params[1]->value.memory.memory, params[2]->value.address, params[3]->value.number, params[3]->value.number);
          else if (strstr(p1, "del") == p1)
            del_var(con, params[1]->value.memory.memory, params[2]->value.address, params[3]->value.number, params[3]->value.number);
          return true;
        }
      return false;
    }
  else if (cmdline->syntax_match(uc, STRING MEMORY ADDRESS)) // DEL|ADD
    {
      char *p1= params[0]->value.string.string;
      if (p1 && *p1)
        {
          if (strcmp(p1, "add") == 0)
            add_var(con, params[1]->value.memory.memory, params[2]->value.address, -1, -1);
          else if (strstr(p1, "del") == p1)
            del_var(con, params[1]->value.memory.memory, params[2]->value.address, -1, -1);
          return true;
        }
      return false;
    }
  else if (cmdline->syntax_match(uc, STRING CELL NUMBER NUMBER)) // DEL|ADD
    {
      char *p1= params[0]->value.string.string;
      if (p1 && *p1)
        {
          if (strcmp(p1, "add") == 0)
            add_var(con, params[1]->value.cell, params[2]->value.number, params[3]->value.number);
          else if (strstr(p1, "del") == p1)
            del_var(con, params[1]->value.cell, params[2]->value.number, params[3]->value.number);
          return true;
        }
      return false;
    }
  else if (cmdline->syntax_match(uc, STRING CELL NUMBER)) // DEL|ADD
    {
      char *p1= params[0]->value.string.string;
      if (p1 && *p1)
        {
          if (strcmp(p1, "add") == 0)
            add_var(con, params[1]->value.cell, params[2]->value.number, params[2]->value.number);
          else if (strstr(p1, "del") == p1)
            del_var(con, params[1]->value.cell, params[2]->value.number, params[2]->value.number);
          return true;
        }
      return false;
    }
  else if (cmdline->syntax_match(uc, STRING NUMBER STRING)) // TIMESCALE, PAUSETIME, STARTTIME
    {
      char *p1= params[0]->value.string.string;
      if (p1 && *p1 &&
          (strcmp(p1, "timescale") == 0 ||
           strcmp(p1, "pausetime") == 0 ||
           strcmp(p1, "starttime") == 0))
        {
          double time = params[1]->value.number;
          char *p2= params[2]->value.string.string;
          if (!strcmp(p2, "fs"))
            time /= 1e15;
          else if (!strcmp(p2, "ps"))
            time /= 1e12;
          else if (!strcmp(p2, "ns"))
            time /= 1e9;
          else if (!strcmp(p2, "us") || !strcmp(p2, "µs"))
            time /= 1e6;
          else if (!strcmp(p2, "ms"))
            time /= 1e3;
          else
            con->dd_printf("Units must be ms, us, ns, ps or fs\n");
          if (p1[0] == 't')
            timescale = time;
          else if (p1[0] == 'p')
            {
              pausetime = time;
              if (state == -1 && started && paused &&
                  (!filename || filename[0] == '|'))
                on = (uc->ticks->get_rtime() - event < pausetime);
            }
          else
            {
              starttime += time;
              event += time;
              //uc->sim->app->debug("vcd[%d]: event rescheduled to %.15f\n", cl_hw::id, event);
            }
          return true;
        }
      return false;
    }
  else if (cmdline->syntax_match(uc, STRING STRING)) // TIMESCALE AUTO, FILE, MOD
    {
      char *p1= params[0]->value.string.string;
      char *p2= params[1]->value.string.string;
      if (p1 && p2 &&
          !strcmp(p1, "timescale") &&
          !strcmp(p2, "auto"))
        {
          timescale = 0;
          return true;
        }
      if (started)
        {
          con->dd_printf("Already started\n");
          return true;
        }
      if (p1 && *p1)
        {
          if (strstr(p1, "mod") == p1)
            {
              if (p2 && *p2)
                modul= chars(p2);
              else
                con->dd_printf("Name missing\n");
              return true;
            }
          if ((strstr(p1, "out") == p1) ||
              (strstr(p1, "file") == p1) ||
              (strstr(p1, "in") == p1))
            {
              if (filename)
                free(filename);
              filename = NULL;
              state = (!strcmp(p1, "input") ? 0 : -1);
              if (!state && (!p2 || !*p2))
                con->dd_printf("Name missing\n");
              else if (p2 && *p2)
                filename = strdup(p2);
              return true;
            }
        }
      return false;
    }
  else if (cmdline->syntax_match(uc, STRING)) // [RE]START, PAUSE, STOP
    {
      char *p1= params[0]->value.string.string;
      if (p1 && *p1)
        {
          if ((strstr(p1, "out") == p1) ||
              (strstr(p1, "file") == p1) ||
              (strstr(p1, "view") == p1))
            {
              if (filename)
                free(filename);
              filename = NULL;
              state = -1;
              return true;
            }
          if (strstr(p1, "re") == p1 ||
              strcmp(p1, "start") == 0 ||
              (paused && strstr(p1, "paus") == p1))
            {
              if (!started)
                {
                  if ((fd= open_vcd(con)) == NULL)
                    return true;

                  if (state != -1)
                    {
                      if (!parse_header(con))
                        return true;
                    }
                  else
                    {
                      starttime += uc->ticks->get_rtime();
                      event += uc->ticks->get_rtime();

                      if (!timescale)
                        {
                          timescale = 1e3;
                          while (timescale * 1.0 / uc->get_xtal() < 1.0)
                            timescale *= 1000.0;
                          if (ddfmod(timescale * 1.0 / uc->get_xtal(), 1.0) > 0.0)
                            timescale *= 1000.0;
                        }

                      // generate vcd file header
                      time_t now = time(NULL);
		      if (application->quiet)
			now= (time_t)0;
		      if (application->quiet)
			fprintf(fd, "$date\n\tDate is omitted in quiet mode\n$end\n");
		      else
			fprintf(fd, "$date\n\t%s$end\n", ctime(&now));
		      fprintf(fd, "$version\n\tucsim\n$end\n$timescale ");
                      if (timescale >= 1e15)
                        fprintf(fd, "%.0ffs", timescale * 1e-15);
                      else if (timescale >= 1e12)
                        fprintf(fd, "%.0fps", timescale * 1e-12);
                      else if (timescale >= 1e9)
                        fprintf(fd, "%.0fns", timescale * 1e-9);
                      else if (timescale >= 1e6)
                        fprintf(fd, "%.0fus", timescale * 1e-6);
                      else if (timescale >= 1e3)
                        fprintf(fd, "%.0fms", timescale * 1e-3);
                      else
                        fprintf(fd, "%.0fms", timescale * 1e3);

                      fprintf(fd, " $end\n$scope module %s $end\n", modul.c_str());

                      for (class cl_vcd_var *var = vars; var; var = var->next_var)
                        {
                          fprintf(fd, "$var wire %d %c ",
                                      (var->bitnr_low >= 0 ? var->bitnr_high - var->bitnr_low + 1 : var->cell->get_width()),
                                      var->var_id);

                          t_addr addr = 0;
                          cl_address_space *as = uc->address_space(var->cell, &addr);
                          t_index i;

                          if (var->bitnr_high >= 0)
                            {
                              if (uc->vars->by_addr.search(as, addr, var->bitnr_high, var->bitnr_low, i))
                                {
                                  fprintf(fd, "%s $end\n", uc->vars->by_addr.at(i)->get_name());
                                  continue;
                                }
                            }

                          if (uc->vars->by_addr.search(as, addr, -1, -1, i) ||
                              uc->vars->by_addr.search(as, addr, var->cell->get_width() - 1, 0, i))
                            fprintf(fd, "%s", uc->vars->by_addr.at(i)->get_name());
                          else
                            {
                              fprintf(fd, "%s_", (as ? as->get_name() : "?"));
                              fprintf(fd, (as ? as->addr_format : "0x06x"), addr);
                            }

                          if (var->bitnr_high == var->bitnr_low)
                            fprintf(fd, ".%d", var->bitnr_low);
                          else if (var->bitnr_low != 0 || var->bitnr_high != var->cell->get_width() - 1)
                            fprintf(fd, "[%d:%d]", var->bitnr_high, var->bitnr_low);

                          fprintf(fd, " $end\n");
                        }

                      fputs("$upscope $end\n$enddefinitions $end\n", fd);
                      if (!filename || filename[0] == '|') fflush(fd);
                    }
                }

              if (!started || paused)
                {
                  if (paused)
                    con->dd_printf("Unpaused\n");
                  if (state == -1)
                    {
                      double now = uc->ticks->get_rtime();
                      double d =  now - event;
                      d = event + (pausetime >= 0 && d > pausetime ? pausetime : d);
                      starttime = now - (d - starttime);
                      fprintf(fd, "#%.0f\n", (now - starttime) * timescale);
                      if (started)
                        {
                          if (pausetime < 0)
                            fprintf(fd, "$comment Unpaused $end\n$dumpon\n");
                          else
                            fprintf(fd, "$comment Unpaused. Real duration was %.15f seconds $end\n$dumpon\n", now - event);
                        }
                      else
                        fprintf(fd, "$dumpvars\n");
                      event = now;
                      for (class cl_vcd_var *var = vars; var; var = var->next_var)
                        report(var, var->cell->get());
                      fprintf(fd, "$end\n");
                      if (!filename || filename[0] == '|') fflush(fd);
                    }
                  else
                    {
                      starttime = uc->ticks->get_rtime() + starttime;
                      event = starttime + event;
                    }
                }

              started = true;
              paused = false;
              if (state >= 0 || !filename || filename[0] == '|')
                on = true;

              return true;
            }
          if (strstr(p1, "paus") == p1)
            {
              if (started)
                {
                  paused = true;

                  if (state != -1 || pausetime < 0)
                    con->dd_printf("Paused\n");
                  else
                    con->dd_printf("Paused (pause limit %.15f s)\n", pausetime);

                  if (state == -1)
                    {
                      event = uc->ticks->get_rtime();
                      fprintf(fd, "#%.0f\n$comment Paused $end\n$dumpoff\n", (event - starttime) * timescale);
                      for (class cl_vcd_var *var = vars; var; var = var->next_var)
                        {
                          if (var->bitnr_low == var->bitnr_high)
                            fprintf(fd, "x%c\n", var->var_id);
                          else
                            {
                              putc('b', fd);
                              for (int i = var->bitnr_high; i >= var->bitnr_low; i--)
                                putc('x', fd);
                              fprintf(fd, " %c\n", var->var_id);
                            }
                        }
                      fprintf(fd, "$end\n");
                      if (!filename || filename[0] == '|') fflush(fd);
                      on = ((!filename || filename[0] == '|') || pausetime >= 0);
                    }
                  else
                    {
                      event = event - starttime;
                      starttime = starttime - uc->ticks->get_rtime();
                      on = false;
                    }
                }
              else
                con->dd_printf("Not started");
              return true;
            }
          if (strcmp(p1, "stop") == 0)
            {
              if (started)
                {
                  if (fd)
                    {
                      if (state == -1)
                        fprintf(fd, "#%.0f\n", (uc->ticks->get_rtime() - starttime) * timescale);
                      close_vcd();
                    }
                  fd= NULL;
                  on = false;
                  starttime = 0.0;
                  event = 0.0;
                  if (state != -1)
                    clear_vars();
                  else if (!vars)
                    reset_next_var_id();
                }
              started= paused= false;
              return true;
            }
          if (strcmp(p1, "break") == 0)
            {
              dobreak= !dobreak;
              con->dd_printf("Break on events %s\n", (dobreak ? "enabled" : "disabled"));
              return true;
            }
          if (strcmp(p1, "info") == 0)
            {
              print_info(con);
              return true;
            }
	  return false;
        }
    }

    return false;
}

void
cl_vcd::set_help(class cl_console_base *con)
{
  con->dd_printf("set hardware vcd[id] add memory address [ [bit_high] bit_low]\n");
  con->dd_printf("set hardware vcd[id] del[ete] memory address [ [bit_high] bit_low]\n");
  con->dd_printf("set hardware vcd[id] timescale n [ms|us|ns|ps|fs]\n");
  con->dd_printf("set hardware vcd[id] starttime n [ms|us|ns|ps|fs]\n");
  con->dd_printf("set hardware vcd[id] input \"vcd_file_name\"\n");
  con->dd_printf("set hardware vcd[id] output|file \"vcd_file_name\"\n");
  con->dd_printf("set hardware vcd[id] mod[ule] module_name\n");
  con->dd_printf("set hardware vcd[id] start\n");
  con->dd_printf("set hardware vcd[id] pause\n");
  con->dd_printf("set hardware vcd[id] [re]start\n");
  con->dd_printf("set hardware vcd[id] stop\n");
  con->dd_printf("set hardware vcd[id] break\n");
  con->dd_printf("set hardware vcd[id] new id\n");
}

void
cl_vcd::report(class cl_vcd_var *var, t_mem v)
{
  double now = uc->ticks->get_rtime();
  //uc->sim->app->debug("vcd[%d]: '%c' changed at %.15f\n",
  //                    cl_hw::id, var->var_id, uc->ticks->get_rtime());
  if (event != now)
    {
      event = now;
      fprintf(fd, "#%.0f\n", (event - starttime) * timescale);
    }

  if (var->bitnr_low == var->bitnr_high)
    {
      fprintf(fd, "%c%c\n", ((v & var->bitmask) ? '1' : '0'), var->var_id);
    }
  else
    {
      putc('b', fd);
      for (int i = var->bitnr_high; i >= var->bitnr_low; i--)
        putc((v & (1U << i)) ? '1' : '0', fd);
      fprintf(fd, " %c\n", var->var_id);
    }

  fflush(fd);
}

int
cl_vcd::tick(int cycles)
{
  if (!fd || !started)
    return 0;

  double now = uc->ticks->get_rtime();

  if (state == -1)
    {
      if (pausetime >= 0 && now - event >= pausetime)
        on = false;
      else if (!filename || filename[0] == '|')
        {
          fprintf(fd, "#%.0f\n", (now - starttime) * timescale);
          fflush(fd);
        }
      return 0;
    }

  bool event_occurred = false;
  bool more_data = true;

  while (more_data && event < now && ULPs(now, event) > 100000)
    {
      t_mem value = 0;
      while ((more_data = read_word(0U)))
        {
          if (state == 0 && word[0] != '1' && word[0] != '0')
            {
              if (word[0] == '#')
                {
                  event = starttime + timescale * strtod(&word[1], NULL);
                  //uc->sim->app->debug("vcd[%d]: next event at %.15f\n", cl_hw::id, event);
                  if (event < now && ULPs(now, event) > 100000)
                    continue;
                  break;
                }
              else if (word[0] == 'b' || word[0] == 'B')
                {
                  // Bit-vector value, next word is ID
                  for (char *ptr = word; *ptr; ptr++)
                    value = (value << 1) | (*ptr == '1' ? 1 : 0);
                  state = 1;
                }
              else if (word[0] == 'r' || word[0] == 'R')
                {
                  // Real value, next word is ID
                  // Unsupported
                  state = 1;
                }
            }
          else if (state == 1 || word[0] == '1' || word[0] == '0')
            {
              // ID for preceding value or bit followed by ID
              char id;
              if (word[0] == '1')
                {
                  value = 1;
                  id = word[1];
                }
              else if (word[0] == '0')
                {
                  value = 0;
                  id = word[1];
                }
              else
                id = word[0];

              for (class cl_vcd_var *var = vars; var; var = var->next_var)
                {
                  if (var->var_id == id)
                    {
                      unsigned long ticks = uc->ticks->get_ticks();
                      uc->ticks->set(ticks, event);
                      //uc->sim->app->debug("vcd[%d]: set '%c' to 0x%u at %.15f now %.15f isless %s ULPs %u\n",
                      //                    cl_hw::id, id, (value << var->bitnr_low) & var->bitmask, uc->ticks->get_rtime(), now, ULPs(now, event));
                      var->cell->write((var->cell->get() & (~var->bitmask)) | ((value << var->bitnr_low) & var->bitmask));
                      uc->ticks->set(ticks, now);
                      event_occurred = true;
                      break;
                    }
                }

              state = 0;
            }
          else if (state == 2)
            {
              // ID for preceding value
              // Unsupported
              state = 0;
            }
        }
    }

  if (event_occurred && dobreak)
    uc->vcd_break = true;

  return 0;
}

void
cl_vcd::print_info(class cl_console_base *con)
{
  con->dd_printf("%s[%d] value change dump\n", id_string, id);
  con->dd_printf("  Modul:      %s\n", modul.c_str());
  con->dd_printf("  Started:    %s\n", (started ? "YES" : "no"));
  con->dd_printf("  Paused:     %s\n", (paused ? "YES" : "no"));
  if (!application->quiet)
    con->dd_printf("  File:       %s\n", (filename ? filename : "(none)"));

  if (state != -1)
    {
      con->dd_printf("  Mode:       input\n");
      con->dd_printf("  Break:      %s\n", (dobreak ? "enabled" : "disabled"));

      if (timescale >= 1e-3)
        con->dd_printf("  Time scale: %.0f ms\n", timescale * 1e3);
      else if (timescale >= 1e-6)
        con->dd_printf("  Time scale: %.0f us\n", timescale * 1e6);
      else if (timescale >= 1e-9)
        con->dd_printf("  Time scale: %.0f ns\n", timescale * 1e9);
      else if (timescale >= 1e-12)
        con->dd_printf("  Time scale: %.0f ps\n", timescale * 1e12);
      else
        con->dd_printf("  Time scale: %.0f fs\n", timescale * 1e15);
    }
  else
    {
      con->dd_printf("  Mode:       output\n");

      if (!timescale)
        con->dd_printf("  Time scale: auto set on start\n");
      else
        {
          if (timescale >= 1e15)
            con->dd_printf("  Time scale: %.0f fs\n", timescale * 1e-15);
          else if (timescale >= 1e12)
            con->dd_printf("  Time scale: %.0f ps\n", timescale * 1e-12);
          else if (timescale >= 1e9)
            con->dd_printf("  Time scale: %.0f ns\n", timescale * 1e-9);
          else if (timescale >= 1e6)
            con->dd_printf("  Time scale: %.0f us\n", timescale * 1e-6);
          else
            con->dd_printf("  Time scale: %.0f ms\n", timescale * 1e-3);
        }
    }

  con->dd_printf("  Start time: %.15f s\n", starttime);
  con->dd_printf("  %s event: %.15f s\n", (state != -1 ? "Next" : "Last"), event);
  con->dd_printf("  Pause time: ");
  if (pausetime >= 0)
    con->dd_printf("%.15f s", pausetime);
  con->dd_printf("\n  Simul time: %.15f s\n", uc->ticks->get_rtime());

  con->dd_printf("  Variables:\n");
  con->dd_printf("    Address           Symbol\n");
  for (class cl_vcd_var *var = vars; var; var = var->next_var)
    {
      t_addr a= 0;
      cl_address_space *as = uc->address_space(var->cell, &a);

      con->dd_printf("    %s[", (as ? as->get_name() : "?"));
      con->dd_printf((as ? as->addr_format : "0x06x"), a);
      con->dd_printf("]");
      if (var->bitnr_high < 0)
        con->dd_printf("      ");
      else if (var->bitnr_high == var->bitnr_low)
        con->dd_printf(".%d    ", var->bitnr_high);
      else if (var->bitnr_low != 0 || var->bitnr_high != var->cell->get_width() - 1)
        con->dd_printf("[%d:%d] ", var->bitnr_high, var->bitnr_low);
      else
        con->dd_printf("      ");

      int i;

      if (uc->vars->by_addr.search(as, a, var->bitnr_high, var->bitnr_low, i))
        con->dd_printf("%s\n", uc->vars->by_addr.at(i)->get_name());
      else
        {
          if (uc->vars->by_addr.search(as, a, -1, -1, i) ||
              uc->vars->by_addr.search(as, a, var->cell->get_width() - 1, 0, i))
            {
              const char *cname = uc->vars->by_addr.at(i)->get_name();
              con->dd_printf("%s", cname);
              if (var->bitnr_high >= 0)
                {
                  if (var->bitnr_high == var->bitnr_low)
                    con->dd_printf(".%d", var->bitnr_high);
                  else if (var->bitnr_low != 0 || var->bitnr_high != var->cell->get_width() - 1)
                    con->dd_printf("[%d:%d]", var->bitnr_high, var->bitnr_low);
                }
            }
          con->dd_printf("\n");
        }
    }

  //print_cfg_info(con);
}


/* End of sim.src/vcd.cc */
