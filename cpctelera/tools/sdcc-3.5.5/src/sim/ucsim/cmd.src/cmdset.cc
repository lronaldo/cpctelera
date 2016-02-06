/*
 * Simulator of microcontrollers (cmd.src/cmdset.cc)
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

#include "ddconfig.h"

#include "cmdlexcl.h"

// prj
#include "i_string.h"
#include "utils.h"
#include "globals.h"

// sim.src
#include "simcl.h"

// local, cmd.src
#include "cmdsetcl.h"
#include "cmdutil.h"


/*
 * Command: run
 *----------------------------------------------------------------------------
 */

//int
//cl_run_cmd::do_work(class cl_sim *sim,
//                  class cl_cmdline *cmdline, class cl_console_base *con)
COMMAND_DO_WORK_SIM(cl_run_cmd)
{
  class cl_brk *b;
  t_addr start, end;
  class cl_cmd_arg *params[4]= { cmdline->param(0),
                                 cmdline->param(1),
                                 cmdline->param(2),
                                 cmdline->param(3) };

  if (params[0])
    if (!(params[0]->get_address(sim->uc, &start)))
      {
        con->dd_printf("Error: wrong start address\n");
        return(DD_FALSE);
      }
  if (params[1])
    if (!(params[1]->get_address(sim->uc, &end)))
      {
        con->dd_printf("Error: wromg end address\n");
        return(DD_FALSE);
      }
  if (params[0])
    {
      if (!sim->uc->inst_at(start))
        con->dd_printf("Warning: maybe not instruction at 0x%06lx\n", start);
      sim->uc->PC= start;
      if (params[1])
        {
          if (start == end)
            {
              con->dd_printf("Addresses must be different.\n");
              return(DD_FALSE);
            }
          if ((b= sim->uc->fbrk_at(end)))
            {
            }
          else
            {
              b= new cl_fetch_brk(sim->uc->address_space(MEM_ROM_ID),
                                  sim->uc->make_new_brknr(), end,
                                  brkDYNAMIC, 1);
              sim->uc->fbrk->add_bp(b);
            }
        }
    }
  con->dd_printf("Simulation started, PC=0x%06x\n", sim->uc->PC);
  if (sim->uc->fbrk_at(sim->uc->PC))
    sim->uc->do_inst(1);

  sim->start(con);
  return(DD_FALSE);
}


/*
 * Command: stop
 *----------------------------------------------------------------------------
 */

//int
//cl_stop_cmd::do_work(class cl_sim *sim,
//                   class cl_cmdline *cmdline, class cl_console_base *con)
COMMAND_DO_WORK_SIM(cl_stop_cmd)
{
  sim->stop(resUSER);
  sim->uc->print_disass(sim->uc->PC, con);
  return(DD_FALSE);
}


/*
 * Command: step
 *----------------------------------------------------------------------------
 */

//int
//cl_step_cmd::do_work(class cl_sim *sim,
//                   class cl_cmdline *cmdline, class cl_console_base *con)
COMMAND_DO_WORK_UC(cl_step_cmd)
{
  int instrs = 1;
  class cl_cmd_arg *parm= cmdline->param(0);
  if (parm != NULL)
    instrs = parm->i_value;
  if (instrs <= 0)
    instrs = 1;
  uc->do_inst(instrs);
  uc->print_regs(con);
  return(0);
}


/*
 * Command: next
 *----------------------------------------------------------------------------
 */

//int
//cl_next_cmd::do_work(class cl_sim *sim,
//                   class cl_cmdline *cmdline, class cl_console_base *con)
COMMAND_DO_WORK_SIM(cl_next_cmd)
{
  class cl_brk *b;
  t_addr next;
  int branch;
  int inst_len;

#if 0
  struct dis_entry *de;
  t_mem code= sim->uc->get_mem(MEM_ROM, sim->uc->PC);
  int i= 0;
  de= &(sim->uc->dis_tbl()[i]);
  while ((code & de->mask) != de->code &&
         de->mnemonic)
    {
      i++;
      de= &(sim->uc->dis_tbl()[i]);
    }
#endif

  branch = sim->uc->inst_branch(sim->uc->PC);
  inst_len = sim->uc->inst_length(sim->uc->PC);

  if ((branch == 'a') || (branch == 'l'))
    {
      next= sim->uc->PC + inst_len;
      if (!sim->uc->fbrk_at(next))
        {
          b= new cl_fetch_brk(sim->uc->address_space(MEM_ROM_ID),
                              sim->uc->make_new_brknr(),
                              next, brkDYNAMIC, 1);

          b->init();
//        sim->uc->fbrk->add_bp(b);

          sim->uc->fbrk->add(b);
          b->activate();
        }
      if (sim->uc->fbrk_at(sim->uc->PC))
        sim->uc->do_inst(1);
      sim->start(con);
      //sim->uc->do_inst(-1);
    }
  else {
    sim->uc->do_inst(1);
    sim->uc->print_regs(con);
  }
  return(DD_FALSE);
}


