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
typedef struct pcimgfileformats_t_
{
	unsigned int filesize;
	int numberoftrack;
	int sectorpertrack;
	int numberofside;
	int gap3len;
	int interleave;
	int RPM;
	int bitrate;
	int encoding;
	int interface_mode;
}pcimgfileformats_t;

static pcimgfileformats_t pcimgfileformats[]=
{
	{163840,40,8,1,84,1,300,250000,1,IBMPC_DD_FLOPPYMODE},
	{184320,40,9,1,84,1,300,250000,1,IBMPC_DD_FLOPPYMODE},
	{327680,40,8,2,84,1,300,250000,1,IBMPC_DD_FLOPPYMODE},
	{368640,40,9,2,84,1,300,250000,1,IBMPC_DD_FLOPPYMODE},
	
	{614400,80,15,1,84,1,300,500000,1,IBMPC_HD_FLOPPYMODE},
	{737280,80,9,2,84,1,300,250000,1,IBMPC_DD_FLOPPYMODE},
	{1228800,80,15,2,84,1,300,500000,1,IBMPC_HD_FLOPPYMODE},
	{1474560,80,18,2,84,1,300,500000,1,IBMPC_HD_FLOPPYMODE},
	{1720320,80,21,2,15,1,300,500000,1,IBMPC_HD_FLOPPYMODE},
	{2949120,80,36,2,84,1,300,1000000,1,IBMPC_ED_FLOPPYMODE},
	{0,0,0,0,0,0}
};
