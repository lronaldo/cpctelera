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
#include <string.h>
#include <stdlib.h>
#include "vfile.h"

HXCFILE * HXC_fopen(const char * filename, const char * mode)
{
	HXCFILE * handle;

	handle=(HXCFILE *)malloc(sizeof(HXCFILE));
	handle->buffer=(unsigned char*)malloc(4096);
	handle->true_buffersize=4096;
	handle->buffersize=0;
	handle->ptr=0;

	return handle;

}
int HXC_fclose(HXCFILE * handle)
{
	if(handle->buffer) free(handle->buffer);
	handle->buffersize=0;
	handle->ptr=0;
	free(handle);
return 0;
}

int HXC_fwrite(const void * inbuffer, int count, int len, HXCFILE * handle)
{
	int i;

	if(handle->buffersize+(len*count)>handle->true_buffersize)
	{
		do
		{
			handle->true_buffersize=handle->true_buffersize+4096;
		}while(handle->buffersize+(len*count)>handle->true_buffersize);
		handle->buffer=(unsigned char*)realloc(handle->buffer,handle->true_buffersize);
	}

	handle->buffersize=handle->buffersize+(len*count);
	
	for(i=0;i<count;i++)
	{
		memcpy(&handle->buffer[handle->ptr],inbuffer,len);
		handle->ptr=handle->ptr+len;
	}
	return len*count;
}
