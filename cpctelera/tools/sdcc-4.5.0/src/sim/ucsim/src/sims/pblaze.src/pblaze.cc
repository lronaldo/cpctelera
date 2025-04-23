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

#include <stdarg.h> /* for va_list */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include "i_string.h"
#include <list>
#include <algorithm>

#include "tinyxml.h"

// prj
#include "globals.h"
#include "utils.h"
#include "pobjcl.h"

// sim
#include "simcl.h"
#include "dregcl.h"

//cmd
#include "cmd_uccl.h"

// local
#include "pblazecl.h"
#include "glob.h"
#include "regspblaze.h"
#include "pblazemac.h"
#include "interruptcl.h"
#include "portidcl.h"
#include "inputportcl.h"
#include "outputportcl.h"
#include "cmdpblazecl.h"

//#define uint32 t_addr
//#define uint8 unsigned char

using namespace std;
/*******************************************************************/


/*
 * Base type of PicoBlaze controllers
 */

cl_pblaze::cl_pblaze(struct cpu_entry *cputype, class cl_sim *asim):
  cl_uc(asim)
{
  PCmask= 0x3ff;
  type = cputype;
}

cl_pblaze::~cl_pblaze(void) {
  // if option -o was specified, at the end of simulation state and outputs are saved into file
  cl_option * opt = sim->app->options->get_option("pblaze_output_file");
  if (opt != NULL) {
    char *file_prefix;
    opt->get_value(&file_prefix);
    char file_name[256];

    sprintf(file_name, "%s_state.xml", file_prefix);
    print_state(NULL, file_name);

    sprintf(file_name, "%s_outputs.xml", file_prefix);
    output_port->print_outputs(file_name);
  }
}

int
cl_pblaze::init(void)
{
  // inits RAM and ROM size and interrupt vector
  init_uc_parameters();

  cl_uc::init(); /* Memories now exist */

  //set_xtal(8000000);

  // loading file with interrupts
  read_interrupt_file();
  read_input_file();

  return(0);
}


void
cl_pblaze::reset(void)
{
  cl_uc::reset();

  sfr->set(FLAGS, 0);
  sfr->set(SP, 0);

  active_regbank = REGISTER_BANK_A;
}


const char *
cl_pblaze::id_string(void)
{
  int i = 0;
  while ((cpus_pblaze[i].type != 0) && (type->type != cpus_pblaze[i].type))
    i++;

  return(cpus_pblaze[i].type_str);
}


/*
 * Making elements of the controller
 */
t_addr
cl_pblaze::get_mem_size(enum mem_class type)
{
  switch(type)
    {
    case MEM_ROM: return rom_size;
    case MEM_IRAM: return ram_size;
    case MEM_STACK: return stack_size;
    default: return(0);
    }
}


void
cl_pblaze::mk_hw_elements(void)
{
  class cl_hw *h;
  cl_uc::mk_hw_elements();

  add_hw(h= new cl_dreg(this, 0, "dreg"));
  h->init();
  
  hws->add(interrupt = new cl_interrupt(this));
  interrupt->init();

  hws->add(port_id = new cl_port_id(this));
  port_id->init();

  hws->add(output_port = new cl_output_port(this));
  output_port->init();

  hws->add(input_port = new cl_input_port(this));
  input_port->init();
}

