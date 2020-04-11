/*-------------------------------------------------------------------------

  device.c - Accomodates subtle variations in PIC16 devices

   Written By -  Scott Dattalo scott@dattalo.com

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
-------------------------------------------------------------------------*/

/*
  PIC device abstraction

  There are dozens of variations of PIC microcontrollers. This include
  file attempts to abstract those differences so that SDCC can easily
  deal with them.
*/

#ifndef  __DEVICE_H__
#define  __DEVICE_H__

#define CONFIGURATION_WORDS     20
#define IDLOCATION_BYTES        20

typedef struct {
  unsigned int mask;
  int emit;
  unsigned int value;
  unsigned int andmask;
} configRegInfo_t;

typedef struct {
  int confAddrStart;      /* starting address */
  int confAddrEnd;        /* ending address */
  configRegInfo_t crInfo[ CONFIGURATION_WORDS ];
} configWordsInfo_t;

typedef struct {
  unsigned char emit;
  unsigned char value;
} idRegInfo_t;

typedef struct {
  int idAddrStart;        /* starting ID address */
  int idAddrEnd;          /* ending ID address */
  idRegInfo_t irInfo[ IDLOCATION_BYTES ];
} idBytesInfo_t;


#define PROCESSOR_NAMES    4
/* Processor unique attributes */
typedef struct PIC16_device {
  char *name[PROCESSOR_NAMES];  /* aliases for the processor name */
  /* RAMsize *must* be the first item to copy for 'using' */
  unsigned int RAMsize;         /* size of Data RAM - VR 031120 */
  unsigned int acsSplitOfs;     /* access bank split offset */
  configWordsInfo_t cwInfo;     /* configuration words info */
  idBytesInfo_t idInfo;         /* ID Locations info */
  int xinst;                    /* device supports XINST */
  /* next *must* be the first field NOT being copied via 'using' */
  struct PIC16_device *next;    /* linked list */
} PIC16_device;

extern PIC16_device *pic16;

/* Given a pointer to a register, this macro returns the bank that it is in */
#define REG_ADDR(r)        ((r)->isBitField ? (((r)->address)>>3) : (r)->address)

#define OF_LR_SUPPORT           0x00000001
#define OF_NO_OPTIMIZE_GOTO     0x00000002
#define OF_OPTIMIZE_CMP         0x00000004
#define OF_OPTIMIZE_DF          0x00000008

typedef struct {
  int no_banksel;
  int opt_banksel;
  int omit_configw;
  int omit_ivt;
  int leave_reset;
  int stack_model;
  int ivt_loc;
  int nodefaultlibs;
  int dumpcalltree;
  char *crt_name;
  int no_crt;
  int ip_stack;
  unsigned long opt_flags;
  int gstack;
  unsigned int debgen;
  int xinst;
  int no_warn_non_free;
} pic16_options_t;

extern pic16_options_t pic16_options;

#define STACK_MODEL_SMALL       (pic16_options.stack_model == 0)
#define STACK_MODEL_LARGE       (pic16_options.stack_model == 1)

extern set *fix_idataSymSet;
extern set *rel_idataSymSet;

#if 0
/* This is an experimental code for #pragma inline
   and is temporarily disabled for 2.5.0 release */
extern set *asmInlineMap;
#endif  /* 0 */

typedef struct {
  unsigned long isize;
  unsigned long adsize;
  unsigned long udsize;
  unsigned long idsize;
  unsigned long intsize;
} stats_t;

extern stats_t statistics;

typedef struct pic16_config_options_s {
  char *config_str;
  struct pic16_config_options_s *next;
} pic16_config_options_t;

extern pic16_config_options_t *pic16_config_options;

/****************************************/
const char *pic16_processor_base_name(void);
void pic16_assignConfigWordValue(int address, unsigned int value);
void pic16_assignIdByteValue(int address, char value);
int pic16_isREGinBank(reg_info *reg, int bank);
int pic16_REGallBanks(reg_info *reg);

int checkAddReg(set **set, reg_info *reg);
int checkAddSym(set **set, symbol *reg);
int checkSym(set *set, symbol *reg);

#endif  /* __DEVICE_H__ */
