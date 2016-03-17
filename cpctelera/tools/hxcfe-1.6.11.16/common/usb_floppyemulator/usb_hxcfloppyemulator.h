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
#define NEXTPACKET_EVENT 0x1
#define TRACKCHANGED_EVENT 0x2
#define INCOMINGPACKET_EVENT 0x4

#define STATUS_ERROR 0
#define STATUS_LOOKINGFOR 1
#define STATUS_ONLINE 2


typedef struct _USBStats
{
	unsigned long totaldataout;
	unsigned long dataout;
	
	unsigned long synclost;

	unsigned long packetsize;
	unsigned long totalpacketsent;
	unsigned long packetsent;
	
}USBStats;

typedef struct usbtrack_
{
	unsigned char * usbtrack;
	unsigned char * randomusbtrack;
	unsigned long tracklen;
}usbtrack;

typedef struct HWINTERFACE_
{
	USBStats usbstats;

	unsigned char number_of_track;
	usbtrack precalcusbtrack[256];

	unsigned char *randomlut;
	unsigned long hw_handle;
	
	unsigned char interface_mode;
	unsigned int  drive_select_source;
	unsigned char double_step;

	unsigned char current_track;

	//	unsigned char floppyloaded;
	unsigned char floppychanged;

	unsigned long trackbuffer_pos;

	unsigned char start_emulation;
	unsigned char stop_emulation;
	unsigned char running;

	unsigned char status; // 0 -> error  1->lookingfor 2->online


}HWINTERFACE;

int HW_CPLDFloppyEmulator_init(HXCFLOPPYEMULATOR* floppycontext,HWINTERFACE * hwif);
int HW_deinit(HXCFLOPPYEMULATOR* floppycontext);
int InjectFloppyImg(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,HWINTERFACE * hwif);