void
cl_pblaze::make_memories(void)
{
  class cl_address_space *as;


  // REGISTERS simulated in SFR memory
  sfr= as= new cl_address_space(MEM_SFR_ID, 0, 0x22, 8);
  as->init();
  address_spaces->add(as);

  // ROM 1k 18b
  rom = as = new cl_address_space(MEM_ROM_ID, 0, rom_size, 18); // picoblaze ma 18b sirokou pamet. kvuli kompatibilite s ucsim (cleneni po bytech) musi byt zvetstena na 24b = 3B
  as->init();
  address_spaces->add(as);


  // RAM 8b (size for KCPSM3 is 64, for KCPSM6
  ram = as= new cl_address_space(MEM_IRAM_ID, 0, ram_size, 8);
  as->init();
  address_spaces->add(as);

  // stack 31 10b
  stack = as = new cl_address_space(MEM_STACK_ID, 0, stack_size, 12);
  as->init();
  address_spaces->add(as);

  class cl_address_decoder *ad;
  class cl_memory_chip *chip;

  // sfr chip init
  chip = new cl_chip8("sfr_chip", 0x22, 8);
  chip->init();
  memchips->add(chip);
  ad = new cl_address_decoder(as = address_space(MEM_SFR_ID), chip, 0, 0x21, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  // rom chip init
  chip = new cl_chip32("rom_chip", rom_size, 18);
  chip->init();
  memchips->add(chip);
  ad = new cl_address_decoder(as = address_space(MEM_ROM_ID), chip, 0, rom_size-1, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  // ram chip init
  chip= new cl_chip8("ram_chip", ram_size, 8);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as = address_space(MEM_IRAM_ID), chip, 0, ram_size-1, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);

  // stack chip
  chip = new cl_chip8("stack_chip", stack_size, 12);
  chip->init();
  memchips->add(chip);
  ad= new cl_address_decoder(as = address_space(MEM_STACK_ID), chip, 0, stack_size-1, 0);
  ad->init();
  as->decoders->add(ad);
  ad->activate(0);
}

void
cl_pblaze::build_cmdset(class cl_cmdset *cmdset)
{
  class cl_cmd *cmd;
  class cl_cmdset *inner_cmd_set;
  class cl_super_cmd *super_cmd;

  cl_uc::build_cmdset(cmdset);
  /*
  cmdset->add(cmd= new cl_dc_cmd("dc", true,
"dc [start [stop]]  Dump ROM",
"long help of dc"));
  cmd->init();

  cmdset->add(cmd= new cl_di_cmd("di", true,
"di [start [stop]]  Dump Internal RAM",
"long help of di"));
  cmd->init();

  cmdset->add(cmd= new cl_dx_cmd("dx", true,
"dx [start [stop]]  Dump External RAM",
"long help of dx"));
  cmd->init();

  cmdset->add(cmd= new cl_ds_cmd("ds", true,
"ds [start [stop]]  Dump SFR",
"long help of ds"));
  cmd->init();
  */

  cmdset->add(cmd= new cl_pbstate_cmd("pbstate", 0));//true,
	      //"pbstate [\"file\"]   Prints PicoBlaze state to std output or specified file",
	      //"long help of pbstate"));
  cmd->init();
  

  { // import
    super_cmd= (class cl_super_cmd *)(cmdset->get_cmd("import"));
    if (super_cmd)
      inner_cmd_set= super_cmd->get_subcommands();
    else {
      inner_cmd_set= new cl_cmdset();
      inner_cmd_set->init();
    }

    inner_cmd_set->add(cmd= new cl_import_pbstate_cmd("pbstate", 0));//,
    //"import pbstate \"file\" Loads Picoblaze state from xml file",
    //"long help of import pbstate"));
    cmd->add_name("pbstate");
    cmd->init();

    inner_cmd_set->add(cmd= new cl_import_interrupts_cmd("interrupts", 0));//,
    //"import interrupts \"file\" Loads interrupts from xml file",
    //"long help of import interrupts"));
    cmd->add_name("interrupts");
    cmd->init();

    inner_cmd_set->add(cmd= new cl_import_input_cmd("input", 0));//,
    //"import input \"file\" Loads input from xml file",
    //"long help of import input"));
    cmd->add_name("input");
    cmd->init();
    
    if (!super_cmd) {
      cmdset->add(cmd= new cl_super_cmd("import", 0, inner_cmd_set));//,
      //"import subcommand  Import, see `import' command for more help",
      //"long help of import", inner_cmd_set));
      cmd->init();
    }
  }

  // print output
  super_cmd = (class cl_super_cmd *)(cmdset->get_cmd("get"));
  if (super_cmd)
    inner_cmd_set= super_cmd->get_subcommands();
  else {
    inner_cmd_set= new cl_cmdset();
    inner_cmd_set->init();
  }
  inner_cmd_set->add(cmd= new cl_get_output_cmd("output", 0));//,
  //"get output [\"file\"] Prints realized outputs of PicoBlaze",
  //"long help of get output"));
    cmd->add_name("output");
    cmd->init();
}




/*
 * Help command interpreter
 */

struct dis_entry *
cl_pblaze::dis_tbl(void)
{
      return disass_pblaze_x;
    //TODO doimplementovat, osetrit;
  //return(disass_pblaze);
}

struct name_entry *
cl_pblaze::sfr_tbl(void)
{
  return(sfr_tab);
}

struct name_entry *
cl_pblaze::bit_tbl(void)
{
  return(bit_tab);
}

// Returns length of instruction at given address
int
cl_pblaze::inst_length(t_addr addr)
{
  int len = 0;

  get_disasm_info(addr, &len, NULL, NULL);

  return len;
}

int
cl_pblaze::inst_branch(t_addr addr)
{
  int b;

  get_disasm_info(addr, NULL, &b, NULL);

  return b;
}

int
cl_pblaze::longest_inst(void)
{
  return 1;
}


const char *
cl_pblaze::get_disasm_info(t_addr addr,
                        int *ret_len,
                        int *ret_branch,
                        int *immed_offset)
{
  const char *b = NULL;
  uint code;
  int len = 0;
  int immed_n = 0;
  int i;
  int start_addr = addr;
  struct dis_entry_pblaze *dis_e;

  code= get_mem(MEM_ROM_ID, addr++);
  dis_e = NULL;

  i = 0;

  dis_entry_pblaze * disass_pblaze = (type->type == CPU_PBLAZE_3 ? disass_pblaze3 : disass_pblaze6);
  while ((code & disass_pblaze[i].mask) != disass_pblaze[i].code && disass_pblaze[i].instruction != BAD_OPCODE)
    i++;

  // found instruction record
  dis_e = &disass_pblaze[i];

  b = dis_e->mnemonic;
  if (b != NULL)
    len += (dis_e->length);

  if (ret_branch) {
    *ret_branch = dis_e->branch;
  }

  if (immed_offset) {
    if (immed_n > 0)
      *immed_offset = immed_n;
    else
      *immed_offset = (addr - start_addr);
  }

  if (len == 0)
    len = 1;

  if (ret_len)
    *ret_len = len;

  return b;
}

char *
cl_pblaze::disass(t_addr addr)
{
  chars work, temp;
  const char *b;
  int len = 0;
  int immed_offset = 0;
  bool first= true;
  
  work= "";

  b = get_disasm_info(addr, &len, NULL, &immed_offset);

  if (b == NULL)
    {
      return strdup("UNKNOWN/INVALID");
    }

  while (*b)
    {
      if ((*b == ' ') && first)
	{
	  first= false;
	  while (work.len() < 6) work.append(' ');
	}
      if (*b == '%')
        {
          b++;
	  temp= "";
          switch (*(b++))
            {
            case 'a': // 10b address
              temp.format("0x%x", (rom->get(addr) & 0x3ff));
              break;
            case 'A': // 12b address
              temp.format("0x%x", (rom->get(addr) & 0xfff));
              break;
            case 'k': // immediate value (constant)
              temp.format("0x%x", (rom->get(addr) & 0xff));
              break;
            case 'K': // immediate value (constant)
              temp.format("0x%x", (rom->get(addr) & 0xff0) >> 4);
              break;
            case 'm': // 6b address to RAM
              temp.format("0x%x", (rom->get(addr) & 0x3f));
              break;
            case 'M': // 8b address to RAM
              temp.format("0x%x", (rom->get(addr) & 0xff));
              break;
            case 'p': // 8b port number
              temp.format("%d", (rom->get(addr) & 0xff));
              break;
            case 'P': // 4b port number
              temp.format("%d", (rom->get(addr) & 0xf));
              break;
            case 'r': // register as first operand
              temp.format("s%x", (rom->get(addr) & 0xf00) >> 8);
              break;
            case 's': // register as second operand
              temp.format("s%x", (rom->get(addr) & 0xf0) >> 4);
              break;
            default:
              temp= "?";
              break;
            }
	  work+= temp;
        }
      else
	work+= *(b++);
    }

  return(strdup(work.c_str()));
}


void
cl_pblaze::print_regs(class cl_console_base *con)
{
  if (type->type == CPU_PBLAZE_6)
    con->dd_printf("Regbank A\n");

  for (int i=0; i<16; i++) {
    con->dd_printf("S%x= 0x%02x %3d %c\n",i, sfr->get(i), sfr->get(i), isprint(sfr->get(i))?sfr->get(i):'.');
  }

  if (type->type == CPU_PBLAZE_6) {
    con->dd_printf("Regbank B\n");

    for (int i=16; i<32; i++) {
      con->dd_printf("S%x= 0x%02x %3d %c\n",i, sfr->get(i), sfr->get(i), isprint(sfr->get(i))?sfr->get(i):'.');
    }
  }

  if (type->type == CPU_PBLAZE_6)
    con->dd_printf("Active regbank= %c\n", active_regbank == REGISTER_BANK_A ? 'A' : 'B');

  con->dd_printf("PC= 0x%06x\n", PC);
  con->dd_printf("SP= 0x%06x\n", sfr->get(SP));
  con->dd_printf("C= %d, Z= %d, I= %d\n", FLAGS_GET_C, FLAGS_GET_Z, FLAGS_GET_I);

  print_disass(PC, con);
}

/*
 * If file name is not specified, PicoBlaze state is printed to console. Otherwise state is saved into specified file
 */
void
cl_pblaze::print_state(class cl_console_base *con, char *file_name)
{
  const int buffer_size = 4096;
  char buffer[buffer_size];
  int written = 0;

  TiXmlDocument doc;
  TiXmlElement * root;
  TiXmlElement * element;
  TiXmlText * text;

	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "", "" );
	doc.LinkEndChild( decl );
  root = new TiXmlElement( "picoblaze" );
	root->SetAttribute("type", id_string());
	doc.LinkEndChild( root );

	// RAM
  element = new TiXmlElement( "ram" );
	element->SetAttribute("size", ram_size);
	element->SetAttribute("bytes", 1);
  for (unsigned int i = 0; i<ram_size; i++)
    written += snprintf(buffer + written, buffer_size - written - 1,"%02x", ram->get(i));
	text = new TiXmlText(buffer);
	element->LinkEndChild( text );
	root->LinkEndChild( element );

	// STACK
  element = new TiXmlElement( "stack" );
	element->SetAttribute("size", stack_size);
	element->SetAttribute("bytes", 2);
	written = 0;
  for (unsigned int i = 0; i<stack_size; i++)
    written += snprintf(buffer + written, buffer_size - written - 1, "%04x", stack->get(i));
	text = new TiXmlText(buffer);
	element->LinkEndChild( text );
	root->LinkEndChild( element );

	// REGISTERS
  TiXmlElement * registers = new TiXmlElement( "registers" );
	if (type->type == CPU_PBLAZE_6) {
    registers->SetAttribute("activeRegbank", active_regbank == REGISTER_BANK_A ? "A" : "B");
  }
	root->LinkEndChild( registers );

  // REGBANK A
  element = new TiXmlElement( "regbank" );
  if (type->type == CPU_PBLAZE_6) {
    element->SetAttribute("name", "A");
  }
	written = 0;
  for (unsigned int i = 0; i<16; i++)
    written += snprintf(buffer + written, buffer_size - written - 1, "%02x", sfr->get(i));
	text = new TiXmlText(buffer);
	element->LinkEndChild( text );
	registers->LinkEndChild( element );

  // REGBANK B
  if (type->type == CPU_PBLAZE_6) {
    element = new TiXmlElement( "regbank" );
      element->SetAttribute("name", "B");
    written = 0;
    for (unsigned int i = 16; i<32; i++)
    written += snprintf(buffer + written, buffer_size - written - 1, "%02x", sfr->get(i));
    text = new TiXmlText(buffer);
    element->LinkEndChild( text );
    registers->LinkEndChild( element );
  }

  // PC
  element = new TiXmlElement( "pc" );
  snprintf(buffer, buffer_size - 1, "%x", AU(PC));
  text = new TiXmlText(buffer);
  element->LinkEndChild( text );
  registers->LinkEndChild( element );

  // SP
  element = new TiXmlElement( "sp" );
  snprintf(buffer, buffer_size - 1, "%x", sfr->get(SP));
  text = new TiXmlText(buffer);
  element->LinkEndChild( text );
  registers->LinkEndChild( element );

  // FLAGS
  TiXmlElement * flags = new TiXmlElement( "flags" );
  registers->LinkEndChild( flags );

  // Carry
  element = new TiXmlElement( "carry" );
  snprintf(buffer, buffer_size - 1, "%x", FLAGS_GET_C ? 1:0);
  text = new TiXmlText(buffer);
  element->LinkEndChild( text );
  flags->LinkEndChild( element );
  // Zero
  element = new TiXmlElement( "zero" );
  snprintf(buffer, buffer_size - 1, "%x", FLAGS_GET_Z ? 1:0);
  text = new TiXmlText(buffer);
  element->LinkEndChild( text );
  flags->LinkEndChild( element );
  // IE
  element = new TiXmlElement( "interruptEnable" );
  snprintf(buffer, buffer_size - 1, "%x", FLAGS_GET_I ? 1:0);
  text = new TiXmlText(buffer);
  element->LinkEndChild( text );
  flags->LinkEndChild( element );


  if (file_name == NULL) {
    TiXmlPrinter printer;
    doc.Accept(&printer);
    con->dd_printf("%s", printer.CStr());
  }
  else {
    doc.SaveFile( file_name );
  }


    /*
  const int buffer_size = 4096;
  char buffer[buffer_size];
  int written;

  written = snprintf(buffer, buffer_size, "RAM:");
  for (unsigned int i = 0; i<ram_size; i++)
    written += snprintf(buffer + written, buffer_size - written - 1,"%02x", ram->get(i));
  written += snprintf(buffer + written, buffer_size - written - 1, "\n");

  written += snprintf(buffer + written, buffer_size - written - 1,"STACK:");
  for (unsigned int i = 0; i<stack_size; i++)
    written += snprintf(buffer + written, buffer_size - written - 1, "%02x", stack->get(i));
  written += snprintf(buffer + written, buffer_size - written - 1, "\n");

  if (type->type == CPU_PBLAZE_6) {
    written += snprintf(buffer + written, buffer_size - written - 1, "ACTIVE_REGBANK:%s\n", active_regbank == REGISTER_BANK_A ? "A" : "B");
  }

  written += snprintf(buffer + written, buffer_size - written - 1, "REGBANK_A:");
  for (unsigned int i = 0; i<16; i++)
    written += snprintf(buffer + written, buffer_size - written - 1, "%02x", sfr->get(i));
  written += snprintf(buffer + written, buffer_size - written - 1, "\n");

  if (type->type == CPU_PBLAZE_6) {
    written += snprintf(buffer + written, buffer_size - written - 1, "REGBANK_B:");
    for (unsigned int i = 16; i<32; i++)
      written += snprintf(buffer + written, buffer_size - written - 1, "%02x", sfr->get(i));
    written += snprintf(buffer + written, buffer_size - written - 1, "\n");
  }

  written += snprintf(buffer + written, buffer_size - written - 1, "PC:%x\n", PC);
  written += snprintf(buffer + written, buffer_size - written - 1, "SP:%x\n", sfr->get(INTERNAL_SP));
  written += snprintf(buffer + written, buffer_size - written - 1, "FLAG_Z:%x\n", FLAGS_GET_Z ? 1:0);
  written += snprintf(buffer + written, buffer_size - written - 1, "FLAG_C:%x\n", FLAGS_GET_C ? 1:0);
  written += snprintf(buffer + written, buffer_size - written - 1, "FLAG_I:%x\n", FLAGS_GET_I ? 1:0);

  if (file_name == NULL) {
    con->dd_printf("%s", buffer);
  }
  else {
    FILE *f = fopen(file_name, "w");
    if (f == NULL) {
      con->dd_printf("Cannot create file '%s'", file_name);
      return;
    }

    fprintf(f, "%s", buffer);
    fclose(f);
  }*/
}

