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
// File : dmk_loader.c
// Contains: DMK TRS-80 floppy image loader and plugins interfaces
//
// Written by:	DEL NERO Jean Francois
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "hxc_floppy_emulator.h"
#include "internal_floppy.h"
#include "floppy_loader.h"
#include "floppy_utils.h"

#include "../common/crc.h"
#include "../common/track_generator.h"

#include "dmk_loader.h"

#include "../common/os_api.h"

typedef struct dmk_header_
{
	unsigned char  write_protected;
	unsigned char  track_number;
	unsigned short track_len;
	unsigned char  flags;
	unsigned char  rsvd_1[(0xB-0x5)+1];
	unsigned char  rsvd_2[(0xF-0xC)+1];	
}dmk_header;

#define SINGLE_SIDE    0x10
#define SINGLE_DENSITY 0x40
#define IGNORE_DENSITY 0x80


int DMK_libIsValidDiskFile(HXCFLOPPYEMULATOR* floppycontext,char * imgfile)
{
	int pathlen;
	unsigned int filesize;
	char * filepath;
	dmk_header dmk_h;
	FILE *f;
	floppycontext->hxc_printf(MSG_DEBUG,"DMK_libIsValidDiskFile %s",imgfile);
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

				f=fopen(imgfile,"r+b");
				if(f)
				{
					fseek (f , 0 , SEEK_END);
					filesize=ftell(f);
					fseek (f , 0 , SEEK_SET);
					fread(&dmk_h,sizeof(dmk_header),1,f);

					fclose(f);
					

					if(dmk_h.track_len)
					{
						if(!dmk_h.track_len)
						{
							floppycontext->hxc_printf(MSG_DEBUG,"non DMK file ! bad file size !");
							free(filepath);
							return LOADER_BADFILE;
						}
						else
						{
							if(((filesize-sizeof(dmk_header))%dmk_h.track_len))
							{
								floppycontext->hxc_printf(MSG_DEBUG,"non DMK file ! bad file size !");
								free(filepath);
								return LOADER_BADFILE;
							}
						}

						if(	
								( (dmk_h.write_protected!=0xFF) && (dmk_h.write_protected!=0x00) )
							
							||
							
								( (dmk_h.rsvd_2[0]!=0x00 && dmk_h.rsvd_2[0]!=0x12) ||
								  (dmk_h.rsvd_2[1]!=0x00 && dmk_h.rsvd_2[1]!=0x34) ||
								  (dmk_h.rsvd_2[2]!=0x00 && dmk_h.rsvd_2[2]!=0x56) ||
								  (dmk_h.rsvd_2[3]!=0x00 && dmk_h.rsvd_2[3]!=0x78) )
							
							||

								(dmk_h.track_number<30 || dmk_h.track_number>90)

							||
								
								( (dmk_h.track_len>0x2940) || (dmk_h.track_len<0xB00) )

							)
						{
							floppycontext->hxc_printf(MSG_DEBUG,"non DMK file ! bad header !");
							free(filepath);
							return LOADER_BADFILE;
						}

						 

					}
					else
					{
						floppycontext->hxc_printf(MSG_DEBUG,"non DMK file ! bad file size !");
						free(filepath);					
						return LOADER_BADFILE;
					}

					floppycontext->hxc_printf(MSG_DEBUG,"DMK file !");
					free(filepath);
					return LOADER_ISVALID;


				}
				
				if(strstr( filepath,".dmk" )!=NULL)
				{
					free(filepath);
					floppycontext->hxc_printf(MSG_DEBUG,"DMK file !");
					return LOADER_ISVALID;
				}

				floppycontext->hxc_printf(MSG_DEBUG,"non DMK file !");
				free(filepath);
				return LOADER_BADFILE;

			}
		}
	}
	
	return LOADER_BADPARAMETER;
}


