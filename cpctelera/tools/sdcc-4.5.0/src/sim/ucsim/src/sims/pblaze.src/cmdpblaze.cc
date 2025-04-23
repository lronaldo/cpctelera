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


// sim.src
#include "simcl.h"

// local
#include "pblazecl.h"
#include "cmdpblazecl.h"

COMMAND_DO_WORK_UC(cl_pbstate_cmd)
{
  char *fname = NULL;
  if (cmdline->param(0) != 0) {
    fname = cmdline->param(0)->get_svalue();
  }

  ((cl_pblaze *) uc)->print_state(con, fname);
  return(0);
}

CMDHELP(cl_pbstate_cmd,
	"pbstate [\"file\"]",
	"Prints PicoBlaze state to std output or specified file",
	"long help of pbstate")

COMMAND_DO_WORK_UC(cl_import_pbstate_cmd)
{
  char *fname = NULL;
  if (cmdline->param(0) != 0) {
    fname = cmdline->param(0)->get_svalue();
    ((cl_pblaze *) uc)->load_state(con, fname);
  }
  else {
    con->dd_printf("%s\n", short_help.c_str());
  }

  return(0);
}

CMDHELP(cl_import_pbstate_cmd,
	"pbstate \"file\"",
	"Loads Picoblaze state from xml file",
	"long help of import pbstate")


COMMAND_DO_WORK_UC(cl_import_interrupts_cmd)
{
  char *fname = NULL;
  if (cmdline->param(0) != 0) {
    fname = cmdline->param(0)->get_svalue();

    cl_option * opt = uc->sim->app->options->get_option("pblaze_interrupt_file");
    if (opt == NULL) {
        uc->sim->app->options->new_option(opt = new cl_string_option(this, "pblaze_interrupt_file","File with interrupts is specified"));
        opt->init();
    }

    opt->set_value(fname);

    ((cl_pblaze *) uc)->read_interrupt_file();
  }
  else {
    con->dd_printf("%s\n", short_help.c_str());
  }

  return(0);
}

CMDHELP(cl_import_interrupts_cmd,
	"interrupts \"file\"",
	"Loads interrupts from xml file",
	"long help of import interrupts")


COMMAND_DO_WORK_UC(cl_import_input_cmd)
{
  char *fname = NULL;
  if (cmdline->param(0) != 0) {
    fname = cmdline->param(0)->get_svalue();

    cl_option * opt = uc->sim->app->options->get_option("pblaze_input_file");
    if (opt == NULL) {
        uc->sim->app->options->new_option(opt = new cl_string_option(this, "pblaze_input_file","File with inputs is specified"));
        opt->init();
    }
    opt->set_value(fname);

    ((cl_pblaze *) uc)->read_input_file();
  }
  else {
    con->dd_printf("%s\n", short_help.c_str());
  }

  return(0);
}

CMDHELP(cl_import_input_cmd,
	"input \"file\"",
	"Loads input from xml file",
	"long help of import input")


COMMAND_DO_WORK_UC(cl_get_output_cmd)
{
  char *fname = NULL;
  if (cmdline->param(0) != 0) {
    fname = cmdline->param(0)->get_svalue();
  }

    ((cl_pblaze *) uc)->output_port->print_outputs(fname);

  return(0);
}

CMDHELP(cl_get_output_cmd,
	"output [\"file\"]",
	"Prints realized outputs of PicoBlaze",
	"long help of get output")

/* ENd of pbalze.src/cmdpblaze.cc */
