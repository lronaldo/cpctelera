/*
 * Simulator of microcontrollers (app.cc)
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

#include "ddconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef HAVE_GETOPT_H
# include <getopt.h>
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
#include "cmd_execcl.h"
#include "cmdutil.h"
#include "cmd_confcl.h"
#include "cmd_showcl.h"
#include "cmd_getcl.h"
#include "cmd_setcl.h"
#include "newcmdposixcl.h"
#include "cmdlexcl.h"

bool jaj= false;


/*
 * Application
 ****************************************************************************
 */

cl_app::cl_app(void)
{
  sim= 0;
  in_files= new cl_ustrings(2, 2, "input files");
  options= new cl_options();
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
  set_name(cchars("application"));
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

enum run_states {
  rs_config,
  rs_read_files,
  rs_start,
  rs_run
};

int
cl_app::run(void)
{
  int done= 0;
  double input_last_checked= 0;
  class cl_option *o= options->get_option("go");
  bool g_opt= false;
  unsigned int cyc= 0;
  enum run_states rs= rs_config;
    
  while (!done)
    {
      if ((rs == rs_config) &&
	  (commander->config_console == NULL))
	{
	  rs= rs_read_files;
	}
      if (rs == rs_read_files)
	{
	  if (sim && (sim->uc != NULL))
	    {
	      int i;
	      for (i= 0; i < in_files->count; i++)
		{
		  char *fname= (char *)(in_files->at(i));
		  long l;
		  if ((l= sim->uc->read_file(fname, NULL)) >= 0)
		    {
		      ///*commander->all_printf*/printf("%ld words read from %s\n", l, fname);
		    }
		}
	    }
	  rs= rs_start;
	}
      if (rs == rs_start)
	{
	  if (o)
	    o->get_value(&g_opt);
	  if (sim && g_opt)
	    sim->start(0, 0);
	  rs= rs_run;
	}
      ++cyc;
      if (!sim)
	{
	  commander->wait_input();
	  done= commander->proc_input();
	}
      else
        {
          if (sim->state & SIM_GO)
            {
	      if (cyc - input_last_checked > 10000)
		{
		  input_last_checked= cyc;
		  if (sim->uc)
		    sim->uc->touch();
		  if (commander->input_avail())
		    done= commander->proc_input();
		}
	      sim->step();
	      if (jaj && commander->frozen_console)
		{
		  sim->uc->print_regs(commander->frozen_console),
		    commander->frozen_console->dd_printf("\n");
		}
            }
	  else
	    {
	      if (commander->input_avail())
		done= commander->proc_input();
	      //commander->wait_input();
	      //done= commander->proc_input();
	      if (sim->uc)
		sim->uc->touch();
	      loop_delay();
	    }
	  if (sim->state & SIM_QUIT)
	    done= 1;
	}
      commander->check();
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
print_help(char *name)
{
  printf("%s: %s\n", name, VERSIONSTR);
  printf("Usage: %s [-hHVvPg] [-p prompt] [-t CPU] [-X freq[k|M]]\n"
	 "       [-C cfg_file] [-c file] [-s file] [-S optionlist]\n"
	 "       [-a nr]"
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
     "  -C cfg_file  Read initial commands from `cfg_file' and execute them\n"
     "  -c file      Open command console on `file' (use `-' for std in/out)\n"
     "  -Z portnum   Use localhost:portnum for command console\n"
     "  -k portnum   Use localhost:portnum for serial I/O\n"
     "  -s file      Connect serial interface uart0 to `file'\n"
     "  -S options   `options' is a comma separated list of options according to\n"
     "               serial interface. Know options are:\n"
     "                  uart=nr   number of uart (default=0)\n"
     "                  in=file   serial input will be read from file named `file'\n"
     "                  out=file  serial output will be written to `file'\n"
     "                  port=nr   Use localhost:nr as server for serial line\n"
     "  -I options   `options' is a comma separated list of options according to\n"
     "               simulator interface. Known options are:\n"
     "                 if=memory[address]  turn on interface on given memory location\n"
     "                 in=file             specify input file for IO\n"
     "                 out=file            specify output file forr IO\n"
     "  -p prompt    Specify string for prompt\n"
     "  -P           Prompt is a null ('\\0') character\n"
     "  -g           Go, start simulation\n"
     "  -G           Go, start simulation, quit on stop\n"
     "  -a nr        Specify size of variable space (default=256)\n"
     "  -w           Writable flash"
     "  -V           Verbose mode\n"
     "  -v           Print out version number and quit\n"
     "  -H           Print out types of known CPUs and quit\n"
     "  -h           Print out this help and quit\n"
     );
}

enum {
  SOPT_IN= 0,
  SOPT_OUT,
  SOPT_UART,
  SOPT_USART,
  SOPT_PORT
};

static const char *S_opts[]= {
  /*[SOPT_IN]=*/	"in",
  /*[SOPT_OUT]=*/	"out",
  /*[SOPT_UART]=*/	"uart",
  /*[SOPT_USART]=*/	"usart",
  /*[SOPT_PORT]=*/	"port",
  NULL
};

enum {
  IOPT_IF= 0,
  IOPT_IN,
  IOPT_OUT
};

static const char *I_opts[]= {
  /*IOPT_IF*/		"if",
  /*IOPT_IN*/		"in",
  /*IOPT_OUT*/		"out",
  NULL
};

int
cl_app::proc_arguments(int argc, char *argv[])
{
  int i, c;
  char opts[100], *cp, *subopts, *value;
  char *cpu_type= NULL;
  bool /*s_done= DD_FALSE,*/ k_done= false;
  //bool S_i_done= false, S_o_done= false;

  strcpy(opts, "c:C:p:PX:vVt:s:S:I:a:whHgGJ_");
#ifdef SOCKET_AVAIL
  strcat(opts, "Z:r:k:");
#endif

  for (i= 0; i < argc; i++)
    {
      if ((strcmp(argv[i], "-fullname") == 0) ||
	  (strcmp(argv[i], "-quiet") == 0) ||
	  (strcmp(argv[i], "-args") == 0) ||
	  (strcmp(argv[i], "-nx") == 0))
	strcpy(argv[i], "-_");
    }
  
  while((c= getopt(argc, argv, opts)) != -1)
    switch (c)
      {
      case '_': break;
      case 'J': jaj= true; break;
      case 'g':
	if (!options->set_value("go", this, true))
	  fprintf(stderr, "Warning: No \"go\" option found "
		  "to set by -g\n");
	break;
      case 'G':
	if (!options->set_value("go", this, true))
	  fprintf(stderr, "Warning: No \"go\" option found "
		  "to set by -G\n");
	if (!options->set_value("quit", this, true))
	  fprintf(stderr, "Warning: No \"quit\" option found "
		  "to set by -G\n");
	break;
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
	  options->new_option(o= new cl_number_option(this, cchars("port_number"),
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
	if (!options->set_value("null_prompt", this, bool(true)))
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
      case 'a':
	if (!options->set_value("var_size", this, strtol(optarg, 0, 0)))
	  fprintf(stderr, "Warning: No \"var_size\" option found to set "
		  "by parameter of -a as variable space size\n");
	break;
      case 'w': {
	if (!options->set_value("writable_flash", this, bool(true)))
	  fprintf(stderr, "Warning: No \"writable_flash\" option found to set\n");	       
	break;
      }
      case 'v':
	printf("%s: %s\n", argv[0], VERSIONSTR);
        exit(0);
        break;
      case 'V':
	if (!options->set_value("debug", this, (bool)true))
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
	if (!options->set_value("serial0_in_file", this, /*(void*)Ser_in*/optarg))
	  fprintf(stderr, "Warning: No \"serial0_in_file\" option found to set "
		  "parameter of -s as serial input file\n");
	if (!options->set_value("serial0_out_file", this, /*Ser_out*/optarg))
	  fprintf(stderr, "Warning: No \"serial_out0_file\" option found "
		  "to set parameter of -s as serial output file\n");
	break;
      }
#ifdef SOCKET_AVAIL
      // socket serial I/O by Alexandre Frey <Alexandre.Frey@trusted-logic.fr>
      case 'k':
	{
	  class cl_f *listener, *fin, *fout;
	  int serverport;
	  char *s;
	  
	  if (k_done) {
	    fprintf(stderr, "Serial input specified more than once.\n");
	  }
	  k_done= true;

	  serverport = strtol(optarg, 0, 0);
	  listener= mk_srv(serverport);
	  fprintf(stderr, "Listening on port %d for a serial connection.\n",
		  serverport);
	  if (srv_accept(listener, &fin, &fout)!=0)
	    {
	      fprintf(stderr, "Error accepting connection on port %d\n", serverport);
	      return 4;
	    }
	  fprintf(stderr, "Serial connection established.\n");
	  s= format_string("\0010x%llx", (unsigned long long int)(fin));
	  if (!options->set_value("serial0_in_file", this, /*(void*)Ser_in*/s))
	    fprintf(stderr, "Warning: No \"serial0_in_file\" option found to "
		    "set parameter of -s as serial input file\n");
	  s= format_string("\0010x%llx", (unsigned long long int)(fout));
	  if (!options->set_value("serial0_out_file", this, /*Ser_out*/s))
	    fprintf(stderr, "Warning: No \"serial0_out_file\" option found "
		    "to set parameter of -s as serial output file\n");
	  break;
	}
#endif
      case 'S':
	{
	  char *iname= NULL, *oname= NULL;
	  int uart=0, port=0;
	  subopts= optarg;
	  while (*subopts != '\0')
	    {
	      switch (get_sub_opt(&subopts, S_opts, &value))
		{
		case SOPT_IN:
		  if (value == NULL) {
		    fprintf(stderr, "No value for -S in\n");
		    exit(1);
		  }
		  iname= value;
		  break;
		case SOPT_OUT:
		  if (value == NULL) {
		    fprintf(stderr, "No value for -S out\n");
		    exit(1);
		  }
		  oname= value;
		  break;
		case SOPT_UART: case SOPT_USART:
		  uart= strtol(value, 0, 0);
		  break;
		case SOPT_PORT:
		  port= strtol(value, 0, 0);
		  break;
		default:
		  /* Unknown suboption. */
		  fprintf(stderr, "Unknown suboption `%s' for -S\n", value);
		  exit(1);
		  break;
		}
	    }
	  if (!iname && !oname && (port<=0))
	    {
	      fprintf(stderr, "Suboption missing for -S\n");
	    }
	  else
	    {
	      char *s, *h;
	      class cl_option *o;
	      if (iname)
		{
		  s= format_string("serial%d_in_file", uart);
		  if ((o= options->get_option(s)) == NULL)
		    {
		      h= format_string("Input file for serial line uart%d (-S)", uart);
		      o= new cl_pointer_option(this, s, h);
		      o->init();
		      o->hide();
		      options->add(o);
		      free(h);
		      printf("app creates %s option @%p\n",s,o);
		    }
		  options->set_value(s, this, /*(void*)Ser_in*/iname);
		  free(s);
		}
	      if (oname)
		{
		  s= format_string("serial%d_out_file", uart);
		  if ((o= options->get_option(s)) == NULL)
		    {
		      h= format_string("Output file for serial line uart%d (-S)", uart);
		      o= new cl_pointer_option(this, s, h);
		      o->init();
		      o->hide();
		      options->add(o);
		      free(h);
		      printf("app creates %s option @%p\n",s,o);
		    }
		  options->set_value(s, this, /*(void*)Ser_out*/oname);
		  free(s);
		}
	      if (port > 0)
		{
		  s= format_string("serial%d_port", uart);
		  if ((o= options->get_option(s)) == NULL)
		    {
		      h= format_string("Use localhost:port for serial line uart%d (-S)", uart);
		      o= new cl_number_option(this, s, h);
		      o->init();
		      o->hide();
		      options->add(o);
		      free(h);
		    }
		  options->set_value(s, this, (long)port);
		  free(s);
		}
	    }
	  break;
	}
      case 'I':
	{
	  char *ifstr= NULL, *in= NULL, *out= NULL;
	  subopts= optarg;
	  while (*subopts != '\0')
	    {
	      switch (get_sub_opt(&subopts, I_opts, &value))
		{
		case IOPT_IF:
		  if (value == NULL) {
		    fprintf(stderr, "No value for -I if\n");
		    exit(1);
		  }
		  ifstr= value;
		  break;
		case IOPT_IN:
		  if (value == NULL) {
		    fprintf(stderr, "No value for -I in\n");
		    exit(1);
		  }
		  in= value;
		  break;
		case IOPT_OUT:
		  if (value == NULL) {
		    fprintf(stderr, "No value for -I out\n");
		    exit(1);
		  }
		  out= value;
		  break;
		}
	    }
	  if (ifstr)
	    {
	      char *s= strtok(ifstr, "[]:.");
	      if (s)
		{
		  options->set_value("simif_memory", this, s);
		  s= strtok(NULL, "[]:.");
		  if (s)
		    {
		      long l= strtol(s, 0, 0);
		      options->set_value("simif_address", this, l);
		    }
		}
	    }
	  if (in)
	    {
	      options->set_value("simif_infile", this, in);
	    }
	  if (out)
	    {
	      options->set_value("simif_outfile", this, out);
	    }
	  break;
	}
      case 'h':
	print_help(cchars("s51"));
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
	  printf("%-20s%-30s%-30s\n", "Parameter", "Family", "Subtype");
	  while (cpus[i].type_str != NULL)
	    {
	      printf("%-20s%-30s%-30s\n",
		     cpus[i].type_str,
		     cpus[i].type_help,
		     cpus[i].sub_help);
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
/*
class cl_cmd *
cl_app::get_cmd(class cl_cmdline *cmdline)
{
  return(0);
}
*/
long
cl_app::eval(chars expr)
{
  expr_result= 0;
  uc_yy_set_string_to_parse((char*)expr);
  yyparse();
  uc_yy_free_string_to_parse();
  return expr_result;
}

void
cl_app::exec(chars line)
{
  class cl_console_base *c= commander->frozen_console;
  if (c == NULL)
    {
      c= new cl_console_dummy();
      c->init();
    }
  class cl_cmdline *cmdline= new cl_cmdline(this, (char*)line, c);
  do
    {
      cmdline->init();
      class cl_cmd *cm= commander->cmdset->get_cmd(cmdline, false/*c->is_interactive()*/);
      if (cm)
	{
	  cm->work(this, cmdline, c);
	}
      else if (cmdline->get_name() != 0)
	{
	  char *e= cmdline->cmd;
	  if (strlen(e) > 0)
	    {
	      long l= eval(e);
	      c->dd_printf("%ld\n", l);
	    }
	}
    }
  while (cmdline->restart_at_rest());
  delete cmdline;
  if (c != commander->frozen_console)
    delete c;
}

/*
 * Messages to broadcast
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

  cmdset->add(cmd= new cl_jaj_cmd("jaj", 0,
"jaj [val]          Jaj",
"long help of jaj "));
  cmd->init();

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
    cset->add(cmd= new cl_show_console("console", 0,
				       "",
				       ""));
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
    cset->add(cmd= new cl_show_error_cmd("error", 0,
"get error          Get class of errors",
"long help of get error"));
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
    cset->add(cmd= new cl_set_console_cmd("console", 0,
"set console interactive [on|off]|noninteractive|raw|edited\n"
"                   Set console parameters",
"long help of set console"));
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

  options->new_option(o= new cl_bool_option(this, "writable_flash",
					    "Use writable flash chip (-w)"));
  o->init();
  o->set_value((bool)false);
  
  options->new_option(o= new cl_bool_option(this, "null_prompt",
					    "Use \\0 as prompt (-P)"));
  o->init();

  options->new_option(o= new cl_string_option(this, "serial0_in_file",
					      "Input file for serial line uart0 (-S)"));
  o->init();
  o->hide();

  options->new_option(o= new cl_string_option(this, "serial0_out_file",
					      "Output file for serial line uart0 (-S)"));
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

  options->new_option(o= new cl_bool_option(this, "go",
					    "Go on start (-g)"));
  o->init();
  o->hide();

  options->new_option(o= new cl_bool_option(this, "quit",
					    "Quit on stop (-G)"));
  o->init();
  o->hide();
  
  options->new_option(o= new cl_number_option(this, "var_size",
					      "Size of variable space (-a)"));
  o->init();
  o->set_value((long)0x100);
  o->hide();

  options->new_option(o= new cl_string_option(this, "simif_memory",
					      "Memory for simulator interface (-I)"));
  o->init();
  o->hide();
  options->new_option(o= new cl_number_option(this, "simif_address",
					      "Address for simulator interface (-I)"));
  o->init();
  o->hide();
  options->new_option(o= new cl_string_option(this, "simif_infile",
					      "Name of input file for simulator interface (-I)"));
  o->init();
  o->hide();
  options->new_option(o= new cl_string_option(this, "simif_outfile",
					      "Name of output file for simulator interface (-I)"));
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