void
cl_pblaze::load_state(class cl_console_base *con, char *file_name)
{
  TiXmlDocument doc( file_name );
  if (!doc.LoadFile()) {
    con->dd_printf("Cannot load file.");
    return;
  }

  TiXmlElement *root = doc.FirstChildElement("picoblaze");
  TiXmlElement *element;

  // TODO lover/upper case
  if (strcmp(root->Attribute("type"), id_string()) != 0) {
    con->dd_printf("Error: types of Picoblaze don't match current configuration.");
    return;
  }

  // IMPORT RAM
  element = root->FirstChildElement("ram");
  if ((unsigned long int)strtol(element->Attribute("size"), NULL, 0) != ram_size) {
    con->dd_printf("Error: RAM size doesn't match current configuration.");
    return;
  }
  else {
    int size = strtol(element->Attribute("bytes"), NULL, 0) * 2; // chars ( bytes * 2 = chars for one value) per one memory cell
    const char * text = element->GetText();

    // format string for sscanf - depends on bytes per memory cell
    char format[5];
    sprintf(format,"%%%ds", size);

    for (unsigned int i=0; i<ram_size; i++) {
      char cell[size+1];
      sscanf(text + i*size, format, cell);
      ram->set(i, strtol(cell, NULL, 16));
    }
  }
  // IMPORT STACK
  element = root->FirstChildElement("stack");
  if ((unsigned long int)strtol(element->Attribute("size"), NULL, 0) != stack_size) {
    con->dd_printf("Error: Stack size doesn't match current configuration.");
    return;
  }
  else {
    int size = strtol(element->Attribute("bytes"), NULL, 0) * 2; // chars ( bytes * 2 = chars for one value) per one memory cell
    const char * text = element->GetText();

    // format string for sscanf - depends on bytes per memory cell
    char format[5];
    sprintf(format,"%%%ds", size);

    for (unsigned int i=0; i<stack_size; i++) {
      char cell[size+1];
      sscanf(text + i*size, format, cell);
      stack->set(i, strtol(cell, NULL, 16));
    }
  }

  TiXmlElement *registers = root->FirstChildElement("registers");

  // active regbank
	if (type->type == CPU_PBLAZE_6) {
    const char *ar = registers->Attribute("activeRegbank"); //, active_regbank == REGISTER_BANK_A ? "A" : "B");
    if (strcmp(ar, "A") == 0) { // TODO lower/upper case
      active_regbank = REGISTER_BANK_A;
    }
    else if (strcmp(ar, "B") == 0) {
      active_regbank = REGISTER_BANK_B;
    }
  }

  // register bank
  element = registers->FirstChildElement("regbank");
  {
    int addr = S0;
    if (type->type == CPU_PBLAZE_6) {
      const char *n = element->Attribute("name");
      if (strcmp(n, "B") == 0) {
        addr = S0b;
      }
    }
    const char * text = element->GetText();
    for (int i=0; i<16; i++) {
      char cell[3];
      sscanf(text + i*2, "%2s", cell);
      sfr->set(addr+i, strtol(cell, NULL, 16));
    }
  }

  // secong register bank (only for kcpsm6)
  if (type->type == CPU_PBLAZE_6) {
    int addr = S0b;
    element = element->NextSiblingElement("regbank");
    const char *n = element->Attribute("name");
    if (strcmp(n, "A") == 0) {
      addr = S0;
    }
    const char *text = element->GetText();
    for (int i=0; i<16; i++) {
      char cell[3];
      sscanf(text + i*2, "%2s", cell);
      sfr->set(addr+i, strtol(cell, NULL, 16));
    }
  }

  // PC
  element = registers->FirstChildElement("pc");
  PC = strtol(element->GetText(), NULL, 16);

  // SP
  element = registers->FirstChildElement("sp");
  sfr->set(SP, strtol(element->GetText(), NULL, 16));

  TiXmlElement *flags = registers->FirstChildElement("flags");

  element = flags->FirstChildElement("carry");
  FLAGS_SET_C(strtol(element->GetText(), NULL, 0));

  element = flags->FirstChildElement("zero");
  FLAGS_SET_Z(strtol(element->GetText(), NULL, 0));

  element = flags->FirstChildElement("interruptEnable");
  FLAGS_SET_I(strtol(element->GetText(), NULL, 0));
}




