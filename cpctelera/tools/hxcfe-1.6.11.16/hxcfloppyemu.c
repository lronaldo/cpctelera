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
#include "./usb_floppyemulator/usb_hxcfloppyemulator.h"

#include "mfm_file_writer.h"

//#include "win32_api.h"



HXCFLOPPYEMULATOR * flopemu;
HWINTERFACE * hwif;

#define NB_ENABLE 1
#define NB_DISABLE 0

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


int kbhit()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

void nonblock(int state)
{
 struct termios ttystate;

  //get the terminal state
  tcgetattr(STDIN_FILENO, &ttystate);

  if (state==NB_ENABLE)
  {
    //turn off canonical mode
    ttystate.c_lflag &= ~ICANON;
    //minimum of number input read.
    ttystate.c_cc[VMIN] = 1;
  }
  else if (state==NB_DISABLE)
  {
    //turn on canonical mode
    ttystate.c_lflag |= ICANON;
  }
  //set the terminal attributes.
  tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}


int main(int argc, char* argv[])
{
	FLOPPY * thefloppydisk;
	int ret;
	unsigned char old_trackpos,trackpos;
	int packetsize;
	int select_line;
	int stop;
	char c;
	
	
	printf("HxC Floppy Emulator : USB Floppy image file loader\n");
	printf("Copyright (C) 2006-2011 Jean-Francois DEL NERO\n");
    printf("This program comes with ABSOLUTELY NO WARRANTY\n");
    printf("This is free software, and you are welcome to redistribute it\n");
    printf("under certain conditions;\n\n");
	
	flopemu=(HXCFLOPPYEMULATOR*)malloc(sizeof(HXCFLOPPYEMULATOR));	
	flopemu->hxc_printf=&CUI_affiche;
	
	initHxCFloppyEmulator(flopemu);
	
	if(argv[1])
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
			//write_MFM_file(flopemu,thefloppydisk,"d:\\test.mfm");
			struct sched_param p;
			
			p.sched_priority = sched_get_priority_max(SCHED_FIFO);
			if(sched_setscheduler(0,SCHED_FIFO,&p)==-1)
			{
				printf("Cannot change the process priority!!!\n");
			}	
			
			hwif=(HWINTERFACE *)malloc(sizeof(HWINTERFACE));
			memset(hwif,0,sizeof(HWINTERFACE));
			
			stop=0;
			packetsize=3072;
			select_line=0;  // DS0
			if(!(HW_CPLDFloppyEmulator_init(flopemu,hwif)<0))
			{
				
				nonblock(NB_ENABLE);
				
				InjectFloppyImg(flopemu,thefloppydisk,hwif);
				printf("\nControl keys : 0,1,2,3=Change select line +,-=change usb packet size q=quit\n");
				do
				{
					hwif->usbstats.packetsize=packetsize;
					hwif->drive_select_source=select_line;
					trackpos=hwif->current_track;
					if(trackpos!=old_trackpos)
					{
						printf("Track %.2d \n",trackpos);
						old_trackpos=trackpos;
					}
					if(kbhit())
					{
						c=fgetc(stdin);
						switch(c)
						{
						case '0':
							select_line=0;
							printf("Switching to DS0.\n");
							break;
						case '1':
							select_line=1;
							printf("Switching to DS1.\n");
							break;
						case '2':
							select_line=2;
							printf("Switching to DS2.\n");
							break;
						case '3':
							select_line=3;
							printf("Switching to DS3.\n");
							break;
							
						case '+':
							if(packetsize<4096) packetsize=packetsize+128;
							printf("Change USB packet size : %d\n",packetsize);
							break;
							
						case '-':
							if(packetsize>256) packetsize=packetsize-128;				
							printf("Change USB packet size : %d\n",packetsize);
							break;
							
						case 'q':
							stop=1;
							break;
							
						default:
							break;
							
						}
						
					}
	
					usleep(10000);//hxc_pause(10);

				}while(!stop);
								
				nonblock(NB_DISABLE);
			}
		}
	}
	else
	{
		printf("Syntax: %s [image-file]\n",argv[0]);
	}
	
	
	free(flopemu);
	return 0;
}


