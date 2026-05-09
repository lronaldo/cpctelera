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
// File : IPF_DiskFile.c
// Contains: IPF floppy image loader and plugins interfaces
//
// Written by:	DEL NERO Jean Francois
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////////
#ifdef IPF_SUPPORT 

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "hxc_floppy_emulator.h"
#include "internal_floppy.h"
#include "floppy_loader.h"
#include "floppy_utils.h"

#include "ipf_loader.h"

#define BYTE char 
#define WORD short
#define DWORD long
#define LIB_TYPE 1
#define LIB_USER 1
#include "./libs/capslib/Comtype.h"
#include "./libs/capslib/CapsAPI.h"

#include "../common/os_api.h"

#include "capslibloader.h"

typedef SDWORD (__cdecl* CAPSINIT)(void);
typedef SDWORD (__cdecl* CAPSADDIMAGE)(void);
typedef SDWORD (__cdecl* CAPSLOCKIMAGEMEMORY)(SDWORD,PUBYTE,UDWORD,UDWORD);
typedef SDWORD (__cdecl* CAPSUNLOCKIMAGE)(SDWORD);
typedef SDWORD (__cdecl* CAPSLOADIMAGE)(SDWORD,UDWORD);
typedef SDWORD (__cdecl* CAPSGETIMAGEINFO)(PCAPSIMAGEINFO,SDWORD);
typedef SDWORD (__cdecl* CAPSLOCKTRACK)(PCAPSTRACKINFO,SDWORD,UDWORD,UDWORD,UDWORD);
typedef SDWORD (__cdecl* CAPSUNLOCKTRACK)(SDWORD id, UDWORD cylinder, UDWORD head);
typedef SDWORD (__cdecl* CAPSUNLOCKALLTRACKS)(SDWORD);
typedef SDWORD (__cdecl* CAPSGETVERSIONINFO)(PCAPSVERSIONINFO,UDWORD);
typedef SDWORD (__cdecl* CAPSREMIMAGE)(SDWORD id);

extern  CAPSINIT pCAPSInit;
extern  CAPSADDIMAGE pCAPSAddImage;
extern  CAPSLOCKIMAGEMEMORY pCAPSLockImageMemory;
extern  CAPSUNLOCKIMAGE pCAPSUnlockImage;
extern  CAPSLOADIMAGE pCAPSLoadImage;
extern  CAPSGETIMAGEINFO pCAPSGetImageInfo;
extern  CAPSLOCKTRACK pCAPSLockTrack;
extern  CAPSUNLOCKTRACK pCAPSUnlockTrack;
extern  CAPSUNLOCKALLTRACKS pCAPSUnlockAllTracks;
extern  CAPSGETVERSIONINFO pCAPSGetVersionInfo;
extern  CAPSREMIMAGE pCAPSRemImage;

int IPF_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen;
	char * filepath;
	floppycontext->hxc_printf(MSG_DEBUG,"IPF_libIsValidDiskFile %s",imgfile);
	if(imgfile)
	{
		pathlen=strlen(imgfile);
		if(pathlen!=0)
		{
			filepath=malloc(pathlen+1);
			if(filepath!=0)
			{
				sprintf(filepath,"%s",imgfile);
				strlower(filepath);

				if(strstr( filepath,".ipf" )!=NULL)
				{
					floppycontext->hxc_printf(MSG_DEBUG,"IPF file !");
					free(filepath);
					if(init_caps_lib())
					{
						return LOADER_ISVALID;
					}
					else
					{
						floppycontext->hxc_printf(MSG_ERROR,"No Caps lib available!");
						return LOADER_INTERNALERROR;
					}
				}
				else
				{
					floppycontext->hxc_printf(MSG_DEBUG,"non IPF file !");
					free(filepath);
					return LOADER_BADFILE;
				}
			}
		}
	}

	return LOADER_BADPARAMETER;
}