/*
 * Execution
 */


int
cl_pblaze::exec_inst(void)
{
  t_mem code;

  if (fetch(&code))
    return(resBREAKPOINT);

  tick(1);

  int i = 0;
  dis_entry_pblaze * disass_pblaze = (type->type == CPU_PBLAZE_3 ? disass_pblaze3 : disass_pblaze6);
  while (((code & disass_pblaze[i].mask) != disass_pblaze[i].code) && disass_pblaze[i].instruction != BAD_OPCODE) {
    i++;
  }

  dis_entry_pblaze instruction = disass_pblaze[i];

    switch (instruction.instruction) {
        case ADD: return (inst_add(code, instruction.operands));
        case ADDCY: return (inst_addcy(code, instruction.operands));
        case AND: return (inst_and(code, instruction.operands));
        case CALL:
        case CALL_AT:
        case CALL_C:
        case CALL_Z:
        case CALL_NC:
        case CALL_NZ:
          return (inst_call(code, instruction.instruction, instruction.operands));
        case COMPARE:
          return (inst_compare(code, instruction.operands));
        case COMPARECY:
          return (inst_comparecy(code, instruction.operands));
        case DISABLE_INTERRUPT:
          return (inst_interrupt(false));
        case ENABLE_INTERRUPT:
          return (inst_interrupt(true));
        case FETCH:
          return (inst_fetch(code, instruction.operands));
        case HWBUILD:
          return (inst_hwbuild(code, instruction.operands));
        case IINPUT:
          return (inst_input(code, instruction.operands));
        case JUMP:
        case JUMP_AT:
        case JUMP_C:
        case JUMP_NC:
        case JUMP_NZ:
        case JUMP_Z:
          return (inst_jump(code, instruction.instruction, instruction.operands));
        case LOAD:
          return (inst_load(code, instruction.operands));
        case LOAD_RETURN:
          return (inst_load_return(code, instruction.operands));
        case OR:
          return (inst_or(code, instruction.operands));
        case OUTPUT:
          return (inst_output(code, instruction.operands));
        case OUTPUTK:
          return (inst_outputk(code, instruction.operands));
        case REGBANK_A:
          return (inst_regbank(REGISTER_BANK_A));
        case REGBANK_B:
          return (inst_regbank(REGISTER_BANK_B));
        case RETURN:
        case RETURN_C:
        case RETURN_NC:
        case RETURN_NZ:
        case RETURN_Z:
          return (inst_return(code, instruction.instruction));
        case RETURNI_DISABLE:
          return (inst_returni(false));
        case RETURNI_ENABLE:
          return (inst_returni(true));
        case RL:
          return (inst_rl(code, instruction.operands));
        case RR:
          return (inst_rr(code, instruction.operands));
        case SL0:
        case SL1:
        case SLA:
        case SLX:
          return (inst_sl(code, instruction.instruction));
        case SR0:
        case SR1:
        case SRA:
        case SRX:
          return (inst_sr(code, instruction.instruction));
        case STAR:
          return (inst_star(code, instruction.operands));
        case STORE:
          return (inst_store(code, instruction.operands));
        case SUB:
          return (inst_sub(code, instruction.operands));
        case SUBCY:
          return (inst_subcy(code, instruction.operands));
        case TEST:
          return (inst_test(code, instruction.operands));
        case TESTCY:
          return (inst_testcy(code, instruction.operands));
        case XOR:
          return (inst_xor(code, instruction.operands));

        case BAD_OPCODE:
        default:
            break;
    }

  // this shouldn't be executed. If so, something bad happened in simulated program
  return(resINV_INST);
}

