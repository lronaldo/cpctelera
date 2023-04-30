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

//#define FTDILIB 1
//#define DEBUGMODE 1

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <pthread.h>
#include <sched.h>
#include <errno.h>
#include <sys/time.h>

#include "WinTypes.h"

#include "hxc_floppy_emulator.h"
#include "internal_floppy.h"
#include "floppy_loader.h"
#include "./usb_floppyemulator/usb_hxcfloppyemulator.h"
#include "linux_api.h"
#include "ftd2xx.h"

#ifdef FTDILIB
#include "libftdi-0.14/src/ftdi.h"
#else
#include "ftdi.h"
#endif


#ifdef FTDILIB
#define BUFFERSIZE 32*1024


EVENT_HANDLE * STOP_THREAD_EVENT;
EVENT_HANDLE * READ_THREAD_EVENT;
int stop_thread;
struct ftdi_context ftdic;


typedef struct _DATA_FIFO{
	unsigned long ptr_in;
	unsigned long ptr_out;
	unsigned char BUFFER[BUFFERSIZE];
} DATA_FIFO;

DATA_FIFO tx_fifo;
DATA_FIFO rx_fifo;

unsigned char RXBUFFER[64];

EVENT_HANDLE * createevent()
{
	EVENT_HANDLE* theevent;
	theevent=(EVENT_HANDLE*)malloc(sizeof(EVENT_HANDLE));
	pthread_mutex_init(&theevent->eMutex, NULL);
	pthread_cond_init(&theevent->eCondVar, NULL);
	return theevent;
}

void setevent(EVENT_HANDLE * theevent)
{
	pthread_cond_signal(&theevent->eCondVar);
}

void destroyevent(EVENT_HANDLE * theevent)
{
	pthread_cond_destroy(&theevent->eCondVar);
	free(theevent);
}

int waitevent(EVENT_HANDLE* theevent,int timeout)
{
	struct timeval now;
	struct timespec timeoutstr;
	int retcode;
	int ret;

	//if(timeout==0) timeout=INFINITE;
	//ret=WaitForSingleObject(eventtab[id],timeout);


	pthread_mutex_lock(&theevent->eMutex);
	gettimeofday(&now,0);
	timeoutstr.tv_sec = now.tv_sec + (timeout/1000);
	timeoutstr.tv_nsec = (now.tv_usec * 1000) ;//+(timeout*1000000);
	retcode = 0;

	retcode = pthread_cond_timedwait(&theevent->eCondVar, &theevent->eMutex, &timeoutstr);	
	if (retcode == ETIMEDOUT) 
	{
	  pthread_mutex_unlock(&theevent->eMutex);
      	  return 1;
	} 
	else 
	{
      	  pthread_mutex_unlock(&theevent->eMutex);
      	  return 0; 

	}
	
}


// --------------------------------------------------------

typedef int (*RDTHREADFUNCTION) (struct ftdi_context * ftdihandle);

typedef struct listenerthreadinit_
{
	RDTHREADFUNCTION thread;
	struct ftdi_context *ftdihandle;
	//HXCFLOPPYEMULATOR * hxcfloppyemulatorcontext;
	//HWINTERFACE * hwcontext;
}listenerthreadinit;

void * FTDIListenerThreadProc( void *lpParameter)
{
	listenerthreadinit *threadinitptr;
	RDTHREADFUNCTION thread;
	
	threadinitptr=(listenerthreadinit*)lpParameter;
	thread=threadinitptr->thread;

       	thread(threadinitptr->ftdihandle);

	return 0;
}

int FTDIListener(struct ftdi_context *ftdihandle)
{
	int returnvalue,i;
	unsigned char * bufferptr;
	
	#ifdef DEBUGMODE
	printf("FTDIListener\n");
	#endif 
	do
	{
		
		do
		{
			returnvalue=ftdi_read_data(ftdihandle, RXBUFFER, 2);
		}while(returnvalue==0);
		if(returnvalue>0)
		{
			for(i=0;i<returnvalue;i++)
			{				
				rx_fifo.BUFFER[(rx_fifo.ptr_in)&(BUFFERSIZE-1)]=RXBUFFER[i];
				rx_fifo.ptr_in=(rx_fifo.ptr_in+1)&(BUFFERSIZE-1);
			}
			
			#ifdef DEBUGMODE
			printf("rx : %d %d \n",rx_fifo.ptr_in,returnvalue);
			#endif
			if(READ_THREAD_EVENT)setevent(READ_THREAD_EVENT);
		}
		else
		{
			stop_thread=1;
			if(READ_THREAD_EVENT)setevent(READ_THREAD_EVENT);
		}

	}while(!stop_thread);
	setevent(STOP_THREAD_EVENT);
	return 0;
}

