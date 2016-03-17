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
///////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------//
//-----------H----H--X----X-----CCCCC----22222----0000-----0000------11----------//
//----------H----H----X-X-----C--------------2---0----0---0----0--1--1-----------//
//---------HHHHHH-----X------C----------22222---0----0---0----0-----1------------//
//--------H----H----X--X----C----------2-------0----0---0----0-----1-------------//
//-------H----H---X-----X---CCCCC-----222222----0000-----0000----1111------------//
//-------------------------------------------------------------------------------//
//----------------------------------------------------- http://hxc2001.free.fr --//
///////////////////////////////////////////////////////////////////////////////////
// File : AMIGADOSFSDK_DiskFile.c
// Contains: AMIGADOSFSDK floppy image loader and plugins interfaces
//
// Written by:	DEL NERO Jean Francois
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include "hxc_floppy_emulator.h"
#include "internal_floppy.h"
#include "floppy_loader.h"
#include "floppy_utils.h"

#include "../common/track_generator.h"

#include "amigadosfs_loader.h"

#include "../os_api.h"


#include "./libs/adflib/Lib/adflib.h"
#include "stdboot3.h"


HXCFLOPPYEMULATOR* global_floppycontext;

int AMIGADOSFSDK_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	
	int pathlen;
	char * filepath;
    struct stat staterep;

	floppycontext->hxc_printf(MSG_DEBUG,"AMIGADOSFSDK_libIsValidDiskFile %s",imgfile);
	if(imgfile)
	{
		pathlen=strlen(imgfile);
		if(pathlen!=0)
		{		
			stat(imgfile,&staterep);

			if(staterep.st_mode&S_IFDIR)
			{
				filepath=malloc(pathlen+1);
				if(filepath!=0)
				{
					sprintf(filepath,"%s",imgfile);
					strlower(filepath);

					if(strstr( filepath,".amigados" )!=NULL)
					{
						floppycontext->hxc_printf(MSG_DEBUG,"AMIGADOSFSDK file !");
						free(filepath);
						return LOADER_ISVALID;
					}
					else
					{
						floppycontext->hxc_printf(MSG_DEBUG,"non AMIGADOSFSDK file ! (.amigados missing)");
						free(filepath);
						return LOADER_BADFILE;
					}
				}
			}
			else
			{
				floppycontext->hxc_printf(MSG_DEBUG,"non AMIGADOSFSDK file ! (it's not a directory)");
				return LOADER_BADFILE;
			}
		}
		floppycontext->hxc_printf(MSG_DEBUG,"0 byte string ?");
	}
	return LOADER_BADPARAMETER;


}

void adlib_printerror(char * msg)
{
	global_floppycontext->hxc_printf(MSG_ERROR,"AdfLib Error: %s",msg);
}

void adlib_printwarning(char * msg)
{
	global_floppycontext->hxc_printf(MSG_WARNING,"AdfLib Warning: %s",msg);
}

void adlib_printdebug(char * msg)
{
	global_floppycontext->hxc_printf(MSG_DEBUG,"AdfLib Debug: %s",msg);
}