/*
 * Command: help
 *----------------------------------------------------------------------------
 */

//int
//cl_help_cmd::do_work(class cl_sim *sim,
//                   class cl_cmdline *cmdline, class cl_console_base *con)
COMMAND_DO_WORK_APP(cl_help_cmd)
{
  class cl_commander_base *commander;
  class cl_cmdset *cmdset= 0;
  int i;
  class cl_cmd_arg *parm= cmdline->param(0);

  if ((commander= app->get_commander()) != 0)
    cmdset= commander->cmdset;
  if (!cmdset)
    return(DD_FALSE);
  if (!parm) {
    for (i= 0; i < cmdset->count; i++)
      {
        class cl_cmd *c= (class cl_cmd *)(cmdset->at(i));
        if (c->short_help)
          con->dd_printf("%s\n", c->short_help);
        else
          con->dd_printf("%s\n", (char*)(c->names->at(0)));
      }
  }
  else
    {
      matches= 0;
      do_set(cmdline, 0, cmdset, con);
      if (matches == 1 &&
          cmd_found)
        {
          int names;
          con->dd_printf("Names of command:");
          for (names= 0; names < cmd_found->names->count; names++)
            con->dd_printf(" %s", (char*)(cmd_found->names->at(names)));
          con->dd_printf("\n");
          class cl_cmdset *subset= cmd_found->get_subcommands();
          if (subset)
            {
              con->dd_printf("\"%s\" must be followed by the name of a "
                             "subcommand\nList of subcommands:\n",
                             (char*)(cmd_found->names->at(0)));
              for (i= 0; i < subset->count; i++)
                {
                  class cl_cmd *c=
                    dynamic_cast<class cl_cmd *>(subset->object_at(i));
                  con->dd_printf("%s\n", c->short_help);
                }
            }
          if (cmd_found->long_help)
            con->dd_printf("%s\n", cmd_found->long_help);
        }
      if (!matches ||
          !cmd_found)
        con->dd_printf("No such command.\n");
      //return(DD_FALSE);
      /*
      int pari;
      for (pari= 0; pari < cmdline->nuof_params(); pari++)
        {
          class cl_cmd_arg *act_param;
          act_param= (class cl_cmd_arg *)(cmdline->param(pari));
          for (i= 0; i < cmdset->count; i++)
            {
              class cl_cmd *c= (class cl_cmd *)(cmdset->at(i));
              if (!c->name_match(act_param->s_value, DD_FALSE))
                continue;
              if (c->short_help)
                con->dd_printf("%s\n", c->short_help);
              else
                con->dd_printf("%s\n", (char*)(c->names->at(0)));
              if (pari < cmdline->nuof_params()-1)
                continue;
              cmdset= c->get_subcommands();
              if (!cmdset)
                return(DD_FALSE);
            }
        }
      return(DD_FALSE);
      */
    }
  return(DD_FALSE);
  /*
  if (cmdline->syntax_match(0, STRING)) {
    matches= 0;
    for (i= 0; i < cmdset->count; i++)
      {
        c= (class cl_cmd *)(cmdset->at(i));
        if (c->name_match(parm->value.string.string, DD_FALSE))
          matches++;
      }
    if (!matches)
      con->dd_printf("No such command\n");
    else if (matches > 1)
      for (i= 0; i < cmdset->count; i++)
        {
          c= (class cl_cmd *)(cmdset->at(i));
          if (!c->name_match(parm->value.string.string, DD_FALSE))
            continue;
          if (c->short_help)
            con->dd_printf("%s\n", c->short_help);
          else
            con->dd_printf("%s\n", (char*)(c->names->at(0)));
        }
    else
      for (i= 0; i < cmdset->count; i++)
        {
          c= (class cl_cmd *)(cmdset->at(i));
          if (!c->name_match(parm->value.string.string, DD_FALSE))
            continue;
          if (c->short_help)
            con->dd_printf("%s\n", c->short_help);
          else
            con->dd_printf("%s\n", (char*)(c->names->at(0)));
          int names;
          con->dd_printf("Names of command:");
          for (names= 0; names < c->names->count; names++)
            con->dd_printf(" %s", (char*)(c->names->at(names)));
          con->dd_printf("\n");
          if (c->long_help)
            con->dd_printf("%s\n", c->long_help);
          else
            con->dd_printf("%s\n", (char*)(c->names->at(0)));
        }
  }
  else
    con->dd_printf("%s\n", short_help?short_help:"Error: wrong syntax");

  return(0);
  */
}