/*
 * Checking for interrupt requests and accept one if needed
 */

int
cl_pblaze::do_interrupt(void)
{
  // is interrupt enabled?
  if (!FLAGS_GET_I)
    return(resGO);

  // check in stored interrupts, if at actual cycle irq is set
  if (find(stored_interrupts.begin(), stored_interrupts.end(), ticks->get_ticks() / clock_per_cycle()) != stored_interrupts.end())
    interrupt->interrupt_request = true;

  if (interrupt->interrupt_request) {
    printf("%g sec (%ld clks): Accepting interrupt, PC= 0x%06x\n",
	   ticks->get_rtime(), (long int)(ticks->get_ticks()), AU(PC));

    tick(1);

    //stack_push(PC);
    interrupt->preserved_flag_c = FLAGS_GET_C;
    interrupt->preserved_flag_z = FLAGS_GET_Z;
    FLAGS_SET_I(0);
    interrupt->interrupt_request = false;


    int res = resERROR;

    if (type->type == CPU_PBLAZE_3)
      res = inst_call(0x30000 /* isntruction call */ | (interrupt_vector & 0x003ff) /* masked interrupt vector */, CALL, ADDRESS10);
    else if (type->type == CPU_PBLAZE_6)
      res = inst_call(0x20000 /* isntruction call */ | (interrupt_vector & 0x00fff) /* masked interrupt vector */, CALL, ADDRESS12);


    if (res != resGO)
      return(res);
    else
      return(resINTERRUPT);
  }

  return(resGO);
}


