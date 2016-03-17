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

#include "hxc_floppy_emulator.h"
#include "internal_floppy.h"
#include "floppy_loader.h"
#include "./usb_floppyemulator/usb_hxcfloppyemulator.h"
#include "linux_api.h"
#include <stdlib.h>
//#include <string.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <ctype.h>
#include <pthread.h>
#include <sched.h>
#include <errno.h>
#include "ftd2xx.h"


EVENT_HANDLE * eventtab[256];

void strlwr(char *string)
{
	int i;

	i=0;
	while (string[i])
	{
		string[i] = tolower(string[i]);
		i++;
	}
}


void * ThreadProc( void *lpParameter)
{
	threadinit *threadinitptr;
	THREADFUNCTION thread;
	HXCFLOPPYEMULATOR* floppycontext;
	HWINTERFACE* hw_context;

	//SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	threadinitptr=(threadinit*)lpParameter;
	thread=threadinitptr->thread;
	floppycontext=threadinitptr->hxcfloppyemulatorcontext;
	hw_context=threadinitptr->hwcontext;
	thread(floppycontext,hw_context);

	return 0;
}

/*
DWORD WINAPI ThreadProc( LPVOID lpParameter)
{
	threadinit *threadinitptr;
	THREADFUNCTION thread;
	HXCFLOPPYEMULATOR* floppycontext;
	HWINTERFACE* hw_context;

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

	threadinitptr=(threadinit*)lpParameter;
	thread=threadinitptr->thread;
	floppycontext=threadinitptr->hxcfloppyemulatorcontext;
	hw_context=threadinitptr->hwcontext;
	thread(floppycontext,hw_context);

	return 0;
}

unsigned long hxc_create_ftdi_event(HXCFLOPPYEMULATOR* floppycontext,unsigned char id)
{
	//eventtab[id]=CreateEvent(NULL,FALSE,FALSE,NULL);
	//seteventnotification_ftdichip(eventtab[id]);
	return (unsigned long)eventtab[id];
}
*/

int hxc_setevent(HXCFLOPPYEMULATOR* floppycontext,unsigned char id)
{
	//SetEvent(eventtab[id]);
	return 0;
}

unsigned long  hxc_createevent(HXCFLOPPYEMULATOR* floppycontext,unsigned char id)
{
	//eventtab[id]=CreateEvent(NULL,FALSE,FALSE,NULL);
	eventtab[id]=(EVENT_HANDLE*)malloc(sizeof(EVENT_HANDLE));
	pthread_mutex_init(&eventtab[id]->eMutex, NULL);
	pthread_cond_init(&eventtab[id]->eCondVar, NULL);
	return (unsigned long)eventtab[id];
}

int hxc_waitevent(HXCFLOPPYEMULATOR* floppycontext,int id,int timeout)
{

struct timeval now;
struct timespec timeoutstr;
int retcode;
	int ret;

	//if(timeout==0) timeout=INFINITE;
	//ret=WaitForSingleObject(eventtab[id],timeout);


	pthread_mutex_lock(&eventtab[id]->eMutex);
	gettimeofday(&now,0);
	timeoutstr.tv_sec = now.tv_sec + (timeout/1000);
	timeoutstr.tv_nsec = (now.tv_usec * 1000) ;//+(timeout*1000000);
	retcode = 0;

	retcode = pthread_cond_timedwait(&eventtab[id]->eCondVar, &eventtab[id]->eMutex, &timeoutstr);	
	if (retcode == ETIMEDOUT) 
	{
	  pthread_mutex_unlock(&eventtab[id]->eMutex);
      	  return 1;
	} 
	else 
	{
      	  pthread_mutex_unlock(&eventtab[id]->eMutex);
      	  return 0; 

	}
	
}

void hxc_pause(int ms)
{
	sleep(1);
	//msleep(ms);
}