int createlistenerthread(RDTHREADFUNCTION thread,int priority,struct ftdi_context * ftdihandle)
{
	unsigned long sit;
	pthread_t threadid;
	listenerthreadinit *threadinitptr;
	pthread_attr_t threadattrib;
	struct sched_param param;

	//pthread_attr_create(&threadattrib);
	pthread_attr_init(&threadattrib);
	pthread_attr_setinheritsched(&threadattrib,1);
	//pthread_attr_setsched(&threadattrib,SCHED_FIFO);
	//pthread_attr_setprio(&threadattrib,4);
	
	pthread_attr_setschedpolicy(&threadattrib,SCHED_FIFO);
	param.sched_priority = sched_get_priority_max(SCHED_FIFO) ;
	/* set the new scheduling param */
	pthread_attr_setschedparam (&threadattrib, &param);

	threadinitptr=(listenerthreadinit *)malloc(sizeof(listenerthreadinit));
	threadinitptr->thread=thread;
	threadinitptr->ftdihandle=ftdihandle;

	//pthread_create(&threadid, &threadattrib,FTDIListenerThreadProc, threadinitptr);
	pthread_create(&threadid,0,FTDIListenerThreadProc, threadinitptr);

	return sit;
}


#endif


int ftdi_load_lib (HXCFLOPPYEMULATOR* floppycontext)
{
	#ifdef DEBUGMODE
	printf("---ftdi_load_lib---\n");
	#endif
	#ifdef FTDILIB
        ftdi_init(&ftdic);
	#endif
	return 1;
}


int open_ftdichip(unsigned long * ftdihandle)
{
	static unsigned char c=0;
	int ret;
	#ifdef DEBUGMODE
	printf("---open_ftdichip---\n");
	#endif

	#ifdef FTDILIB

	ret = ftdi_usb_open(&ftdic, 0x0403, 0x6001);
	if(ret<0)
	{
		c++;
		*ftdihandle=0;
		return -1;
	}
	else
	{
		stop_thread=0;
		*ftdihandle=(unsigned long)(&ftdic);
		STOP_THREAD_EVENT=createevent();
		READ_THREAD_EVENT=0;
		tx_fifo.ptr_in=0;
		tx_fifo.ptr_out=0;
		memset(tx_fifo.BUFFER,0,BUFFERSIZE);
		rx_fifo.ptr_in=0;
		rx_fifo.ptr_out=0;
		memset(rx_fifo.BUFFER,0,BUFFERSIZE);

		createlistenerthread(FTDIListener,128,&ftdic);
	}

	#else

	if(FT_Open(c&0xf,(FT_HANDLE*)ftdihandle)!=FT_OK)
	{
		c++;
		*ftdihandle=0;
		return -1;
	}

	#endif
	return 0;
}

int close_ftdichip(unsigned long ftdihandle)
{
	#ifdef DEBUGMODE
	printf("---close_ftdichip---\n");
	#endif
	#ifdef FTDILIB
	stop_thread=1;
	waitevent(STOP_THREAD_EVENT,10000);
        ftdi_usb_close((struct ftdi_context *)ftdihandle);
	destroyevent(STOP_THREAD_EVENT);
	#else

	if(FT_Close((FT_HANDLE)ftdihandle)!=FT_OK)
	{
		return -1;
	}

	#endif
	return 0;
}

int purge_ftdichip(unsigned long ftdihandle,unsigned long buffer)
{
	int ret;
	#ifdef DEBUGMODE
	printf("---purge_ftdichip---\n");
	#endif

	#ifdef FTDILIB

	ftdi_usb_purge_rx_buffer((struct ftdi_context *)ftdihandle);
        ftdi_usb_purge_tx_buffer((struct ftdi_context *)ftdihandle);
        ret=ftdi_usb_purge_buffers((struct ftdi_context *)ftdihandle);
	
	rx_fifo.ptr_in=0;
	rx_fifo.ptr_out=0;
	
	if(ret)
	{
		return -1;	
	}
	
	#else

	if(FT_Purge((FT_HANDLE)ftdihandle,buffer)!=FT_OK)
	{
		return -1;
	}

	#endif

	return 0;
}

