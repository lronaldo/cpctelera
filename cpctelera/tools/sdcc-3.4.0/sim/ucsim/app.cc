/*
 * Simulator of microcontrollers (app.cc)
 *
 * Copyright (C) 2001,01 Drotos Daniel, Talker Bt.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif
#ifdef SOCKET_AVAIL
# include HEADER_SOCKET
#endif
#include <ctype.h>
#include <errno.h>
#include "i_string.h"

// prj
#include "utils.h"
#include "appcl.h"
#include "optioncl.h"
#include "globals.h"

// sim.src
#include "simcl.h"

// cmd.src
#include "cmdsetcl.h"
#include "cmdutil.h"
#include "cmdconfcl.h"
#include "showcl.h"
#include "getcl.h"
#include "setcl.h"
#ifdef _WIN32
#include "newcmdwin32cl.h"
#else
#include "newcmdposixcl.h"
#endif


/*
 * Program options
 */

/*cl_option::cl_option(int atype, char sn, char *ln)
{
  type= atype;
  short_name= sn;
  if (!ln)
    long_name= NULL;
  else
    long_name= strdup(ln);
  values= new cl_ustrings(1, 1);
}

cl_option::~cl_option(void)
{
  if (long_name)
    free(long_name);
  delete values;
}

int
cl_option::add_value(char *value)
{
  values->add(value);
  return(values->count - 1);
}

char *
cl_option::get_value(int index)
{
  if (index > values->count - 1)
    return(0);
  return((char*)(values->at(index)));
}*/

/* List of options */

/*cl_options::cl_options(void):
  cl_list(2, 2)
{
}*/


/*
 * Application
 ****************************************************************************
 */

cl_app::cl_app(void)
{
  sim= 0;
  in_files= new cl_ustrings(2, 2, "input files");
  options= new cl_options();
  going= 1;
}

cl_app::~cl_app(void)
{
  remove_simulator();
  delete commander;
  delete in_files;
  delete options;
}

int
cl_app::init(int argc, char *argv[])
{
  cl_base::init();
  set_name("application");
  mk_options();
  proc_arguments(argc, argv);
  class cl_cmdset *cmdset= new cl_cmdset();
  cmdset->init();
  build_cmdset(cmdset);
  commander= new cl_commander(this, cmdset/*, sim*/);
  commander->init();
  return(0);
}

/* Main cycle */

int
cl_app::run(void)
{
  int done= 0;
  unsigned  input_check_skip = 0;
  
  while (!done &&
         going)
    {
      if (sim)
        {
          if (sim->state & SIM_GO)
            {
              if ((!input_check_skip) && (commander->input_avail()))
                {
                  done= commander->proc_input();
                  
                  // run a few steps before checking for more input
                  ++input_check_skip;
                }
              else
                {
                  sim->step();
                  
                  input_check_skip = (input_check_skip + 1) % 50;
                }
            }
          else
            {
              commander->wait_input();
              done= commander->proc_input();
            }
        }
      else
        {
          commander->wait_input();
          done= commander->proc_input();
        }
    }
  return(0);
}

void
cl_app::done(void)
{
}


/*
 * Interpretation of parameters
 */

static void
print_help(const char *name)
{
  printf("%s: %s\n", name, VERSIONSTR);
  printf("Usage: %s [-hHVvP] [-p prompt] [-t CPU] [-X freq[k|M]]\n"
         "       [-c file] [-s file] [-S optionlist]"
#ifdef SOCKET_AVAIL
         " [-Z portnum] [-k portnum]"
#endif
         "\n"
         "       [files...]\n", name);
  printf
    (
     "Options:\n"
     "  -t CPU       Type of CPU: 51, C52, 251, etc.\n"
     "  -X freq[k|M] XTAL frequency\n"
     "  -c file      Open command console on `file'\n"
#ifdef SOCKET_AVAIL
     "  -Z portnum   Use localhost:portnumber for command console\n"
     "  -k portnum   Use localhost:portnum for serial I/O\n"
#endif
     "  -s file      Connect serial interface to `file'\n"
     "  -S options   `options' is a comma separated list of options according to\n"
     "               serial interface. Know options are:\n"
     "                  in=file   serial input will be read from file named `file'\n"
     "                  out=file  serial output will be written to `file'\n"
     "  -p prompt    Specify string for prompt\n"
     "  -P           Prompt is a null ('\\0') character\n"
     "  -V           Verbose mode\n"
     "  -v           Print out version number\n"
     "  -H           Print out types of known CPUs\n"
     "  -h           Print out this help\n"
     );
}

