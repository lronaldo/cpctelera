/*
 * ADF Library
 *
 * adf_dump.c
 *
 *  $Id$
 *
 * Amiga Dump File specific routines
 *
 *  This file is part of ADFLib.
 *
 *  ADFLib is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  ADFLib is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>

#include"adf_defs.h"
#include"adf_str.h"
#include"adf_disk.h"
#include"adf_nativ.h"
#include"adf_err.h"

extern struct Env adfEnv;

/*
 * adfInitDumpDevice
 *
 */
RETCODE adfInitDumpDevice(struct Device* dev, char* name, BOOL ro)
{
    struct nativeDevice* nDev;
    long size;

    nDev = (struct nativeDevice*)dev->nativeDev;

    nDev = (struct nativeDevice*)calloc(1,sizeof(struct nativeDevice));
    if (!nDev) {
        (*adfEnv.eFct)("adfInitDumpDevice : malloc");
        return RC_MALLOC;
    }
    dev->nativeDev = nDev;

    dev->readOnly = ro;
    errno = 0;
    if (!ro) {
        nDev->fd = fopen(name,"rb+");
        /* force read only */
        if (!nDev->fd && (errno==EACCES || errno==EROFS) ) {
            nDev->fd = fopen(name,"rb");
            dev->readOnly = TRUE;
            if (nDev->fd)
                (*adfEnv.wFct)("myInitDevice : fopen, read-only mode forced");
        }
    }
    else
        /* read only requested */
        nDev->fd = fopen(name,"rb");

    if (!nDev->fd) {
        free(nDev);
        (*adfEnv.eFct)("myInitDevice : fopen");
        return RC_ERROR;
    }

    /* determines size */
    fseek(nDev->fd, 0, SEEK_END);
	size = ftell(nDev->fd);
    fseek(nDev->fd, 0, SEEK_SET);

    dev->size = size;
	
    return RC_OK;
}


/*
 * adfReadDumpSector
 *
 */
RETCODE adfReadDumpSector(struct Device *dev, long n, int size, unsigned char* buf)
{
    struct nativeDevice* nDev;
    int r;
/*puts("adfReadDumpSector");*/
    nDev = (struct nativeDevice*)dev->nativeDev;
	
	if(nDev->fd)
	{
    r = fseek(nDev->fd, 512*n, SEEK_SET);
/*printf("nnn=%ld size=%d\n",n,size);*/
    if (r==-1)
        return RC_ERROR;
/*puts("123");*/
    if ((r=fread(buf, 1, size, nDev->fd))!=size) {
/*printf("rr=%d\n",r);*/
        return RC_ERROR;
}
/*puts("1234");*/
	}
	else
	{
		if((unsigned long)((512*n)+size)<=nDev->memory_buffer_size)
		{
			memcpy(buf,&nDev->memory_buffer[512*n],size);
		}
		else
		{
			return RC_ERROR;
		}
	}

    return RC_OK;
}


/*
 * adfWriteDumpSector
 *
 */
RETCODE adfWriteDumpSector(struct Device *dev, long n, int size, unsigned char* buf)
{
    struct nativeDevice* nDev;
    int r;

    nDev = (struct nativeDevice*)dev->nativeDev;
	if(nDev->fd)
	{
		r=fseek(nDev->fd, 512*n, SEEK_SET);
		if (r==-1)
			return RC_ERROR;

		if ( fwrite(buf, 1, size, nDev->fd)!=(unsigned int)(size) )
			return RC_ERROR;
	}
	else
	{
		if((unsigned long)((512*n)+size)<=nDev->memory_buffer_size)
		{
			memcpy(&nDev->memory_buffer[512*n],buf,size);
		}
		else
		{
			return RC_ERROR;
		}
	}
/*puts("adfWriteDumpSector");*/
    return RC_OK;
}


/*
 * adfReleaseDumpDevice
 *
 */
RETCODE adfReleaseDumpDevice(struct Device *dev)
{
    struct nativeDevice* nDev;

    if (!dev->nativeDev)
		return RC_ERROR;
	
    nDev = (struct nativeDevice*)dev->nativeDev;
    
	if(nDev->fd) fclose(nDev->fd);
	if(nDev->memory_buffer) free(nDev->memory_buffer);

    free(nDev);

    return RC_OK;
}


/*
 * adfCreateHdFile
 *
 */
RETCODE adfCreateHdFile(struct Device* dev, char* volName, int volType)
{
	
    if (dev==NULL) {
        (*adfEnv.eFct)("adfCreateHdFile : dev==NULL");
        return RC_ERROR;
    }
    dev->volList =(struct Volume**) calloc(1,sizeof(struct Volume*));
    if (!dev->volList) { 
                (*adfEnv.eFct)("adfCreateHdFile : unknown device type");
        return RC_ERROR;
    }

    dev->volList[0] = adfCreateVol( dev, 0L, (long)dev->cylinders, volName, volType,0 );
    if (dev->volList[0]==NULL) {
        free(dev->volList);
        return RC_ERROR;
    }

    dev->nVol = 1;
    dev->devType = DEVTYPE_HARDFILE;

    return RC_OK;
}