/*
 * Converting bit address into real memory
 */

class cl_address_space *
cl_pblaze::bit2mem(t_addr bitaddr, t_addr *memaddr, t_mem *bitmask)
{
  // bits can be set only for flags C, Z and I simulated in FLAGS (address) register in SFR

  // address in SFR indicates higher byte of bitaddr
  if (memaddr)
    *memaddr = bitaddr & 0xf0;

  // bit position indicates lower byte in bitaddr
  if (bitmask)
    *bitmask= 1 << (bitaddr & 0xf);

  return sfr;
}




/* ******
 * Redefined functions from general uc
 * ******/


static long
ReadInt(FILE *f, bool *ok, int bytes)
{
  char s2[3];
  long l= 0;

  *ok= false;
  while (bytes)
    {
      if (fscanf(f, "%2c", &s2[0]) == EOF)
        return(0);
      s2[2]= '\0';
      l= l*256 + strtol(s2, NULL, 16);
      bytes--;
    }
  *ok= true;
  return(l);
}

static long
ReadPblazeRomHex(FILE *f, bool *ok)
{
  char rec[6];
  long l= 0;

  *ok= false;

  // read 5 character representim one record (one instruction) in rom
  if (fscanf(f, "%5c", rec) == EOF)
    return(0);

  rec[5]= '\0';
  l= strtol(rec, NULL, 16);

  *ok= true;
  return(l);
}

long
cl_pblaze::read_hex_file(const char *nam)
{
  cl_option * opt = sim->app->options->get_option("pblaze_hex");
  bool value = false;
  if (opt != NULL) {
    opt->get_value(&value);
  }

  long ret;
  if (value)
    ret = pblaze_read_hex_file(nam);
  else
    ret = std_read_hex_file(nam);

  analyze_init();
  return ret;
}


long
cl_pblaze::pblaze_read_hex_file(const char *nam)
{
  FILE *f;
  uint  addr= 0;  // address
  long written= 0, record = 0;
  bool ok = true;

  if (!rom)
    {
      sim->app->get_commander()->
        dd_printf("No ROM address space to read in.\n");
      return(-1);
    }

  if (!nam)
    {
      sim->app->get_commander()->
        dd_printf("cl_uc::read_hex_file File name not specified\n");
      return(-1);
    }
  else
    if ((f= fopen(nam, "r")) == NULL)
      {
        fprintf(stderr, "Can't open `%s': %s\n", nam, strerror(errno));
        return(-1);
      }

  while (ok)
  {
    record = ReadPblazeRomHex(f, &ok);

    if (ok) {
      rom->set(addr, record);
      addr++;
      written++;

      // skip unnecessary characters (white space etc)
      if (fscanf(f, "%*[ \n\r\t]"))
        {} // if statement only for prevent warning during compilation
    }
  }

  // if read wasn't ok and not at the and of file, read error has occurred
  if (!ok && getc(f) != EOF)
  {
      application->debug("Read error in record %ld.\n", written);
  }

  if (nam)
    fclose(f);
  application->debug("%ld records have been read\n", written);
  return(written);
}