enum {
  SOPT_IN= 0,
  SOPT_OUT
};

static const char *S_opts[]= {
  /*[SOPT_IN]=*/ "in",
  /*[SOPT_OUT]=*/ "out",
  NULL
};

int
cl_app::proc_arguments(int argc, char *argv[])
{
  int i, c;
  char opts[100], *cp, *subopts, *value;
  char *cpu_type= NULL;
  bool s_done= DD_FALSE, k_done= DD_FALSE;
  bool S_i_done= DD_FALSE, S_o_done= DD_FALSE;

  strcpy(opts, "c:C:p:PX:vVt:s:S:hHk:");
#ifdef SOCKET_AVAIL
  strcat(opts, "Z:r:");
#endif

  while((c= getopt(argc, argv, opts)) != -1)
    switch (c)
      {
      case 'c':
        if (!options->set_value("console_on", this, optarg))
          fprintf(stderr, "Warning: No \"console_on\" option found "
                  "to set by -c\n");
        break;
      case 'C':
        if (!options->set_value("config_file", this, optarg))
          fprintf(stderr, "Warning: No \"config_file\" option found to set "
                  "parameter of -C as config file\n");
        break;
#ifdef SOCKET_AVAIL
      case 'Z': case 'r':
        {
          // By Sandeep
          // Modified by DD
          class cl_option *o;
          options->new_option(o= new cl_number_option(this, "port_number",
                                                      "Listen on port (-Z)"));
          o->init();
          o->hide();
          if (!options->set_value("port_number", this, strtol(optarg, NULL, 0)))
            fprintf(stderr, "Warning: No \"port_number\" option found"
                    " to set parameter of -Z as pot number to listen on\n");
          break;
        }
#endif
      case 'p': {
        if (!options->set_value("prompt", this, optarg))
          fprintf(stderr, "Warning: No \"prompt\" option found to set "
                  "parameter of -p as default prompt\n");
        break;
      }
      case 'P':
        if (!options->set_value("null_prompt", this, bool(DD_TRUE)))
          fprintf(stderr, "Warning: No \"null_prompt\" option found\n");
        break;
      case 'X':
        {
          double XTAL;
          for (cp= optarg; *cp; *cp= toupper(*cp), cp++);
          XTAL= strtod(optarg, &cp);
          if (*cp == 'K')
            XTAL*= 1e3;
          if (*cp == 'M')
            XTAL*= 1e6;
          if (XTAL == 0)
            {
              fprintf(stderr, "Xtal frequency must be greather than 0\n");
              exit(1);
            }
          if (!options->set_value("xtal", this, XTAL))
            fprintf(stderr, "Warning: No \"xtal\" option found to set "
                    "parameter of -X as XTAL frequency\n");
          break;
        }
      case 'v':
        printf("%s: %s\n", argv[0], VERSIONSTR);
        exit(0);
        break;
      case 'V':
        if (!options->set_value("debug", this, (bool)DD_TRUE))
          fprintf(stderr, "Warning: No \"debug\" option found to set "
                  "by -V parameter\n");
        break;
      case 't':
        {
          if (cpu_type)
            free(cpu_type);
          cpu_type= case_string(case_upper, optarg);
          if (!options->set_value("cpu_type", this, /*optarg*/cpu_type))
            fprintf(stderr, "Warning: No \"cpu_type\" option found to set "
                    "parameter of -t as type of controller\n");
          break;
        }
      case 's':
      {
#ifdef _WIN32
        /* TODO: this code should be probably used for all platforms? */
        FILE *Ser;
        if (s_done)
          {
            fprintf(stderr, "-s option can not be used more than once.\n");
            break;
          }
        s_done= DD_TRUE;
        if ((Ser= fopen(optarg, "r+")) == NULL)
          {
            fprintf(stderr,
                    "Can't open `%s': %s\n", optarg, strerror(errno));
            return(4);
          }
        if (!options->set_value("serial_in_file", this, Ser))
          fprintf(stderr, "Warning: No \"serial_in_file\" option found to set "
                  "parameter of -s as serial input file\n");
        if (!options->set_value("serial_out_file", this, Ser))
          fprintf(stderr, "Warning: No \"serial_out_file\" option found "
                  "to set parameter of -s as serial output file\n");
#else
        FILE *Ser_in, *Ser_out;
        if (s_done)
          {
            fprintf(stderr, "-s option can not be used more than once.\n");
            break;
          }
        s_done= DD_TRUE;
        if ((Ser_in= fopen(optarg, "r")) == NULL)
          {
            fprintf(stderr,
                    "Can't open `%s': %s\n", optarg, strerror(errno));
            return(4);
          }
        if (!options->set_value("serial_in_file", this, Ser_in))
          fprintf(stderr, "Warning: No \"serial_in_file\" option found to set "
                  "parameter of -s as serial input file\n");
        if ((Ser_out= fopen(optarg, "w")) == NULL)
          {
            fprintf(stderr,
                    "Can't open `%s': %s\n", optarg, strerror(errno));
            return(4);
          }
        if (!options->set_value("serial_out_file", this, Ser_out))
          fprintf(stderr, "Warning: No \"serial_out_file\" option found "
                  "to set parameter of -s as serial output file\n");
#endif
        break;
      }
#ifdef SOCKET_AVAIL
      // socket serial I/O by Alexandre Frey <Alexandre.Frey@trusted-logic.fr>
      case 'k':
        {
          FILE *Ser_in, *Ser_out;
          UCSOCKET_T sock;
          unsigned short serverport;
          UCSOCKET_T client_sock;

          if (k_done)
            {
              fprintf(stderr, "Serial input specified more than once.\n");
            }
          k_done= DD_TRUE;

          serverport = atoi(optarg);
          sock = make_server_socket(serverport);
#ifdef _WIN32
          if (SOCKET_ERROR == listen((SOCKET)sock, 1))
            {
              fprintf(stderr, "Listen on port %d: %d\n", serverport,
                WSAGetLastError());
              return (4);
            }
          fprintf(stderr, "Listening on port %d for a serial connection.\n",
            serverport);
          if (INVALID_SOCKET == (client_sock = accept(sock, NULL, NULL)))
            {
              fprintf(stderr, "accept: %d\n", WSAGetLastError());
              return (4);
            }
          fprintf(stderr, "Serial connection established.\n");

          int fh = _open_osfhandle((intptr_t)client_sock, 0);
          if (-1 == fh)
            {
              perror("_open_osfhandle");
              return (4);
            }
          if (NULL == (Ser_in = fdopen(fh, "r")))
            {
              fprintf(stderr, "Can't create input stream: %s\n", strerror(errno));
              return (4);
            }

          fh = _open_osfhandle((intptr_t)client_sock, 0);
          if (-1 == fh)
            {
              perror("_open_osfhandle");
            }
          if (NULL == (Ser_out = fdopen(fh, "w"))) {
            fprintf(stderr, "Can't create output stream: %s\n", strerror(errno));
            return (4);
          }
#else
          if (listen(sock, 1) < 0) {
            fprintf(stderr, "Listen on port %d: %s\n", serverport,
                    strerror(errno));
            return (4);
          }
          fprintf(stderr, "Listening on port %d for a serial connection.\n",
                  serverport);
          if ((client_sock= accept(sock, NULL, NULL)) < 0) {
            fprintf(stderr, "accept: %s\n", strerror(errno));
          }
          fprintf(stderr, "Serial connection established.\n");

          if ((Ser_in= fdopen(client_sock, "r")) == NULL) {
            fprintf(stderr, "Can't create input stream: %s\n", strerror(errno));
            return (4);
          }
          if ((Ser_out= fdopen(client_sock, "w")) == NULL) {
            fprintf(stderr, "Can't create output stream: %s\n", strerror(errno));
            return (4);
          }
#endif
          if (!options->set_value("serial_in_file", this, (void*)Ser_in))
            fprintf(stderr, "Warning: No \"serial_in_file\" option found to "
                    "set parameter of -s as serial input file\n");
          if (!options->set_value("serial_out_file", this, Ser_out))
            fprintf(stderr, "Warning: No \"serial_out_file\" option found "
                    "to set parameter of -s as serial output file\n");
          break;
        }
#endif
      case 'S':
        subopts= optarg;
        while (*subopts != '\0')
          switch (get_sub_opt(&subopts, S_opts, &value))
            {
              FILE *Ser_in, *Ser_out;
            case SOPT_IN:
              if (value == NULL) {
                fprintf(stderr, "No value for -S in\n");
                exit(1);
              }
              if (S_i_done)
                {
                  fprintf(stderr, "Serial input specified more than once.\n");
                  break;
                }
              S_i_done= DD_TRUE;
              if ((Ser_in= fopen(value, "r")) == NULL)
                {
                  fprintf(stderr,
                          "Can't open `%s': %s\n", value, strerror(errno));
                  exit(4);
                }
              if (!options->set_value("serial_in_file", this, (void*)Ser_in))
                fprintf(stderr, "Warning: No \"serial_in_file\" option found "
                        "to set parameter of -s as serial input file\n");
              break;
            case SOPT_OUT:
              if (value == NULL) {
                fprintf(stderr, "No value for -S out\n");
                exit(1);
              }
              if (S_o_done)
                {
                  fprintf(stderr, "Serial output specified more than once.\n");
                  break;
                }
              if ((Ser_out= fopen(value, "w")) == NULL)
                {
                  fprintf(stderr,
                          "Can't open `%s': %s\n", value, strerror(errno));
                  exit(4);
                }
              if (!options->set_value("serial_out_file", this, Ser_out))
                fprintf(stderr, "Warning: No \"serial_out_file\" option found "
                        "to set parameter of -s as serial output file\n");
              break;
            default:
              /* Unknown suboption. */
              fprintf(stderr, "Unknown suboption `%s' for -S\n", value);
              exit(1);
              break;
            }
        break;
      case 'h':
        print_help("s51");
        exit(0);
        break;
      case 'H':
        {
          if (!cpus)
            {
              fprintf(stderr, "CPU type is not selectable\n");
              exit(0);
            }
          i= 0;
          while (cpus[i].type_str != NULL)
            {
              printf("%s\n", cpus[i].type_str);
              i++;
            }
          exit(0);
          break;
        }
      case '?':
        if (isprint(optopt))
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
        return(1);
        break;
      default:
        exit(c);
      }

  for (i= optind; i < argc; i++)
    in_files->add(argv[i]);

  return(0);
}


