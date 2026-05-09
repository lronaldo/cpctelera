#ifndef ADFLIB_H
#define ADFLIB_H 1

/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 *
 * adflib.h
 *
 *  $Id$
 *
 * general include file
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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* Visual C++ DLL specific, define  WIN32DLL or not in the makefile */

#ifdef WIN32DLL
#define PREFIX __declspec(dllimport)
#else
#define PREFIX 
#endif /* WIN32DLL */

#include "adf_defs.h"
#include "adf_str.h"

/* util */
PREFIX struct List* newCell(struct List* list, void* content);
PREFIX void freeList(struct List* list);

/* dir */
PREFIX RETCODE adfToRootDir(struct Volume *vol);
PREFIX RETCODE adfCreateDir(struct Volume* vol, SECTNUM parent, char* name);
PREFIX RETCODE adfChangeDir(struct Volume* vol, char *name);
PREFIX RETCODE adfParentDir(struct Volume* vol);
PREFIX RETCODE adfRemoveEntry(struct Volume *vol, SECTNUM pSect, char *name);
PREFIX struct List* adfGetDirEnt(struct Volume* vol, SECTNUM nSect );
PREFIX struct List* adfGetRDirEnt(struct Volume* vol, SECTNUM nSect, BOOL recurs );
PREFIX void printEntry(struct Entry* entry);
PREFIX void adfFreeDirList(struct List* list);
PREFIX void adfFreeEntry(struct Entry *);
PREFIX RETCODE adfRenameEntry(struct Volume *vol, SECTNUM, char *old,SECTNUM,char *pNew);	/* BV */
PREFIX RETCODE adfSetEntryAccess(struct Volume*, SECTNUM, char*, long);
PREFIX RETCODE adfSetEntryComment(struct Volume*, SECTNUM, char*, char*);

/* file */
PREFIX long adfFileRealSize(unsigned long size, int blockSize, long *dataN, long *extN);
PREFIX struct File* adfOpenFile(struct Volume *vol, char* name, char *mode);
PREFIX void adfCloseFile(struct File *file);
PREFIX long adfReadFile(struct File* file, long n, unsigned char *buffer);
PREFIX BOOL adfEndOfFile(struct File* file);
PREFIX long adfWriteFile(struct File *file, long n, unsigned char *buffer);
PREFIX void adfFlushFile(struct File *file);
PREFIX void adfFileSeek(struct File *file, unsigned long pos);

/* volume */
PREFIX RETCODE adfInstallBootBlock(struct Volume *vol,unsigned char*);
PREFIX struct Volume* adfMount( struct Device *dev, int nPart, BOOL readOnly );
PREFIX void adfUnMount(struct Volume *vol);
PREFIX void adfVolumeInfo(struct Volume *vol);

/* device */
PREFIX void adfDeviceInfo(struct Device *dev);
PREFIX struct Device* adfMountDev( char* filename,BOOL ro);
PREFIX void adfUnMountDev( struct Device* dev);
PREFIX RETCODE adfCreateHd(struct Device* dev, int n, struct Partition** partList );
PREFIX RETCODE adfCreateFlop(struct Device* dev, char* volName, int volType,struct DateTime * voldate );
PREFIX RETCODE adfCreateHdFile(struct Device* dev, char* volName, int volType);

/* dump device */
PREFIX struct Device* adfCreateDumpDevice(char* filename, long cyl, long heads, long sec);
PREFIX struct Device* adfCreateMemoryDumpDevice(long cylinders, long heads, long sectors,unsigned char ** memorybuffer,int * membufsize);

/* env */
PREFIX void adfEnvInitDefault();
PREFIX void adfEnvCleanUp();
PREFIX void adfChgEnvProp(int prop, void *pNew);											/* BV */
PREFIX char* adfGetVersionNumber();
PREFIX char* adfGetVersionDate();
/* obsolete */
PREFIX void adfSetEnvFct( void(*e)(char*), void(*w)(char*), void(*v)(char*) );

/* link */
PREFIX RETCODE adfBlockPtr2EntryName(struct Volume *, SECTNUM, SECTNUM,char **, long *);

/* salv */
PREFIX struct List* adfGetDelEnt(struct Volume *vol);
PREFIX RETCODE adfUndelEntry(struct Volume* vol, SECTNUM parent, SECTNUM nSect);
PREFIX void adfFreeDelList(struct List* list);
PREFIX RETCODE adfCheckEntry(struct Volume* vol, SECTNUM nSect, int level);

/* middle level API */

PREFIX BOOL isSectNumValid(struct Volume *vol, SECTNUM nSect);

/* low level API */

PREFIX RETCODE adfReadBlock(struct Volume* , long nSect, unsigned char* buf);
PREFIX RETCODE adfWriteBlock(struct Volume* , long nSect, unsigned char* buf);
PREFIX long adfCountFreeBlocks(struct Volume* vol);


#ifdef __cplusplus
}
#endif

#endif /* ADFLIB_H */
/*##########################################################################*/