/*
 * adfCreateDumpDevice
 *
 * returns NULL if failed
 */ 
    struct Device*
adfCreateDumpDevice(char* filename, long cylinders, long heads, long sectors)
{
    struct Device* dev;
    unsigned char buf[LOGICAL_BLOCK_SIZE];
    struct nativeDevice* nDev;
/*    long i;*/
    int r;
	
    dev=(struct Device*)calloc(1,sizeof(struct Device));
    if (!dev) { 
        (*adfEnv.eFct)("adfCreateDumpDevice : malloc dev");
        return NULL;
    }
    nDev = (struct nativeDevice*)calloc(1,sizeof(struct nativeDevice));
    if (!nDev) {
        free(dev); 
        (*adfEnv.eFct)("adfCreateDumpDevice : malloc nDev");
        return NULL;
    }
    dev->nativeDev = nDev;
	
	nDev->memory_buffer=0;
    nDev->fd = (FILE*)fopen(filename,"wb");
    if (!nDev->fd) {
        free(nDev); free(dev);
        (*adfEnv.eFct)("adfCreateDumpDevice : fopen");
        return NULL;
    }

/*    for(i=0; i<cylinders*heads*sectors; i++)
        fwrite(buf, sizeof(unsigned char), 512 , nDev->fd);
*/
    r=fseek(nDev->fd, ((cylinders*heads*sectors)-1)*LOGICAL_BLOCK_SIZE, SEEK_SET);
    if (r==-1) {
        fclose(nDev->fd); free(nDev); free(dev);
        (*adfEnv.eFct)("adfCreateDumpDevice : fseek");
        return NULL;
    }

    fwrite(buf, LOGICAL_BLOCK_SIZE, 1, nDev->fd);

    fclose(nDev->fd);

    nDev->fd=(FILE*)fopen(filename,"rb+");
    if (!nDev->fd) {
        free(nDev); free(dev);
        (*adfEnv.eFct)("adfCreateDumpDevice : fopen");
        return NULL;
    }
    dev->cylinders = cylinders;
    dev->heads = heads;
    dev->sectors = sectors;
    dev->size = cylinders*heads*sectors* LOGICAL_BLOCK_SIZE;	

    if (dev->size==80*11*2*LOGICAL_BLOCK_SIZE)
        dev->devType = DEVTYPE_FLOPDD;
    else if (dev->size==80*22*2*LOGICAL_BLOCK_SIZE)
        dev->devType = DEVTYPE_FLOPHD;
	else 	
        dev->devType = DEVTYPE_HARDDISK;
		
    dev->nVol = 0;
    dev->isNativeDev = FALSE;
    dev->readOnly = FALSE;

    return(dev);
}


/*
 * adfCreateDumpDevice
 *
 * returns NULL if failed
 */ 
    struct Device*
adfCreateMemoryDumpDevice(long cylinders, long heads, long sectors,unsigned char ** memorybuffer,int * membufsize)
{
    struct Device* dev;
    struct nativeDevice* nDev;
/*    long i;*/

	
    dev=(struct Device*)calloc(1,sizeof(struct Device));
    if (!dev) { 
        (*adfEnv.eFct)("adfCreateDumpDevice : malloc dev");
        return NULL;
    }
    nDev = (struct nativeDevice*)calloc(1,sizeof(struct nativeDevice));
    if (!nDev) {
        free(dev); 
        (*adfEnv.eFct)("adfCreateDumpDevice : malloc nDev");
        return NULL;
    }
    dev->nativeDev = nDev;
	
	nDev->fd=0;
	nDev->memory_buffer=(unsigned char *) calloc(1,((cylinders*heads*sectors))*LOGICAL_BLOCK_SIZE);
    if (!nDev->memory_buffer) {
        free(nDev); free(dev);
        (*adfEnv.eFct)("adfCreateMemoryDumpDevice : malloc");
        return NULL;
    }
	nDev->memory_buffer_size=((cylinders*heads*sectors))*LOGICAL_BLOCK_SIZE;
	memset(nDev->memory_buffer,0,((cylinders*heads*sectors))*LOGICAL_BLOCK_SIZE);

    dev->cylinders = cylinders;
    dev->heads = heads;
    dev->sectors = sectors;
    dev->size = cylinders*heads*sectors* LOGICAL_BLOCK_SIZE;	

    if (dev->size==80*11*2*LOGICAL_BLOCK_SIZE)
        dev->devType = DEVTYPE_FLOPDD;
    else if (dev->size==80*22*2*LOGICAL_BLOCK_SIZE)
        dev->devType = DEVTYPE_FLOPHD;
	else 	
        dev->devType = DEVTYPE_HARDDISK;
		
    dev->nVol = 0;
    dev->isNativeDev = FALSE;
    dev->readOnly = FALSE;
	
	*memorybuffer=nDev->memory_buffer;
	*membufsize=dev->size;
    return(dev);
}
/*##################################################################################*/