class cl_uc *
cl_app::get_uc(void)
{
  if (!sim)
    return(0);
  return(sim->get_uc());
}


/* Command handling */

class cl_cmd *
cl_app::get_cmd(class cl_cmdline *cmdline)
{
  return(0);
}


/*
 * Messages to broadcast
 */

/*
void
cl_app::mem_cell_changed(class cl_m *mem, t_addr addr)
{
  if (sim)
    sim->mem_cell_changed(mem, addr);
}
*/


/* Adding and removing components */

void
cl_app::set_simulator(class cl_sim *simulator)
{
  if (sim)
    remove_simulator();
  sim= simulator;

}

void
cl_app::remove_simulator(void)
{
  if (!sim)
    return;
  delete sim;
  sim= 0;
}

void
cl_app::build_cmdset(class cl_cmdset *cmdset)
{
  class cl_cmd *cmd;
  class cl_super_cmd *super_cmd;
  class cl_cmdset *cset;

  {
    cset= new cl_cmdset();
    cset->init();
    cset->add(cmd= new cl_conf_cmd("_no_parameters_", 0,
"conf               Configuration",
"long help of conf"));
    cmd->init();
    cset->add(cmd= new cl_conf_objects_cmd("objects", 0,
"conf objects       Show object tree",
"long help of conf objects"));
    cmd->init();
  }
  cmdset->add(cmd= new cl_super_cmd("conf", 0,
"conf subcommand    Information, see `conf' command for more help",
"long help of conf", cset));
  cmd->init();

  cmd= new cl_help_cmd("help", 0,
"help [command]     Help about command(s)",
"long help of help");
  cmdset->add(cmd);
  cmd->init();
  cmd->add_name("?");

  cmdset->add(cmd= new cl_quit_cmd("quit", 0,
"quit               Quit",
"long help of quit"));
  cmd->init();

  cmdset->add(cmd= new cl_kill_cmd("kill", 0,
"kill               Shutdown simulator",
"long help of kill"));
  cmd->init();

  cmdset->add(cmd= new cl_exec_cmd("exec", 0,
"exec file          Execute commands from file",
"long help of exec"));
  cmd->init();

  cmdset->add(cmd= new cl_expression_cmd("expression", 0,
"expression expr    Evaluate the expression",
"long help of expression "));
  cmd->init();
  cmd->add_name("let");

  {
    super_cmd= (class cl_super_cmd *)(cmdset->get_cmd("show"));
    if (super_cmd)
      cset= super_cmd->commands;
    else {
      cset= new cl_cmdset();
      cset->init();
    }
    cset->add(cmd= new cl_show_copying_cmd("copying", 0,
"show copying       Conditions for redistributing copies of uCsim",
"long help of show copying"));
    cmd->init();
    cset->add(cmd= new cl_show_warranty_cmd("warranty", 0,
"show warranty      Various kinds of warranty you do not have",
"long help of show warranty"));
    cmd->init();
    cset->add(cmd= new cl_show_option_cmd("option", 0,
"show option [name] Show internal data of options",
"long help of show option"));
    cmd->init();
    cset->add(cmd= new cl_show_error_cmd("error", 0,
"show error         Show class of errors",
"long help of show error"));
    cmd->init();
  }
  if (!super_cmd)
    {
      cmdset->add(cmd= new cl_super_cmd("show", 0,
"show subcommand    Generic command for showing things about the uCsim",
"long help of show", cset));
      cmd->init();
    }

  {
    super_cmd= (class cl_super_cmd *)(cmdset->get_cmd("get"));
    if (super_cmd)
      cset= super_cmd->commands;
    else {
      cset= new cl_cmdset();
      cset->init();
    }
    cset->add(cmd= new cl_get_option_cmd("option", 0,
"get option [name]  Get value of an option",
"long help of get option"));
    cmd->init();
  }
  if (!super_cmd)
    {
      cmdset->add(cmd= new cl_super_cmd("get", 0,
"get subcommand     Get, see `get' command for more help",
"long help of get", cset));
      cmd->init();
    }

  {
    super_cmd= (class cl_super_cmd *)(cmdset->get_cmd("set"));
    if (super_cmd)
      cset= super_cmd->commands;
    else {
      cset= new cl_cmdset();
      cset->init();
    }
    cset->add(cmd= new cl_set_option_cmd("option", 0,
"set option name|nr value\n"
"                   Set value of an option",
"long help of set option"));
    cmd->init();
    cset->add(cmd= new cl_set_error_cmd("error", 0,
"set error error_name on|off|unset\n"
"                   Set value of an error",
"long help of set error"));
    cmd->init();
  }
  if (!super_cmd)
    {
      cmdset->add(cmd= new cl_super_cmd("set", 0,
"set subcommand     Set, see `set' command for more help",
"long help of set", cset));
      cmd->init();
    }
}