bool
cl_help_cmd::do_set(class cl_cmdline *cmdline, int pari,
                    class cl_cmdset *cmdset,
                    class cl_console_base *con)
{
  int i;
  for (i= 0; i < cmdset->count; i++)
    {
      class cl_cmd *cmd= dynamic_cast<class cl_cmd *>(cmdset->object_at(i));
      if (!cmd)
        continue;
      if (pari >= cmdline->nuof_params())
        return(DD_FALSE);
      class cl_cmd_arg *param= cmdline->param(pari);
      if (!param)
        return(DD_FALSE);
      class cl_cmdset *next_set= cmd->get_subcommands();
      if (cmd->name_match(param->s_value, DD_FALSE))
        {
          if (pari+1 >= cmdline->nuof_params())
            {
              matches++;
              cmd_found= cmd;
              if (cmd->short_help)
                con->dd_printf("%s\n", cmd->short_help);
              else
                con->dd_printf("%s\n", (char*)(cmd->names->at(0)));
              //continue;
            }
          else
            if (next_set)
              do_set(cmdline, pari+1, next_set, con);
        }
    }
  return(DD_TRUE);
}


/*
 * Command: quit
 *----------------------------------------------------------------------------
 */

//int
//cl_quit_cmd::do_work(class cl_sim *sim,
//                   class cl_cmdline */*cmdline*/, class cl_console_base */*con*/)
COMMAND_DO_WORK(cl_quit_cmd)
{
  return(1);
}


/*
 * Command: kill
 *----------------------------------------------------------------------------
 */

//int
//cl_kill_cmd::do_work(class cl_sim *sim,
//                   class cl_cmdline */*cmdline*/, class cl_console_base */*con*/)
COMMAND_DO_WORK_APP(cl_kill_cmd)
{
  app->going= 0;
  if (app->sim)
    app->sim->state|= SIM_QUIT;
  return(1);
}


/*
 * EXEC file
 */

COMMAND_DO_WORK_APP(cl_exec_cmd)
{
  class cl_cmd_arg *parm= cmdline->param(0);
  char *fn= 0;

  if (cmdline->syntax_match(0, STRING)) {
    fn= parm->value.string.string; 
  }
  else
    con->dd_printf("%s\n", short_help?short_help:"Error: wrong syntax\n");

  class cl_commander_base *c= app->get_commander();
  class cl_console_base *cons= con->clone_for_exec(fn);
  if (cons)
    {
      cons->flags|= CONS_NOWELCOME;
      c->add_console(cons);
    }

  return(DD_FALSE);
}


/*
 * expression expression
 */

COMMAND_DO_WORK_APP(cl_expression_cmd)
{
  //con->dd_printf("\"%s\"\n", cmdline->cmd);
  char *s= cmdline->cmd;
  if (!s ||
      !*s)
    return(DD_FALSE);
  int i= strspn(s, " \t\v\n");
  s+= i;
  //con->dd_printf("\"%s\"\n", s);
  i= strspn(s, "abcdefghijklmnopqrstuvwxyz");
  s+= i;
  uc_yy_set_string_to_parse(s);
  yyparse();
  uc_yy_free_string_to_parse();
  return(DD_FALSE);
}


/* End of cmd.src/cmdset.cc */