int ScanFile(HXCFLOPPYEMULATOR* floppycontext,struct Volume * adfvolume,char * folder,char * file)
{
	long hfindfile;
	filefoundinfo FindFileData;
	int bbool;
	int byte_written;
	FILE * ftemp;
	unsigned char  tempbuffer[512];
	struct File* adffile;
	unsigned char * fullpath;//,*fileimg;
	int size,filesize;
	RETCODE  rc;
	
	hfindfile=find_first_file(folder,file, &FindFileData); 
	if(hfindfile!=-1)
	{
		bbool=TRUE;
		while(hfindfile!=-1 && bbool)
		{
			if(FindFileData.isdirectory)
			{
				if(strcmp(".",FindFileData.filename)!=0 && strcmp("..",FindFileData.filename)!=0)
				{
					if(adfCountFreeBlocks(adfvolume)>4)
					{
						floppycontext->hxc_printf(MSG_INFO_1,"Adding directory %s",FindFileData.filename);
						rc=adfCreateDir(adfvolume,adfvolume->curDirPtr,FindFileData.filename); 
						if(rc==RC_OK)
						{
							floppycontext->hxc_printf(MSG_INFO_1,"entering directory %s",FindFileData.filename);
							rc=adfChangeDir(adfvolume, FindFileData.filename);
							if(rc==RC_OK)
							{
								
								fullpath=malloc(strlen(FindFileData.filename)+strlen(folder)+2);
								sprintf(fullpath,"%s\\%s",folder,FindFileData.filename);

								if(ScanFile(floppycontext,adfvolume,fullpath,file))
								{
									adfParentDir(adfvolume);
									free(fullpath);
									return 1;
								}
								floppycontext->hxc_printf(MSG_INFO_1,"Leaving directory %s",FindFileData.filename);
								free(fullpath);
								adfParentDir( adfvolume);
							}
							else
							{
								floppycontext->hxc_printf(MSG_ERROR,"Cannot enter to the directory %s !",FindFileData.filename);
								return 1;
							}
						}
						else
						{
							floppycontext->hxc_printf(MSG_ERROR,"Cannot Add the directory %s !",FindFileData.filename);
							return 1;
						}
					}
					else
					{
						floppycontext->hxc_printf(MSG_ERROR,"Cannot Add a directory ! : no more free block!!!");
					    return 1;
					}
					
				}
			}
			else
			{
				if(adfCountFreeBlocks(adfvolume)>4)
				{
						floppycontext->hxc_printf(MSG_INFO_1,"Adding file %s, %dB",FindFileData.filename,FindFileData.size);
						adffile = adfOpenFile(adfvolume, FindFileData.filename, "w");
						if(adffile)
						{
							if(FindFileData.size)
							{
								fullpath=malloc(strlen(FindFileData.filename)+strlen(folder)+2);
								sprintf(fullpath,"%s\\%s",folder,FindFileData.filename);

								ftemp=fopen(fullpath,"rb");
								if(ftemp)
								{
									fseek(ftemp,0,SEEK_END);
									filesize=ftell(ftemp);
									fseek(ftemp,0,SEEK_SET);
									do
									{					
										if(filesize>=512)
										{
											size=512;		
										}
										else
										{
                                            size=filesize;
										}
										fread(&tempbuffer,size,1,ftemp);

										byte_written=adfWriteFile(adffile, size, tempbuffer);
										if((byte_written!=size) || (adfCountFreeBlocks(adfvolume)<2) )
										{
											floppycontext->hxc_printf(MSG_ERROR,"Error while writting the file %s. No more free block ?",FindFileData.filename);
											adfCloseFile(adffile);
											fclose(ftemp);
											free(fullpath);
											return 1;
										}
										filesize=filesize-512;

									}while( (filesize>0) && (byte_written==size));
								

									/*fileimg=(unsigned char*)malloc(filesize);
									memset(fileimg,0,filesize);
									fread(fileimg,filesize,1,ftemp);
									adfWriteFile(adffile, filesize, fileimg);
									free(fileimg);*/

									adfCloseFile(adffile);
									fclose(ftemp);
									free(fullpath);
								}
								else
								{
										floppycontext->hxc_printf(MSG_ERROR,"Error : Cannot open %s !!!",fullpath);
										free(fullpath);
										return 1;
								}
							}
						}
						else
						{
                             floppycontext->hxc_printf(MSG_ERROR,"Error : Cannot create %s, %dB!!!",FindFileData.filename,FindFileData.size);
							 return 1;
						}
				}
				else
				{
					floppycontext->hxc_printf(MSG_ERROR,"Error : Cannot add a file : no more free block");
					return 1;
				}
			}
			bbool=find_next_file(hfindfile,folder,file,&FindFileData);
		}
		
	}
	else printf("Error FindFirstFile\n");

	find_close(hfindfile);
	return 0;
}

int AMIGADOSFSDK_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	unsigned int i,j;
	unsigned int file_offset;
	struct Device * adfdevice;
	struct Volume * adfvolume;
	unsigned char * flatimg;
	unsigned char * flatimg2;
	unsigned char * repname;
	int flatimgsize;
	int numberoftrack;
	int numberofsectorpertrack;

	unsigned short sectorsize;
	unsigned char gap3len,skew,trackformat,interleave;


    struct stat repstate;
	struct tm * ts;
	struct DateTime reptime;