long
cl_pblaze::std_read_hex_file(const char *nam)
{
  FILE *f;
  int c;
  long written= 0, recnum= 0;

  uchar dnum;     // data number
  uchar rtyp=0;   // record type
  uint  addr= 0;  // address
  uchar rec[300]; // data record
  uchar sum ;     // checksum
  uchar chk ;     // check
  int  i;
  bool ok, get_low= 1, get_middle = 0;
  uchar low= 0, middle=0, high=0;

  if (!rom)
    {
      sim->app->get_commander()->
        dd_printf("No ROM address space to read in.\n");
      return(-1);
    }

  if (!nam)
    {
      sim->app->get_commander()->
        dd_printf("cl_uc::read_hex_file File name not specified\n");
      return(-1);
    }
  else
    if ((f= fopen(nam, "r")) == NULL)
      {
        fprintf(stderr, "Can't open `%s': %s\n", nam, strerror(errno));
        return(-1);
      }

  //memset(inst_map, '\0', sizeof(inst_map));
  ok= true;
  while (ok &&
         rtyp != 1)
    {
      while (((c= getc(f)) != ':') &&
             (c != EOF)) ;
      if (c != ':')
        {fprintf(stderr, ": not found\n");break;}
      recnum++;
      dnum= ReadInt(f, &ok, 1);//printf("dnum=%02x",dnum);
      chk = dnum;
      addr= ReadInt(f, &ok, 2);//printf("addr=%04x",addr);
      chk+= (addr & 0xff);
      chk+= ((addr >> 8) & 0xff);
      rtyp= ReadInt(f, &ok, 1);//printf("rtyp=%02x ",rtyp);
      chk+= rtyp;
      for (i= 0; ok && (i < dnum); i++)
        {
          rec[i]= ReadInt(f, &ok, 1);//printf("%02x",rec[i]);
          chk+= rec[i];
        }
      if (ok)
        {
          sum= ReadInt(f, &ok, 1);//printf(" sum=%02x\n",sum);
          if (ok)
            {
              if (((sum + chk) & 0xff) == 0)
                {
                  if (rtyp == 0)
                    {
                      if (rom->width > 8)
                        addr/= 2;
                      for (i= 0; i < dnum; i++)
                        {
                          if (rom->width <= 8)
                            {
                              rom->set(addr, rec[i]);
                              addr++;
                              written++;
                            }
                          else if (rom->width <= 16)
                            {
                              if (get_low)
                                {
                                  low= rec[i];
                                  get_low= 0;
                                }
                              else
                                {
                                  high= rec[i];
                                  rom->set(addr, (high*256)+low);
                                  addr++;
                                  written++;
                                  get_low= 1;
                                }
                            }
                          else if (rom->width <= 24)
                            {
                              if (get_low)
                                {
                                  low= rec[i];
                                  get_low= 0;
                                  get_middle = 1;
                                }
                              else if (get_middle)
                                {
                                    middle = rec[i];
                                    get_middle = 0;
                                }
                              else
                                {
                                  high= rec[i];
                                  rom->set(addr, (high*256*256)+(middle*256)+low);
                                  addr++;
                                  written++;
                                  get_low= 1;
                                }
                            }
                        }
                    }
                  else
                    if (rtyp != 1)
                      application->debug("Unknown record type %d(0x%x)\n",
                                         rtyp, rtyp);
                }
              else
                application->debug("Checksum error (%x instead of %x) in "
                                   "record %ld.\n", chk, sum, recnum);
            }
          else
            application->debug("Read error in record %ld.\n", recnum);
        }
    }
  if (rom->width > 8 &&
      !get_low)
    rom->set(addr, low);

  if (nam)
    fclose(f);
  application->debug("%ld records have been read\n", recnum);
  return(written);
}


void
cl_pblaze::read_interrupt_file(void)
{
  cl_option * opt = sim->app->options->get_option("pblaze_interrupt_file");
  if (opt != NULL) {
    char *file = 0;
    opt->get_value(&file);

    TiXmlDocument doc(file);

    if (!doc.LoadFile()) {
      fprintf(stderr, "Can't open file with interrupts `%s',=.\n", file);
      return;
    }

    TiXmlElement *root = doc.FirstChildElement("interrupts");
    TiXmlElement *element;

    for(element = root->FirstChildElement("interrupt"); element; element = element->NextSiblingElement()) {
      long interrupt_tick = strtol(element->GetText(), NULL, 0);
      stored_interrupts.push_back(interrupt_tick);
    }

    /*
    char *file = 0;
    opt->get_value(&file);

    FILE *f;
    if ((f = fopen(file, "r")) == NULL)
    {
      fprintf(stderr, "Can't open file with interrupts `%s': %s\n", file, strerror(errno));
    }
    else {
      while (true)
      {
        int i;
        int res = fscanf(f, "%d", &i);
        if (res == EOF)
          break;
        else if (res == 0) {
          fprintf(stderr, "Error during reading file with interrupts `%s'\n", file);
          break;
        }

        stored_interrupts.insert(i);

        // skip white characters
        res = fscanf(f, "*[ \n\r\t]");
      }
      fclose(f);
    } */
  }
}

