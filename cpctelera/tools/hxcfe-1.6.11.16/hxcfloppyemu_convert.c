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
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h> 
#include <time.h>

#include "hxc_floppy_emulator.h"
#include "internal_floppy.h"
#include "floppy_loader.h"

#include "mfm_file_writer.h"
#include "afi_file_writer.h"
#include "hfe_file_writer.h"
#include "raw_file_writer.h"
#include "cpcdsk_file_writer.h"



//#include "win32_api.h"



HXCFLOPPYEMULATOR * flopemu;

#define NB_ENABLE 1
#define NB_DISABLE 0


enum
{
    TYPE_HFE = 1,
    TYPE_AFI,
    TYPE_CPCDSK,
    TYPE_RAW,
    TYPE_MFM,
};



int CUI_affiche(int MSGTYPE,char * chaine, ...)
{
	//if(MSGTYPE!=MSG_DEBUG)
	{
		va_list marker;
		va_start( marker, chaine );     
		
		vprintf(chaine,marker);
		printf("\n");

		va_end( marker );
	}
    return 0;
}

int gettype(char * str_type)
{
	
	if(!strcmp(str_type,"-HFE"))
	{
		return TYPE_HFE;	
	}

	if(!strcmp(str_type,"-AFI"))
	{
		return TYPE_AFI;	
	}

	if(!strcmp(str_type,"-CPCDSK"))
	{
		return TYPE_CPCDSK;	
	}

	if(!strcmp(str_type,"-RAW"))
	{
		return TYPE_RAW;	
	}

	if(!strcmp(str_type,"-MFM"))
	{
		return TYPE_MFM;
	}

	return 0;
}



void get_filename(char * path,char * filename)
{
int i,done;
	i=strlen(path);
	done=0;
	while(i && !done)
	{
		i--;
		
		if(path[i]=='/')
		{
			done=1;
			i++;
		}
		

	}

	sprintf(filename,"%s",&path[i]);

	i=0;
	while(filename[i])
	{
		if(filename[i]=='.')
		{
			filename[i]='_';
		}

	i++;
	}
	
	return;
}

int main(int argc, char* argv[])
{
	FLOPPY * thefloppydisk;
	int ret;
	unsigned char old_trackpos,trackpos;
	int packetsize;
	int select_line;
	int stop,output_file_type;
	char c;
	char filename[512];


	printf("HxC Floppy Emulator : Floppy image file converter\n");
	printf("Copyright (C) 2006-2011 Jean-Francois DEL NERO\n");
    	printf("This program comes with ABSOLUTELY NO WARRANTY\n");
    	printf("This is free software, and you are welcome to redistribute it\n");
    	printf("under certain conditions;\n\n");


	flopemu=(HXCFLOPPYEMULATOR*)malloc(sizeof(HXCFLOPPYEMULATOR));	
	flopemu->hxc_printf=&CUI_affiche;
	
	initHxCFloppyEmulator(flopemu);

		
	if(argv[1] && argv[2])
	{

		output_file_type=gettype(argv[2]);
			
		if(output_file_type)
		{
			thefloppydisk=(FLOPPY *)malloc(sizeof(FLOPPY));
			memset(thefloppydisk,0,sizeof(FLOPPY));

			ret=floppy_load(flopemu,thefloppydisk,argv[1]);


			if(ret!=LOADER_NOERROR)
			{
				switch(ret)
				{
					case LOADER_UNSUPPORTEDFILE:
						printf("Load error!: Image file not yet supported!\n");
					break;
					case LOADER_FILECORRUPT:
						printf("Load error!: File corrupted ? Read error ?\n");
					break;
					case LOADER_ACCESSERROR:
						printf("Load error!:  Read file error!\n");
					break;
					default:
						printf("Load error! error %d\n",ret);
					break;
				}
				free(thefloppydisk);
			}
			else
			{

				get_filename(argv[1],filename);

				switch(output_file_type)
				{
					case TYPE_HFE:
						strcat(filename,".hfe");
						write_HFE_file(flopemu,thefloppydisk,filename,-1,0);
					break;
					case TYPE_AFI:
						strcat(filename,".afi");
						write_AFI_file(flopemu,thefloppydisk,filename);
					break;
					case TYPE_CPCDSK:
						strcat(filename,".dsk");
						write_CPCDSK_file(flopemu,thefloppydisk,filename);
					break;
					case TYPE_RAW:
						strcat(filename,".img");
						write_RAW_file(flopemu,thefloppydisk,filename);
					break;
					case TYPE_MFM:
						strcat(filename,".mfm");
						write_MFM_file(flopemu,thefloppydisk,filename);
					break;
				}

				//
  
			
			}
		}
		else
		{
			printf("Syntax: %s [image-file] [-HFE/-AFI/-CPCDSK/-RAW/-MFM]\n",argv[0]);
		}
	}
	else
	{
		printf("Syntax: %s [image-file] [-HFE/-AFI/-CPCDSK/-RAW/-MFM]\n",argv[0]);
	}


	free(flopemu);
	return 0;
}