//	FILE * debugadf;
	int rc;
	CYLINDER* currentcylinder;
  
	numberoftrack=80;
	numberofsectorpertrack=11;
	
	floppycontext->hxc_printf(MSG_DEBUG,"AMIGADOSFSDK_libLoad_DiskFile %s",imgfile);

	stat(imgfile,&repstate);
	ts=localtime(&repstate.st_ctime);
	if(repstate.st_mode&S_IFDIR)
	{

		global_floppycontext=floppycontext;
		adfEnvInitDefault();
		adfChgEnvProp(PR_EFCT,adlib_printerror);
		adfChgEnvProp(PR_WFCT,adlib_printwarning);
		adfChgEnvProp(PR_VFCT,adlib_printdebug);

		floppycontext->hxc_printf(MSG_DEBUG,"ADFLib %s %s",adfGetVersionNumber(), adfGetVersionDate());

		adfdevice = adfCreateMemoryDumpDevice(numberoftrack, 2, numberofsectorpertrack,&flatimg,&flatimgsize);
		if(adfdevice)
		{

			repname=(unsigned char *)malloc(strlen(imgfile)+1);
			memset(repname,0,strlen(imgfile)+1);
			i=strlen(imgfile);
			if( (imgfile[i]=='\\' || imgfile[i]=='/') && i)
			{
				imgfile[i]=0;
				i--;
			}
			while(i && (imgfile[i]!='\\' && imgfile[i]!='/'))
			{
				i--;
			}
			if((imgfile[i]=='\\' || imgfile[i]=='/'))
			{
				 i++;
			}
			sprintf(repname,"%s",&imgfile[i]);

			if(ts)
			{
				reptime.year=ts->tm_year;         /* since 1900 */
				reptime.mon=ts->tm_mon+1;
				reptime.day=ts->tm_mday;
				reptime.hour=ts->tm_hour;
				reptime.min=ts->tm_min;
				reptime.sec=ts->tm_sec;
			}
			else
			{
				reptime.year=0;
				reptime.mon=0;
				reptime.day=0;
				reptime.hour=0;
				reptime.min=0;
				reptime.sec=0;

			}

			rc=adfCreateFlop(adfdevice, repname, 0,&reptime );
			free(repname);

			if (rc==RC_OK)
			{					
				adfvolume = adfMount(adfdevice, 0, 0);
				if(adfvolume)
				{
					floppycontext->hxc_printf(MSG_DEBUG,"adfCreateFlop ok");
					if(adfInstallBootBlock(adfvolume, stdboot3)!=RC_OK)
					{
						floppycontext->hxc_printf(MSG_ERROR,"adflib: adfInstallBootBlock error!");
					}

					if(ScanFile(floppycontext,adfvolume,imgfile,"*.*"))
					{
								floppycontext->hxc_printf(MSG_DEBUG,"ScanFile error!");
								return LOADER_INTERNALERROR;
					}
					flatimg2=(unsigned char*)malloc(flatimgsize);
					memcpy(flatimg2,flatimg,flatimgsize);
					adfUnMountDev(adfdevice);			
					/*////////
					debugadf=fopen("d:\\debug.adf","wb");
					if(debugadf)
					{
					   fwrite(flatimg2,flatimgsize,1,debugadf);
					   fclose(debugadf);
					}
					//////////*/

				}
				else
				{
					floppycontext->hxc_printf(MSG_ERROR,"adflib: adfMount error!");
					return LOADER_INTERNALERROR;
				}
			}
			else
			{
				floppycontext->hxc_printf(MSG_ERROR,"adflib: Error while creating the virtual floppy!");
				return LOADER_INTERNALERROR;
			}
		}
		else
		{
			floppycontext->hxc_printf(MSG_ERROR,"adflib: adfCreateMemoryDumpDevice error!");
			return LOADER_INTERNALERROR;
		}

		if(flatimg2)
		{		
			sectorsize=512;
			interleave=1;
			gap3len=0;
			skew=0;
			trackformat=AMIGAFORMAT_DD;

			floppydisk->floppySectorPerTrack=numberofsectorpertrack;
			floppydisk->floppyNumberOfSide=2;
			floppydisk->floppyNumberOfTrack=numberoftrack;
			floppydisk->floppyBitRate=DEFAULT_AMIGA_BITRATE;
			floppydisk->floppyiftype=AMIGA_DD_FLOPPYMODE;
			floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);

			for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
			{
				
				floppydisk->tracks[j]=allocCylinderEntry(DEFAULT_AMIGA_RPM,floppydisk->floppyNumberOfSide);
				currentcylinder=floppydisk->tracks[j];
										
				for(i=0;i<floppydisk->floppyNumberOfSide;i++)
				{	
					file_offset=(sectorsize*(j*floppydisk->floppySectorPerTrack*floppydisk->floppyNumberOfSide))+
								(sectorsize*(floppydisk->floppySectorPerTrack)*i);
					
					currentcylinder->sides[i]=tg_generatetrack(&flatimg2[file_offset],sectorsize,floppydisk->floppySectorPerTrack,(unsigned char)j,(unsigned char)i,0,interleave,(unsigned char)(((j<<1)|(i&1))*skew),floppydisk->floppyBitRate,currentcylinder->floppyRPM,trackformat,gap3len,2500,-11150);
				}
			}

			free(flatimg2);
			floppycontext->hxc_printf(MSG_INFO_1,"AMIGADOSFSDK Loader : tracks file successfully loaded and encoded!");
			return LOADER_NOERROR;
		}
			
		floppycontext->hxc_printf(MSG_ERROR,"flatimg==0 !?");
		return LOADER_INTERNALERROR;
	}
	else
	{
		floppycontext->hxc_printf(MSG_ERROR,"not a directory !");
		return LOADER_BADFILE;
	}
}


