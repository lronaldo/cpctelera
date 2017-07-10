/*
 * Simulator of microcontrollers (cmd.src/cmdmemcl.h)
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

#ifndef CMD_CMD_MEM_HEADER
#define CMD_CMD_MEM_HEADER

#include "newcmdcl.h"


// MEMORY CREATE CHIP
COMMAND_ON(uc,cl_memory_create_chip_cmd);

// MEMORY CREATE ADDRESSSPACE
COMMAND_ON(uc,cl_memory_create_addressspace_cmd);

// MEMORY CREATE ADDRESSDECODER
COMMAND_ON(uc,cl_memory_create_addressdecoder_cmd);

// MEMORY CREATE BANKER
COMMAND_ON(uc,cl_memory_create_banker_cmd);

// MEMORY CREATE BANDER
COMMAND_ON(uc,cl_memory_create_bander_cmd);

// MEMORY CREATE BANK
COMMAND_ON(uc,cl_memory_create_bank_cmd);

// MEMORY CELL
COMMAND_ON(uc,cl_memory_cell_cmd);


#endif

/* End of cmd.src/cmd_memcl.h */