void
cl_app::mk_options(void)
{
  class cl_option *o;

  options->new_option(o= new cl_bool_option(this, "null_prompt",
                                            "Use \\0 as prompt (-P)"));
  o->init();

  options->new_option(o= new cl_pointer_option(this, "serial_in_file",
                                               "Input file for serial line (-s)"));
  o->init();
  o->hide();

  options->new_option(o= new cl_pointer_option(this, "serial_out_file",
                                               "Output file for serial line (-s)"));
  o->init();
  o->hide();

  options->new_option(o= new cl_string_option(this, "prompt",
                                              "String of prompt (-p)"));
  o->init();

  options->new_option(o= new cl_bool_option(this, "debug",
                                            "Print debug messages (-V)"));
  o->init();

  options->new_option(o= new cl_string_option(this, "console_on",
                                              "Open console on this file (-c)"));
  o->init();
  o->hide();

  options->new_option(o= new cl_string_option(this, "config_file",
                                              "Execute this file at startup (-C)"));
  o->init();
  o->hide();

  options->new_option(o= new cl_float_option(this, "xtal",
                                             "Frequency of XTAL in Hz"));
  o->init();
  o->set_value(11059200.0);

  options->new_option(o= new cl_string_option(this, "cpu_type",
                                              "Type of controller (-t)"));
  o->init();
  o->hide();
}


int
cl_app::dd_printf(const char *format, ...)
{
  va_list ap;

  if (!commander)
    return(0);

  va_start(ap, format);
  int i= commander->dd_printf(format, ap);
  va_end(ap);
  return(i);
}

int
cl_app::debug(const char *format, ...)
{
  va_list ap;

  if (!commander)
    return(0);

  va_start(ap, format);
  int i= commander->debug(format, ap);
  va_end(ap);
  return(i);
}


/* End of app.cc */