unsigned long trackcopy(unsigned char * dest,unsigned char * src,unsigned long overlap,unsigned long tracklen,unsigned char nooverlap)
{
	unsigned long i,j,k,dest_tracklen;


	k=0;
	j=0;

	for(i=0;i<overlap;i++)
	{
		if( (src[k>>3]>>(0x7-(k&0x7)))&1)
		{
			dest[j>>3]=dest[j>>3]|(0x80>>(j&0x7));
		}
		else
		{
			dest[j>>3]=dest[j>>3]&(~((0x80)>>(j&0x7)));
		}

		j++;
		if(j>=tracklen) j=0;

		k++;
		if(k>=tracklen) k=0;

	}

	dest_tracklen=tracklen;
/*	if(tracklen&0x7)
	{
	  j=j+(((tracklen&~0x7)+8)-tracklen);
	  dest_tracklen=tracklen+(((tracklen&~0x7)+8)-tracklen);
	}*/

	if(!nooverlap)
	{
		for(i=0;i<1;i++)
		{
			dest[j>>3]=dest[j>>3]&(~((0x80)>>(j&0x7)));
			j++;
			if(j>=tracklen) j=0;
			dest_tracklen++;
		}
	}

	for(i=overlap;i<tracklen;i++)
	{
		if( (src[k>>3]>>(0x7-(k&0x7)))&1)
		{
			dest[j>>3]=dest[j>>3]|(0x80>>(j&0x7));
		}
		else
		{
			dest[j>>3]=dest[j>>3]&(~((0x80)>>(j&0x7)));
		}

		j++;
		if(j>=dest_tracklen) 
		{
			j=0;
		}

		k++;
		if(k>=tracklen) 
		{
			k=0;
		}
	}

	return dest_tracklen;
}

int IPF_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	
	unsigned int filesize;
	unsigned int i,j,k,l,m,len;
	unsigned char *fileimg;
	unsigned char oldlib;
	typedef struct CapsImageInfo CapsImageInfo_;
	struct CapsTrackInfoT1 ti;
	struct CapsTrackInfoT1 flakeyti;
	struct CapsVersionInfo cvi;

	unsigned char * temptrack;
	CapsImageInfo_ ci2;
	FILE * f;
	int img;
	int ret;
	int overlap;
	int intrackflakeybit;
	unsigned long bitrate;
	int rpm,sizefactor;
	CYLINDER* currentcylinder;
	unsigned char flakeybyte;
	SIDE* currentside;
	
	UDWORD flag;
	
	floppycontext->hxc_printf(MSG_DEBUG,"IPF_libLoad_DiskFile %s",imgfile);
	
	f=fopen(imgfile,"rb");
	if(f==NULL) 
	{
		floppycontext->hxc_printf(MSG_ERROR,"Cannot open %s !",imgfile);
		return LOADER_ACCESSERROR;
	}
	
	fseek (f , 0 , SEEK_END); 
	filesize=ftell(f);
	fseek (f , 0 , SEEK_SET); 
	
	if(filesize!=0)
	{
		fileimg=(char*)malloc(filesize);
		
		if(fileimg!=NULL)
		{
			i=0;
			do
			{
				fread(fileimg+(i*1024),1024,1,f);
				i++;
			}while(i<((filesize/1024)+1));
			
		}
		else
		{
			floppycontext->hxc_printf(MSG_ERROR,"Memory error!");
			fclose(f);
			return LOADER_INTERNALERROR;
		}	
	}
	else
	{
		floppycontext->hxc_printf(MSG_ERROR,"0 byte file!");
		fclose(f);
		return LOADER_BADFILE;
	}
	
	fclose(f);
	
	cvi.type = LIB_TYPE;
	pCAPSGetVersionInfo (&cvi, 0);

	flag = DI_LOCK_DENVAR/*|DI_LOCK_DENNOISE|DI_LOCK_NOISE*/|DI_LOCK_UPDATEFD|DI_LOCK_TYPE | DI_LOCK_INDEX;
	
	floppycontext->hxc_printf(MSG_DEBUG,"CAPS: library version %d.%d (flags=%08X)", cvi.release, cvi.revision, cvi.flag);
	oldlib = (cvi.flag & (DI_LOCK_TRKBIT | DI_LOCK_OVLBIT)) != (DI_LOCK_TRKBIT | DI_LOCK_OVLBIT);
	sizefactor=8;

	if (!oldlib)
	{
		sizefactor=1;
		flag |= DI_LOCK_TRKBIT | DI_LOCK_OVLBIT;
	}