SIDE* DMKpatchtrack(HXCFLOPPYEMULATOR* floppycontext,unsigned char * trackdata, unsigned char * trackclk,unsigned short * idamoffset,unsigned int tracklen,unsigned long * tracktotalsize, dmk_header *dmkh)
{
	int i,j,l;
	unsigned int nbofsector,lastptr,lastdensity,tracksize,bitrate,k;
	unsigned char * track_density;
	unsigned char trackformat;
	unsigned int  final_tracklen;
	unsigned char trackstep;
	SIDE* currentside;
	track_generator tg;
	
	lastptr=0;
	bitrate=250000;
	
	trackstep=1;
	if(dmkh->flags&0xC0)
	{
		trackstep=0;
	}


	nbofsector=0;
	k=0;
	track_density=malloc(tracklen+4);

	if(idamoffset[k]&0x8000)
	{
		memset(track_density,ISOFORMAT_DD,tracklen);
		lastdensity=ISOFORMAT_DD;
		trackformat=ISOFORMAT_DD;
	}
	else
	{
		lastdensity=ISOFORMAT_SD;
		memset(track_density,ISOFORMAT_SD,tracklen);
		trackformat=ISOFORMAT_SD;
	}

	floppycontext->hxc_printf(MSG_ERROR,"----------------------------");

	while(idamoffset[k] && k<64)
	{
		if(idamoffset[k]&0x8000)
		{
			
			if((unsigned int)((idamoffset[k]&0x3FFF)-0x80-1)<tracklen)
				trackclk[((idamoffset[k]&0x3FFF)-0x80-1)]=0x0A;
			if((unsigned int)((idamoffset[k]&0x3FFF)-0x80-2)<tracklen)
				trackclk[((idamoffset[k]&0x3FFF)-0x80-2)]=0x0A;
			if((unsigned int)((idamoffset[k]&0x3FFF)-0x80-3)<tracklen)
				trackclk[((idamoffset[k]&0x3FFF)-0x80-3)]=0x0A;

			j=8;
			i=(idamoffset[k]&0x3FFF)-0x80;
			do
			{
				l=-1;
				if((trackdata[i+j]==0xA1) && (trackdata[i+j+1]==0xA1) && (trackdata[i+j+2]==0xA1) && ((trackdata[i+j+3]&0xF0)==0xF0))
				{
					l=j;
					trackclk[i+j+0]=0x0A;
					trackclk[i+j+1]=0x0A;
					trackclk[i+j+2]=0x0A;
					trackclk[i+j+3]=0xFF;
					j=64;
				}
			   j++;
			}while(j<64);

			while((lastptr<(unsigned int)((idamoffset[k]&0x3FFF)-0x80-6)) && lastptr<tracklen)
			{
				track_density[lastptr]=lastdensity;
				lastptr++;
			}
			lastdensity=ISOFORMAT_DD;
			floppycontext->hxc_printf(MSG_DEBUG,"MFM %.4X - %.4X",~0x8000&idamoffset[k] ,l);
		}
		else
		{
			floppycontext->hxc_printf(MSG_DEBUG,"FM  %.4X",idamoffset[k]);
		
			trackclk[(idamoffset[k]&0x3FFF)-0x80+0]=0xC7;
			trackclk[(idamoffset[k]&0x3FFF)-0x80+trackstep]=0xC7;

			j=8*(trackstep*2);
			i=(idamoffset[k]&0x3FFF)-0x80;
			do
			{
				if((trackdata[i+j]==0xFB && trackdata[i+j+trackstep]==0xFB)|| (trackdata[i+j]==0xF8 && trackdata[i+j+trackstep]==0xF8)|| (trackdata[i+j]==0xFA && trackdata[i+j+trackstep]==0xFA))
				{
					trackclk[i+j+0]=0xC7;
					trackclk[i+j+trackstep]=0xC7;
					j=64;
				}
		
			   j++;
			}while(j<64);
			
			while((lastptr<((unsigned int)(idamoffset[k]&0x3FFF)-0x80-6)) && lastptr<tracklen)
			{
				track_density[lastptr]=lastdensity;
				lastptr++;
			}
			lastdensity=ISOFORMAT_SD;			
		}
		
		k++;
	};


	while(lastptr<tracklen)
	{
		track_density[lastptr]=lastdensity;
		lastptr++;
	}


	final_tracklen=0;
	k=0;
	do
	{
		if(track_density[k]==ISOFORMAT_SD)
		{
			k++;
			k=k+trackstep;
			final_tracklen=final_tracklen+4;
		}
		else
		{
			k++;
			final_tracklen=final_tracklen+2;
		}
	}while(k<tracklen);

	// alloc the track...
	tg_initTrackEncoder(&tg);
	tracksize=final_tracklen*8;
	if(tracksize<((((20000) * (bitrate/4) )/(12500))))
	{
		tracksize=((((20000) * (bitrate/4) )/(12500)));
	}
	if(tracksize&0x1F) tracksize=(tracksize&(~0x1F))+0x20;

	currentside=tg_initTrack(&tg,tracksize,0,trackformat,DEFAULT_DD_BITRATE,0);
	currentside->number_of_sector=k;
	k=0;
	do
	{
		pushTrackCode(&tg,trackdata[k],trackclk[k],currentside,track_density[k]);
		if(track_density[k]==ISOFORMAT_SD)
		{
			k++;
			k=k+trackstep;
		}
		else
		{
			k++;
		}

	}while(k<tracklen);


	free(track_density);

	tg_completeTrack(&tg,currentside,trackformat);

	return currentside;
}