int hxc_createthread(HXCFLOPPYEMULATOR* floppycontext,HWINTERFACE* hwcontext,THREADFUNCTION thread,int priority)
{
	unsigned long sit;
	pthread_t threadid;
	pthread_attr_t threadattrib;
	threadinit *threadinitptr;
	struct sched_param param;

	pthread_attr_init(&threadattrib); 
	
	pthread_attr_setschedpolicy(&threadattrib,SCHED_FIFO);
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	/* set the new scheduling param */
	pthread_attr_setschedparam (&threadattrib, &param);

	threadinitptr=(threadinit *)malloc(sizeof(threadinit));
	threadinitptr->thread=thread;
	threadinitptr->hxcfloppyemulatorcontext=floppycontext;
	threadinitptr->hwcontext=hwcontext;

	pthread_create(&threadid, &threadattrib,ThreadProc, threadinitptr);

	return sit;
}

int getlistoffile(unsigned char * directorypath,unsigned char *** filelist)
{
	int numberoffile;
	char ** filepathtab;
/*	
	HANDLE findfilehandle;
	WIN32_FIND_DATA FindFileData;

	filepathtab=0;
	numberoffile=0;

	findfilehandle=FindFirstFile(directorypath,&FindFileData);
	if(findfilehandle!=INVALID_HANDLE_VALUE)
	{
		
		do
		{
			filepathtab=(char **) realloc(filepathtab,sizeof(char*)*(numberoffile+1));
			filepathtab[numberoffile]=(char*)malloc(strlen(FindFileData.cFileName)+1);
			strcpy(filepathtab[numberoffile],FindFileData.cFileName);
			numberoffile++;	
		}while(FindNextFile(findfilehandle,&FindFileData));
				
		FindClose(findfilehandle);
	}
	*filelist=filepathtab;*/

	return 0;//numberoffile;
}


char * getcurrentdirectory(char *currentdirectory,int buffersize)
{
	memset(currentdirectory,0,buffersize);
/*	if(GetModuleFileName(GetModuleHandle(NULL),currentdirectory,buffersize))
	{
		if(strrchr(currentdirectory,'\\'))
		{
			*((char*)strrchr(currentdirectory,'\\'))=0;
			return currentdirectory;
		}
	}*/



	return 0;
}


int loaddiskplugins(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * newfloppy,char *pluginpath)
{
/*	HMODULE module;

//	module=LoadLibrary(pluginpath);
	if(module)
	{
			newfloppy->IsValidDiskFile=(ISVALIDDISKFILE )GetProcAddress(module,"IsValidDiskFile");
			newfloppy->UnLoadDiskFile=(UNLOADDISKFILE )GetProcAddress(module,"UnLoadDiskFile");
			newfloppy->GetFloppyPropretiesInt=(GETFLOPPYPROPRETIESINT)GetProcAddress(module,"GetFloppyPropretiesInt");
			newfloppy->GetTrackData=(GETTRACKDATA)GetProcAddress(module,"GetTrackData");
			newfloppy->GetIndexData=(GETINDEXDATA)GetProcAddress(module,"GetIndexData");
			newfloppy->LoadDiskFile=(LOADDISKFILE)GetProcAddress(module,"LoadDiskFile");
			if(newfloppy->IsValidDiskFile && newfloppy->UnLoadDiskFile && newfloppy->GetFloppyPropretiesInt && newfloppy->GetTrackData && newfloppy->GetIndexData && newfloppy->LoadDiskFile)
			{
				return 1;
			}
			else
			{
				FreeLibrary(module);
			}
	}*/

	return 0;
}

long find_first_file(char *folder,char *file,filefoundinfo* fileinfo)
{
	return -1;
}

long find_next_file(long handleff,char *folder,char *file,filefoundinfo* fileinfo)
{
	return 0;
}

long find_close(long handle)
{

	return 0;
}

char * strupper(char * str)
{
	int i;
	
	i=0;
	while(str[i])
	{
	
		if(str[i]>='a' && str[i]<='z')
		{
			str[i]=str[i]+('A'-'a');
		}
		i++;
	}

	return str;
}


char * strlower(char * str)
{
	int i;
	
	i=0;
	while(str[i])
	{
	
		if(str[i]>='A' && str[i]<='Z')
		{
			str[i]=str[i]+('a'-'A');
		}
		i++;
	}

	return str;
}