void
cl_pblaze::read_input_file(void)
{
  cl_option * opt = sim->app->options->get_option("pblaze_input_file");
  if (opt != NULL) {
    char *file = 0;
    opt->get_value(&file);

    TiXmlDocument doc(file);

    if (!doc.LoadFile()) {
      fprintf(stderr, "Can't open file with inputs `%s'.\n", file);
      return;
    }

    TiXmlElement *root = doc.FirstChildElement("inputs");
    TiXmlElement *element;

    for(element = root->FirstChildElement("input"); element; element = element->NextSiblingElement()) {

      const char *at_port = element->Attribute("port");
      int in_port = -1;
      if (at_port != NULL) {
        in_port = strtol(at_port, NULL, 0);
      }

      const char *at_tick = element->Attribute("tick");
      long in_tick = -1;
      if (at_tick != NULL) {
        in_tick = strtol(at_tick, NULL, 0);
      }

      const char *at_type = element->Attribute("type") != NULL ? element->Attribute("type") : "hex"; // default input type

      const char *input = element->GetText();

      if (strcmp(at_type, "int") == 0) {
        t_mem value = strtol(input, NULL, 10);

        // set input
        input_port->add_input(value, in_port, in_tick);
      }
      else if (strcmp(at_type, "hex") == 0) {
        int start = 0;
        int length = strlen(input);

        while (start < length) {
          t_mem value;
          sscanf(input + start, "%2x", &value);

          // if tick is specified, only first value is loaded
          if (in_tick != -1)
            break;

          start += 2;

          // set input
          input_port->add_input(value, in_port, in_tick);
        }
      }
      else if (strcmp(at_type, "char") == 0) {
        int start = 0;
        int length = strlen(input);

        while (start < length) {
          char value;
          sscanf(input + start, "%1c", &value);

          // if tick is specified, only first value is loaded
          if (in_tick != -1)
            break;

          start++;

          // set input
          input_port->add_input(value, in_port, in_tick);
        }
      }
    }
  }

  // DEBUG print inputs
  for(input_port_map::iterator it = input_port->inputs.begin(); it != input_port->inputs.end(); it++) {
    application->debug("port: %d\n", it->first);

    for(input_tick_map::iterator it_inner = it->second.begin(); it_inner != it->second.end(); it_inner++) {
      application->debug("\ttick: %ld\n", it_inner->first);

      for(list<t_mem>::iterator it_list = it_inner->second.begin(); it_list != it_inner->second.end(); it_list++) {
        application->debug("\t\t: %d\n", *it_list);
      }
    }
  }

}



void
cl_pblaze::init_uc_parameters(void)
{
  // default values
  active_regbank = REGISTER_BANK_A;
  rom_size = 1024;
  ram_size = 64;
  interrupt_vector = 0x3ff;

  if (type->type == CPU_PBLAZE_3)
    stack_size = 31;
  else if (type->type == CPU_PBLAZE_6)
    stack_size = 30;

  // ram size
  cl_option * opt = sim->app->options->get_option("pblaze_ram_size");
  if (opt != NULL) {
    if (type->type == CPU_PBLAZE_6) {
      long size = -1;
      opt->get_value(&size);

      if (size <= 0 || (size != 64 && size != 128 && size != 256))
      {
        fprintf(stderr, "Invalid RAM size. Valid values are 64 (default), 128 and 256.\n");
      }
      else
      {
        ram_size = (unsigned int) size;
      }
    }
    else
    {
      fprintf(stderr, "RAM size can be specified only for KCPSM6. Using default size 64b.\n");
    }
  }

  // rom size
  opt = sim->app->options->get_option("pblaze_rom_size");
  if (opt != NULL) {
    if (type->type == CPU_PBLAZE_6) {
      long size = -1;
      opt->get_value(&size);

      if (size <= 0)
      {
        fprintf(stderr, "Invalid ROM size.\n");
      }
      else
      {
        rom_size = (unsigned int) size;
      }
    }
    else
    {
      fprintf(stderr, "ROM size can be specified only for KCPSM6.\n");
    }
  }

  // interrupt vector
  opt = sim->app->options->get_option("pblaze_interrupt_vector");
  if (opt != NULL) {
    if (type->type == CPU_PBLAZE_6) {
      long iv = -1;
      opt->get_value(&iv);

      if (iv <= 0 || (unsigned int) iv >= rom_size)
      {
        fprintf(stderr, "Invalid interrupt vector. %ld\n", iv);
      }
      else
      {
        interrupt_vector = (unsigned int) iv;
      }
    }
    else
    {
      fprintf(stderr, "Interrupt vector can be specified only for KCPSM6.\n");
    }
  }

  // built-in hardware constant
  opt = sim->app->options->get_option("pblaze_hw_const");
  if (opt != NULL) {
    if (type->type == CPU_PBLAZE_6) {
      long value = -1;
      opt->get_value(&value);

      if (value <= 0 || value >= 256)
      {
        fprintf(stderr, "Invalid built-in hardware constant. %ld\n", value);
      }
      else
      {
        hw_constant = value;
      }
    }
    else
    {
      fprintf(stderr, "Built-in hardware constant can be specified only for KCPSM6.\n");
    }
  }
}

/* End of pblaze.src/pblaze.cc */
