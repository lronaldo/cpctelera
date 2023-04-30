/*
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011 Jean-François DEL NERO
//
// This file is part of HxCFloppyEmulator.
//
// HxCFloppyEmulator may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// HxCFloppyEmulator is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// HxCFloppyEmulator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HxCFloppyEmulator; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/
#define SRAMSIZE 8*1024

#define FT_PURGE_RX         1
#define FT_PURGE_TX         2
#define FT_EVENT_RXCHAR		    1

int ftdi_load_lib (HXCFLOPPYEMULATOR* floppycontext);

int open_ftdichip(unsigned long * ftdihandle);
int close_ftdichip(unsigned long ftdihandle);
int purge_ftdichip(unsigned long ftdihandle,unsigned long buffer);
int setusbparameters_ftdichip(unsigned long ftdihandle,unsigned long buffersizetx,unsigned long buffersizerx);
int setlatencytimer_ftdichip(unsigned long ftdihandle,unsigned char latencytimer_ms);
int write_ftdichip(unsigned long ftdihandle,unsigned char * buffer,unsigned int size);
int read_ftdichip(unsigned long ftdihandle,unsigned char * buffer,unsigned int size);
int getfifostatus_ftdichip(unsigned long ftdihandle,unsigned long * txlevel,unsigned long *rxlevel,unsigned long * event);
int seteventnotification_ftdichip(unsigned long ftdihandle,unsigned long eventmask,void * event);



