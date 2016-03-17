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
// loader return code
#define LOADER_ISVALID			1
#define LOADER_NOERROR			0 
#define LOADER_ACCESSERROR		-1
#define LOADER_BADFILE			-2
#define LOADER_FILECORRUPT		-3 
#define LOADER_BADPARAMETER		-4
#define LOADER_INTERNALERROR	-5
#define LOADER_UNSUPPORTEDFILE	-6

typedef int (*ISVALIDDISKFILE) (void* floppycontext,char * imgfile);
typedef int (*LOADDISKFILE)(void* floppycontext,void * floppydisk,char * imgfile,void * parameters);
typedef int (*WRITEDISKFILE)(void* floppycontext,void * floppydisk,char * imgfile,void * parameters);
typedef int (*GETPLUGININFOS)(void* floppycontext,void * floppydisk,void * pluginsinfos);

typedef struct plugins_ptr_
{
	ISVALIDDISKFILE libIsValidDiskFile;
	LOADDISKFILE	libLoad_DiskFile;
	WRITEDISKFILE	libWrite_DiskFile;
	GETPLUGININFOS	libGetPluginInfos;
}plugins_ptr;


int floppy_load(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * newfloppy,char* imgname);
int floppy_unload(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk);