int setusbparameters_ftdichip(unsigned long ftdihandle,unsigned long buffersizetx,unsigned long buffersizerx)
{
	#ifdef DEBUGMODE
	 printf("---setusbparameters_ftdichip---\n");
	#endif

	#ifdef FTDILIB

	 ftdi_read_data_set_chunksize((struct ftdi_context *)ftdihandle, buffersizerx);
         ftdi_write_data_set_chunksize((struct ftdi_context *)ftdihandle, buffersizetx);

	#else

	if(FT_SetUSBParameters ((FT_HANDLE)ftdihandle,buffersizerx,buffersizetx)!=FT_OK)
	{
		return -1;
	}

	#endif

	return 0;
}

int setlatencytimer_ftdichip(unsigned long ftdihandle,unsigned char latencytimer_ms)
{
	#ifdef DEBUGMODE
	printf("---setlatencytimer_ftdichip---\n");
	#endif

	#ifdef FTDILIB

	if(ftdi_set_latency_timer((struct ftdi_context *)ftdihandle, latencytimer_ms)<0)
	{
             return -1;
	}

	#else

	if(FT_SetLatencyTimer ((FT_HANDLE)ftdihandle,latencytimer_ms)!=FT_OK)
	{
		return -1;
	}

	#endif


	return 0;
}

int write_ftdichip(unsigned long ftdihandle,unsigned char * buffer,unsigned int size)
{
	long dwWritten;
	#ifdef DEBUGMODE
	printf("---write_ftdichip---\n");
	#endif

	#ifdef FTDILIB

	dwWritten=ftdi_write_data((struct ftdi_context *)ftdihandle, buffer, size);
        if(dwWritten<0)
	{
		return -1;
        }

	return dwWritten;

	#else

	if(FT_Write ((FT_HANDLE)ftdihandle, (LPVOID)buffer, size,&dwWritten)!=FT_OK)
	{
		return -1;
	}

        #endif

	return dwWritten;
}

int read_ftdichip(unsigned long ftdihandle,unsigned char * buffer,unsigned int size)
{
	long returnvalue,nb_of_byte;
	
	#ifdef DEBUGMODE
	printf("---read_ftdichip---\n");
	#endif

	#ifdef FTDILIB
	nb_of_byte=0;
	while((rx_fifo.ptr_in != rx_fifo.ptr_out) && (nb_of_byte<size))
	{
		buffer[nb_of_byte]=rx_fifo.BUFFER[(rx_fifo.ptr_out)&(BUFFERSIZE-1)];
		nb_of_byte++;
		rx_fifo.ptr_out=(rx_fifo.ptr_out+1)&(BUFFERSIZE-1);
	}
	if(stop_thread)
	{
		return -1;
	}
	else
	{
		return nb_of_byte;
	}

	#else

	if(FT_Read((FT_HANDLE)ftdihandle,buffer,size,&returnvalue)!=FT_OK)
	{
		return -1;
	}

	#endif
	return returnvalue;
}

int getfifostatus_ftdichip(unsigned long ftdihandle,unsigned long * txlevel,unsigned long *rxlevel,unsigned long * event)
{
	long nb_of_byte,ptr_out;
	#ifdef DEBUGMODE
	printf("---getfifostatus_ftdichip---\n");
	#endif

	#ifdef FTDILIB
		if(rx_fifo.ptr_in != rx_fifo.ptr_out)
		{
			nb_of_byte=0;
			ptr_out=rx_fifo.ptr_out&(BUFFERSIZE-1);
			while((rx_fifo.ptr_in != ptr_out))
			{
				nb_of_byte++;			
				ptr_out=(ptr_out+1)&(BUFFERSIZE-1);
			}
			*event=FT_EVENT_RXCHAR;
			*rxlevel=nb_of_byte;
			//printf("%d...\n",*rxlevel);
		}
		else
		{
			*txlevel=0;
			*rxlevel=0;
			*event=0;
		
		}

	#else

	if(FT_GetStatus((FT_HANDLE)ftdihandle,rxlevel,txlevel,event)!=FT_OK)
	{
		return -1;
	}

        #endif

	return 0;
}

int seteventnotification_ftdichip(unsigned long ftdihandle,unsigned long eventmask,void * event)
{
	#ifdef DEBUGMODE
	printf("---seteventnotification_ftdichip---\n");
	#endif

	#ifdef FTDILIB

	READ_THREAD_EVENT=(EVENT_HANDLE*)event;
	#else

	if(FT_SetEventNotification((FT_HANDLE)ftdihandle,eventmask,event)!=FT_OK)
	{
		return -1;
	}

	#endif

	return 0;
}
/////////////
