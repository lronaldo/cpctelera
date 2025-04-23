/*
 * Copyright (C) 2012-2013 Jiří Šimek
 * Copyright (C) 2013 Zbyněk Křivka <krivka@fit.vutbr.cz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


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
#include "apppblazecl.h"
#include "optioncl.h"
#include "globals.h"

// sim.src
#include "simcl.h"

// cmd.src
#include "newcmdcl.h"
#include "cmdutil.h"
#include "cmd_confcl.h"
#include "cmd_showcl.h"
#include "cmd_getcl.h"
#include "cmd_setcl.h"
//#ifdef _WIN32
//#include "newcmdwin32cl.h"
//#else
#include "newcmdposixcl.h"
//#endif

/* XXX */
#define PORTVERSION "0.2.0"
/*
 * Interpretation of parameters
 */

static void
print_help(const char *name)
{
  printf("%s: %s (uCsim %s)\n", name, PORTVERSION, VERSIONSTR);
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
     "  -t CPU       Type of CPU: kcpsm3 or kcpsm6\n"
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

     "  -x           Input file format isn't Intel HEX file, but raw HEX file for PicoBlaze\n"
     "  -i file      Read file with interrupts\n"
     "  -I value     Address of interrupt vector\n"
     "  -m value     Size of RAM (only for KCPSM6). Valid values are 64, 128 and 256.\n"
     "  -M value     Size of ROM (only for KCPSM6). Valid values are 1024, 2048 and 4096.\n"
     "  -n file      Loads file with inputs.\n"
     "  -w value     Built-in hardware constant (only for KCPSM6).\n"
     "  -o file      At the end of simulation saves PicoBlaze state and PicoBlaze outputs into files <file>_state.xml and <file>_outputs.xml.\n"
     "\n"
     "Authors: Jiri Simek and Zbynek Krivka (krivka@fit.vutbr.cz)\n"
     "\n"
     "Acknowledgement: This software tool has been elaborated in the\n"
     "framework of the IT4Innovations Centre of Excellence project, \n"
     "reg. no. CZ.1.05/1.1.00/02.0070 supported by Operational Programme\n"
     "'Research and Development for Innovations' funded by Structural Funds\n" 
     "of the European Union and state budget of the Czech Republic.\n"
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
cl_apppblaze::proc_arguments(int argc, char *argv[])
{
  int i, c;
  char opts[100], *cp, *subopts, *value;
  char *cpu_type= NULL;
  bool s_done= false, k_done= false;
  bool S_i_done= false, S_o_done= false;

  strcpy(opts, "c:C:p:PX:vVt:s:S:hHk:xi:I:m:M:w:n:o:");
#ifdef SOCKET_AVAIL
  strcat(opts, "Z:r:");
#endif

  while((c= getopt(argc, argv, opts)) != -1)
    switch (c)
      {
    /* ADDED by Jiri Simek */
      case 'x':
      {

        class cl_option *o;
        options->new_option(o= new cl_bool_option(this, "pblaze_hex","Read pblaze hex file"));
        o->init();

        if (!options->set_value("pblaze_hex", this, (bool)true))
          fprintf(stderr, "Warning: Cannot set option -x");

          o = options->get_option("pblaze_hex");
          bool v;
          o->get_value(&v);
        break;
      }
      case 'i':
      {
        class cl_option *o;
        options->new_option(o= new cl_string_option(this, "pblaze_interrupt_file","File with interrupts is specified"));
        o->init();

        if (!options->set_value("pblaze_interrupt_file", this, optarg))
          fprintf(stderr, "Warning: No \"pblaze_interrupt_file\" option found to set "
                  "parameter of -i as interrupt file\n");
        break;
      }
      case 'I':
      {
        class cl_option *o;
        options->new_option(o= new cl_number_option(this, "pblaze_interrupt_vector","Address of interrupt vector"));
        o->init();

        if (!options->set_value("pblaze_interrupt_vector", this, optarg))
          fprintf(stderr, "Warning: No \"pblaze_interrupt_vector\" option found to set "
                  "parameter of -I as interrupt vector\n");
        break;
      }
      case 'm':
      {
        class cl_option *o;
        options->new_option(o= new cl_number_option(this, "pblaze_ram_size","Set RAM size"));
        o->init();

        if (!options->set_value("pblaze_ram_size", this, strtol(optarg, NULL, 0)))
          fprintf(stderr, "Warning: No \"pblaze_ram_size\" option found to set "
                  "parameter of -m as RAM size\n");
        break;
      }
      case 'n':
      {
        class cl_option *o;
        options->new_option(o= new cl_string_option(this, "pblaze_input_file","File with inputs is specified"));
        o->init();

        if (!options->set_value("pblaze_input_file", this, optarg))
          fprintf(stderr, "Warning: No \"pblaze_input_file\" option found to set "
                  "parameter of -i as interrupt file\n");
        break;
      }
      case 'M':
      {
        class cl_option *o;
        options->new_option(o= new cl_number_option(this, "pblaze_rom_size","Set ROM size"));
        o->init();

        if (!options->set_value("pblaze_rom_size", this, strtol(optarg, NULL, 0)))
          fprintf(stderr, "Warning: No \"pblaze_rom_size\" option found to set "
                  "parameter of -M as ROM size\n");
        break;
      }
      case 'w':
      {
        class cl_option *o;
        options->new_option(o= new cl_number_option(this, "pblaze_hw_const","Builtin hardware constant"));
        o->init();

        if (!options->set_value("pblaze_hw_const", this, strtol(optarg, NULL, 0)))
          fprintf(stderr, "Warning: No \"pblaze_hw_const\" option found to set "
                  "parameter of -w as builtin hardware constant\n");
        break;
      }
      case 'o':
      {
        class cl_option *o;
        options->new_option(o= new cl_string_option(this, "pblaze_output_file","Part of file names with PicoBlaze state and outputs at the end of simulation"));
        o->init();

        if (!options->set_value("pblaze_output_file", this, optarg))
          fprintf(stderr, "Warning: No \"pblaze_output_file\" option found to set "
                  "parameter of -o as output files\n");
        break;
      }



      // default parameters
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
              fprintf(stderr, "Xtal frequency must be greater than 0\n");
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
#ifdef _WIN32
        /* TODO: this code should be probably used for all platforms? */
        FILE *Ser;
        if (s_done)
          {
            fprintf(stderr, "-s option can not be used more than once.\n");
            break;
          }
        s_done= true;
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
        s_done= true;
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
          k_done= true;

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
              S_i_done= true;
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
        print_help("spblaze");
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

/* End of pblaze.src/apppblaze.cc */