//	for (i = 0; i < 4; i++)
//		caps_cont[i] = pCAPSAddImage ();


	//flag=DI_LOCK_DENVAR|DI_LOCK_UPDATEFD|DI_LOCK_TYPE;

	
	img=pCAPSAddImage();
	
	if(img!=-1)
	{
		if(pCAPSLockImageMemory(img, fileimg,filesize,0)!=imgeUnsupported )
		{
			pCAPSLoadImage(img, flag);
			pCAPSGetImageInfo(&ci2, img);
			////// debug //////
			floppycontext->hxc_printf(MSG_DEBUG,"Image Info: %s",imgfile);
			floppycontext->hxc_printf(MSG_DEBUG,"Type     : %.8x",ci2.type);
			floppycontext->hxc_printf(MSG_DEBUG,"Release  : %.8x",ci2.release);
			floppycontext->hxc_printf(MSG_DEBUG,"Revision : %.8x",ci2.revision);
			floppycontext->hxc_printf(MSG_DEBUG,"Platform : %.8x",ci2.platform);
			floppycontext->hxc_printf(MSG_DEBUG,"minhead  : %d",ci2.minhead);
			floppycontext->hxc_printf(MSG_DEBUG,"maxhead  : %d",ci2.maxhead);
			floppycontext->hxc_printf(MSG_DEBUG,"mincylinder : %d",ci2.mincylinder);
			floppycontext->hxc_printf(MSG_DEBUG,"maxcylinder : %d",ci2.maxcylinder);
			floppycontext->hxc_printf(MSG_DEBUG,"Date : %d/%d/%d",ci2.crdt.day,ci2.crdt.month,ci2.crdt.year);
			///////////////////
			
			
			if(ci2.type == ciitFDD)
			{
				floppydisk->floppySectorPerTrack=0;
				floppydisk->floppyNumberOfSide=(unsigned char)(ci2.maxhead-ci2.minhead)+1;
				floppydisk->floppyNumberOfTrack=(unsigned char)ci2.maxcylinder+1;
				floppydisk->floppyBitRate=250000;
				rpm=300;
				floppydisk->floppyiftype=AMIGA_DD_FLOPPYMODE;
				floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
				memset(floppydisk->tracks,0,sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
				
				for(i=ci2.mincylinder;i<=ci2.maxcylinder;i++)
				{
					for(j=ci2.minhead;j<=ci2.maxhead;j++)
					{
						ti.type = LIB_TYPE;
						floppycontext->hxc_printf(MSG_DEBUG,"-----------------------------%d %d",i,j);
						ret=pCAPSLockTrack((struct CapsTrackInfo *)&ti, img, i, j, flag);
						if(ret==imgeOk)
						{
							floppycontext->hxc_printf(MSG_DEBUG,"Track Info  : %d %d",i,j);
							floppycontext->hxc_printf(MSG_DEBUG,"Cylinder    : %d",ti.cylinder);
							floppycontext->hxc_printf(MSG_DEBUG,"Head        : %d",ti.head);
							floppycontext->hxc_printf(MSG_DEBUG,"Sectorcnt   : %d",ti.sectorcnt);
							floppycontext->hxc_printf(MSG_DEBUG,"sectorsize  : %d",ti.sectorsize);
							floppycontext->hxc_printf(MSG_DEBUG,"Type        : %.8X",ti.type);
							//	floppycontext->hxc_printf(MSG_DEBUG,"trackcnt    : %d\n",ti.trackcnt);
							floppycontext->hxc_printf(MSG_DEBUG,"tracklen     : %d %s",ti.tracklen*sizefactor,(ti.tracklen*sizefactor)&0x7?"non aligned !":"aligned");
							floppycontext->hxc_printf(MSG_DEBUG,"overlap     : %d %s",ti.overlap*sizefactor,(ti.overlap*sizefactor)&0x7?"non aligned !":"aligned");
														
							if(!floppydisk->tracks[i])
							{
								floppydisk->tracks[i]=(CYLINDER*)malloc(sizeof(CYLINDER));
								currentcylinder=floppydisk->tracks[i];
								currentcylinder->number_of_side=floppydisk->floppyNumberOfSide;
								currentcylinder->sides=(SIDE**)malloc(sizeof(SIDE*)*currentcylinder->number_of_side);
								memset(currentcylinder->sides,0,sizeof(SIDE*)*currentcylinder->number_of_side);
								
								currentcylinder->floppyRPM=rpm;	
							}
							
							currentcylinder->sides[j]=malloc(sizeof(SIDE));
							memset(currentcylinder->sides[j],0,sizeof(SIDE));
							currentside=currentcylinder->sides[j];

							currentside->flakybitsbuffer=0;
							currentside->track_encoding=AMIGA_MFM_ENCODING;
							
							intrackflakeybit=-1;

							////////////////////////////////////////////////////////////////
							// get track data & flakey bit(s)
							if(!( (ti.type & CTIT_MASK_TYPE) == ctitNoise) && ti.trackbuf)
							{

								currentside->tracklen=ti.tracklen*sizefactor;
								len=(currentside->tracklen>>3)+16;

								currentside->databuffer=malloc(len);
								memset(currentside->databuffer,0,len);
								
								//flakey bits in track ?
								if(ti.type & CTIT_FLAG_FLAKEY)
								{

									temptrack=(unsigned char *)malloc(len);
									memset(temptrack,0,len);

									overlap=0;
									if(ti.overlap>=0)
										overlap=ti.overlap*sizefactor;


									currentside->tracklen=trackcopy(currentside->databuffer,ti.trackbuf,overlap,ti.tracklen,(unsigned char)((ti.overlap<0)?0xFF:0x00));

									currentside->flakybitsbuffer=malloc(len);
									memset(currentside->flakybitsbuffer,0x00,len);	

									pCAPSUnlockTrack(img,i, j);
									
									// try to read the track x time, and check for differences
									for(k=0;k<10;k++)
									{
										flakeyti.type = LIB_TYPE;
										ret=pCAPSLockTrack((struct CapsTrackInfo *)&flakeyti, img, i, j, flag);
										if(ret==imgeOk)
										{

											overlap=0;
											if(ti.overlap>=0)
												overlap=flakeyti.overlap;

											trackcopy(temptrack,flakeyti.trackbuf,overlap,flakeyti.tracklen,(unsigned char)((ti.overlap<0)?0xFF:0x00));

											for(l=0;l<ti.tracklen>>3;l++)
											{
												flakeybyte= temptrack[l] ^ currentside->databuffer[l];
												for(m=0;m<8;m=m+2)
												{
													// a flaybit detected ?
													if((flakeybyte&(0xC0>>m)) || ((currentside->databuffer[l]&(0xC0>>m))==(0xC0>>m) ) )
													{
														currentside->flakybitsbuffer[l]=currentside->flakybitsbuffer[l] | (0xC0>>m);
														currentside->databuffer[l]=currentside->databuffer[l] & ~(0xC0>>m);
														intrackflakeybit=l;
													}
												}
											

											}
											pCAPSUnlockTrack(img,i, j);
										}
									}

									ti.type = LIB_TYPE;
									ret=pCAPSLockTrack((struct CapsTrackInfo *)&ti, img, i, j, flag);

									free(temptrack);

								}
								else
								{	

									overlap=0;
									if(ti.overlap>0)
										overlap=ti.overlap*sizefactor;
							
									currentside->tracklen=trackcopy(currentside->databuffer,ti.trackbuf,overlap,ti.tracklen,(unsigned char)((ti.overlap<0)?0xFF:0x00));
								}
							}
							else
							{
								currentside->tracklen=12500*8;
								len=currentside->tracklen>>3;
								currentside->databuffer=malloc(len);
								memset(currentside->databuffer,0,len);
								currentside->flakybitsbuffer=malloc(len);
								memset(currentside->flakybitsbuffer,0xFF,len);
							}
							
							bitrate=((rpm/60)*currentside->tracklen)>>1;							
						
							floppycontext->hxc_printf(MSG_DEBUG,"Fixed bitrate    : %d",bitrate);
							currentside->timingbuffer=0;
							currentside->bitrate=bitrate;
							currentside->track_encoding=AMIGA_MFM_ENCODING;
							////////////////////////////////////////////////////////////////
							// get track timing
							if(ti.timebuf!=0)// && (ti.type & CTIT_FLAG_FLAKEY))
							{
								floppycontext->hxc_printf(MSG_DEBUG,"Variable bit rate!!!");
								floppycontext->hxc_printf(MSG_DEBUG,"timelen     : %d",ti.timelen);
								len=ti.timelen*sizeof(unsigned long);
								if(currentside->tracklen&7) len=len+4;
								currentside->timingbuffer=malloc(len);
								memset(currentside->timingbuffer,0,len);
								k=0;
								do
								{							
									currentside->timingbuffer[k]=((1000-ti.timebuf[k])*(bitrate/1000))+bitrate;
									k++;
								}while(k<ti.timelen);

								if(currentside->tracklen&7)
								{
									currentside->timingbuffer[currentside->tracklen>>3]=currentside->timingbuffer[ti.timelen-1];
								}

								currentside->bitrate=VARIABLEBITRATE;
							}
								
							floppycontext->hxc_printf(MSG_DEBUG,"timebuf     : %.8X",ti.timebuf);
							
							if(ti.type & CTIT_FLAG_FLAKEY) 
							{
								floppycontext->hxc_printf(MSG_DEBUG,"Track %d Side %d: CTIT_FLAG_FLAKEY",i,j);
							}

							
							if((ti.type & CTIT_MASK_TYPE) ==  ctitNoise) 
							{
								floppycontext->hxc_printf(MSG_DEBUG,"Track %d Side %d: cells are unformatted (random size)",i,j);
							}
							

							if(intrackflakeybit !=  -1) 
							{
								floppycontext->hxc_printf(MSG_DEBUG,"In track flakey bit found (last: %d)",intrackflakeybit);
							}

							pCAPSUnlockTrack(img,i, j);
							if(floppydisk->floppySectorPerTrack<ti.sectorcnt )
								floppydisk->floppySectorPerTrack=(unsigned short)ti.sectorcnt;
							
							currentside->number_of_sector=ti.sectorcnt;

							len=currentside->tracklen>>3;
							if(currentside->tracklen&0x7) len++;

							
							currentside->indexbuffer=malloc(len);
							memset(currentside->indexbuffer,0,len);
							fillindex(-2500,currentside,2500,TRUE,0);
							//fillindex(0,currentside,3200,0);

						}
						else
						{
							// error -> random track
							currentside->timingbuffer=0;
							currentside->tracklen=12500*sizefactor;
							currentside->databuffer=malloc(currentside->tracklen>>3);
							memset(currentside->databuffer,0,currentside->tracklen>>3);
							
							currentside->flakybitsbuffer=malloc(currentside->tracklen>>3);
							memset(currentside->flakybitsbuffer,0xFF,currentside->tracklen>>3);
						}
						
					}
					
				}				
				
			}
			
			pCAPSUnlockAllTracks(img);
			
		}
		
		pCAPSUnlockImage(img);
		
		
		pCAPSRemImage(img);
		
		free(fileimg);
		
		floppycontext->hxc_printf(MSG_INFO_1,"IPF Loader : tracks file successfully loaded and encoded!");
		return LOADER_NOERROR;
		
		
	}
	
	
	return LOADER_INTERNALERROR;
}
#endif