int DMK_libLoad_DiskFile(HXCFLOPPYEMULATOR* floppycontext,FLOPPY * floppydisk,char * imgfile,void * parameters)
{
	
	FILE * f;
	unsigned int filesize;
	unsigned int i,j;
	unsigned int file_offset;
	unsigned long tracktotalsize;
	unsigned char* trackdata;
	unsigned char* trackclk;
	unsigned short rpm;
	int numberoftrack,numberofside;
	CYLINDER* currentcylinder;
	SIDE* currentside;
	dmk_header dmk_h;
	unsigned short idam_offset_table[64];

	floppycontext->hxc_printf(MSG_DEBUG,"DMK_libLoad_DiskFile %s",imgfile);
	
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
			
		fread(&dmk_h,sizeof(dmk_h),1,f);
		floppydisk->floppyBitRate=250000;		

		numberofside=2;
		if(dmk_h.flags&SINGLE_SIDE) numberofside--;

		numberoftrack=dmk_h.track_number;
			
		floppydisk->floppyNumberOfTrack=numberoftrack;
		floppydisk->floppyNumberOfSide=numberofside;
		floppydisk->floppySectorPerTrack=-1;
		floppydisk->floppyiftype=GENERIC_SHUGART_DD_FLOPPYMODE;
		floppydisk->tracks=(CYLINDER**)malloc(sizeof(CYLINDER*)*floppydisk->floppyNumberOfTrack);
			
		rpm=300; // normal rpm
			
		floppycontext->hxc_printf(MSG_INFO_1,"filesize:%dkB, %d tracks, %d side(s), %d sectors/track, rpm:%d",filesize/1024,floppydisk->floppyNumberOfTrack,floppydisk->floppyNumberOfSide,floppydisk->floppySectorPerTrack,rpm);
				
		trackdata=(unsigned char*)malloc(dmk_h.track_len+16);
		trackclk=(unsigned char*)malloc(dmk_h.track_len+16);
			
		for(j=0;j<floppydisk->floppyNumberOfTrack;j++)
		{
				
			floppydisk->tracks[j]=allocCylinderEntry(rpm,floppydisk->floppyNumberOfSide);
			currentcylinder=floppydisk->tracks[j];
				
			for(i=0;i<floppydisk->floppyNumberOfSide;i++)
			{		
				file_offset=sizeof(dmk_header)+((dmk_h.track_len)*(j*floppydisk->floppyNumberOfSide))+((dmk_h.track_len)*(i&1));					
				fseek (f , file_offset , SEEK_SET);				
				fread(&idam_offset_table,128,1,f);
				fread(trackdata,dmk_h.track_len-128,1,f);
				memset(trackclk,0xFF,dmk_h.track_len-128);
				
				floppycontext->hxc_printf(MSG_INFO_1,"Track %d Side %d Tracklen %d TTableOffset:0x%.8x",j,i,dmk_h.track_len,file_offset);
					
				currentside=DMKpatchtrack(floppycontext,trackdata, trackclk,idam_offset_table,dmk_h.track_len-128,&tracktotalsize, &dmk_h);
				currentcylinder->sides[i]=currentside;
				fillindex(-2500,currentside,2500,TRUE,1);

			}
		}
		
		free(trackdata);
		
		floppycontext->hxc_printf(MSG_INFO_1,"track file successfully loaded and encoded!");
		
		fclose(f);
		return LOADER_NOERROR;
	}

	floppycontext->hxc_printf(MSG_ERROR,"file size=%d !?",filesize);
	fclose(f);
	return LOADER_BADFILE;
}
