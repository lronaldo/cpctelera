#ifndef ADF_FILE_H
#define ADF_FILE_H 1

/*
 *  ADF Library. (C) 1997-2002 Laurent Clevy
 *
 *  adf_file.h
 *
 *  $Id$
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

#include"prefix.h"

#include"adf_str.h"

RETCODE adfGetFileBlocks(struct Volume* vol, struct bFileHeaderBlock* entry,
    struct FileBlocks* );
RETCODE adfFreeFileBlocks(struct Volume* vol, struct bFileHeaderBlock *entry);
PREFIX long adfFileRealSize(unsigned long size, int blockSize, long *dataN, long *extN);

long adfPos2DataBlock(long pos, int blockSize, int *posInExtBlk, int *posInDataBlk, long *curDataN );

RETCODE adfWriteFileHdrBlock(struct Volume *vol, SECTNUM nSect, struct bFileHeaderBlock* fhdr);

RETCODE adfReadDataBlock(struct Volume *vol, SECTNUM nSect, void *data);
RETCODE adfWriteDataBlock(struct Volume *vol, SECTNUM nSect, void *data);
RETCODE adfReadFileExtBlock(struct Volume *vol, SECTNUM nSect, struct bFileExtBlock* fext);
RETCODE adfWriteFileExtBlock(struct Volume *vol, SECTNUM nSect, struct bFileExtBlock* fext);

PREFIX struct File* adfOpenFile(struct Volume *vol, char* name, char *mode);
PREFIX void adfCloseFile(struct File *file);
PREFIX long adfReadFile(struct File* file, long n, unsigned char *buffer);
PREFIX BOOL adfEndOfFile(struct File* file);
PREFIX void adfFileSeek(struct File *file, unsigned long pos);		/* BV */
RETCODE adfReadNextFileBlock(struct File* file);
PREFIX long adfWriteFile(struct File *file, long n, unsigned char *buffer);
SECTNUM adfCreateNextFileBlock(struct File* file);
PREFIX void adfFlushFile(struct File *file);



#endif /* ADF_FILE_H */

