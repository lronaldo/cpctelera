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



	typedef struct cfgrawfile_
	{
		unsigned char sectorpertrack;
		unsigned char sectorsize;
		unsigned long numberoftrack;
		unsigned char tracktype;
		unsigned char sidecfg;	
		unsigned long gap3;
		unsigned long rpm;
		unsigned long bitrate;
		unsigned char interleave;
		unsigned char firstidsector;
		unsigned char skew;
		unsigned char autogap3;
		unsigned char fillvalue;
		unsigned char intersidesectornumbering;
		unsigned char sideskew;
	}cfgrawfile;

	enum
	{
		FM_TRACK_TYPE,
		FMIBM_TRACK_TYPE,
		MFM_TRACK_TYPE,
		MFMIBM_TRACK_TYPE,
		GCR_TRACK_TYPE
	};

	typedef struct track_type_
	{
		int id;
		char * name;

	}track_type;

	typedef struct sectorsize_type_
	{
		int id;
		char * name;

	}sectorsize_type;


	
	#define TWOSIDESFLOPPY 0x02
	#define SIDE_INVERTED 0x04
	#define SIDE0_FIRST 0x08



int RAW_libIsValidFormat(HXCFLOPPYEMULATOR* floppycontext,cfgrawfile * imgformatcfg);
int RAW_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,cfgrawfile * imgformatcfg);



