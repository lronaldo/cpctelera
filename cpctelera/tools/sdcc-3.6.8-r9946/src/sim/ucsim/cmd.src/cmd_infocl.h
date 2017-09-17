/*
 * Simulator of microcontrollers (cmd.src/infocl.h)
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

#ifndef CMD_CMD_INFOCL_HEADER
#define CMD_CMD_INFOCL_HEADER

#include "newcmdcl.h"


// INFO BP
COMMAND_ON(uc,cl_info_bp_cmd);

// INFO REG
COMMAND_ON(uc,cl_info_reg_cmd);

// INFO HW
COMMAND_ON(uc,cl_info_hw_cmd);

// INFO STACK
COMMAND_ON(uc,cl_info_stack_cmd);

// INFO MEMORY
COMMAND_ON(uc,cl_info_memory_cmd);

// INFO VARIABLES
COMMAND_ON(uc,cl_info_var_cmd);


#endif

/* End of cmd.src/cmd_infocl.h */
