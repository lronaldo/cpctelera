/*-------------------------------------------------------------------------

   device.c - Accomodates subtle variations in PIC devices
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

#include "common.h"

#define MAX_NUM_CONFIGS 16

/*
 * Imports
 */
extern char *iComments2;

/* memRange - a structure to define a range of valid memory addresses 
 * 
 * The Memory of most PICs (and other micros) is a collection of
 * disjoint chunks. The memRange structure will define the start
 * and end address of one of these chunks. The memory map of a
 * particular device is a collection of memRange struct's.
 */

typedef struct memRange {
	int start_address;      /* first address in range */
	int end_address;        /* last */
	int alias;              /* bit mask defining how/if memory range is aliased 
	                         * e.g. alias = 0x80 means start_address is identical
	                         * to the memory location at (0x80 | start_address) */
	int bank;               /* PIC memory bank this range occupies */
	struct memRange *next;	/* linked list */

} memRange;

/* Processor unique attributes */
typedef struct PIC_device {
	char *name;                 /* the processor name */

	memRange *ram;              /* RAM memory map */
	memRange *sfr;              /* SFR memory map */

	int maxRAMaddress;          /* maximum value for a data address */
	int defMaxRAMaddrs;         /* default maximum value for a data address */
	int bankMask;               /* Bitmask that is ANDed with address to extract banking bits */
	//  int hasAliasedRAM:1;        /* True if there are bank independent registers */
	int num_configs;                /* number of config words for this device */
	int config[MAX_NUM_CONFIGS];    /* addresses of config word(s) */
	
	int programMemSize;         /* program memory size in words - for device listing only */
	int dataMemSize;            /* data (RAM) memory size in bytes - for device listing only */
	int eepromMemSize;          /* EEPROM memory size in bytes - for device listing only */
	int ioPins;                 /* number of I/O pins - for device listing only */
        int isEnhancedCore;         /* enhanced cores (19f1934) feature automatic context saving */

} PIC_device;


PIC_device *init_pic(char *pic_type);
int picIsInitialized(void);
char *processor_base_name(void);
int IS_CONFIG_ADDRESS(int addr);
void pic14_assignConfigWordValue(int address, int value);
int pic14_emitConfigWord(FILE *vFile);

int pic14_allRAMShared(void);
int pic14_getSharedStack(int *low, int *high, int *size);
PIC_device * pic14_getPIC(void);

#endif  /* __DEVICE_H__ */
