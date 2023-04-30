#if !defined(AFX_ADFWRAPPER_H__20001226_7276_AC4A_215B_0080AD509054__INCLUDED_)
#define AFX_ADFWRAPPER_H__20001226_7276_AC4A_215B_0080AD509054__INCLUDED_

#pragma once

//
// These classes are C++ wrappers for the ADFlib (Amiga Disk File library).
// ADFlib is developed by Laurent Clévy.
//
// They do not extend the functionality of ADFlib in any way,
// except that they give me: nice C++ classes, dtor cleanup,
// more const-ness, Windows datatypes, MBCS and UNICODE compilation.
//

#ifndef __ATLBASE_H__
  #error adfwrappers.h requires atlbase.h to be included first
#endif

#ifdef _DEBUG
#pragma comment(lib, "adflib\\libs\\adfdbg.lib")
#else
#pragma comment(lib, "adflib\\libs\\adf.lib")
#endif

#include "adflib\include\adflib.h"
extern "C" {
   #include "adflib\include\adf_util.h"
   #include "adflib\include\adf_dir.h"
   #include "adflib\include\Win32\adf_nativ.h"
}
extern "C" struct Env adfEnv;

class CAdfDevice;
class CAdfVolume;
class CAdfFile;
class CAdfDirList;
class CAdfDelList;


class CAdf
{
public:
   CAdf()
   {
      // System initialize...
      adfEnvInitDefault();
      // Use the dir cache blocks
      BOOL boolPr = TRUE;
      adfChgEnvProp(PR_USEDIRC, (void*)&boolPr);   
      // Need to set callbacks to prevent Console output
      adfSetEnvFct(ADFError, ADFWarning, ADFVerbose);
      adfEnv.rwhAccess = ADFAccess;
      adfEnv.progressBar = ADFProgress;
      adfEnv.useRWAccess = TRUE;
      adfEnv.useProgressBar = TRUE;
      adfEnv.useDirCache = FALSE;
   }
   virtual ~CAdf()
   {
      // Shutdown ADF system
      adfEnvCleanUp();
   }

   LPCSTR GetVersionNumber() { return adfGetVersionNumber(); };

   LPCSTR GetVersionDate() { return adfGetVersionDate(); };

   BOOL IsSilent() const { return m_bSilent; };

   void SetSilent(BOOL bSilent) { m_bSilent = bSilent; };

// Attributes
private:
   static BOOL m_bSilent; // Supress error/warnings?
                          // Needs to be static because it's used by the callbacks.

// Callbacks
private:
   static VOID ADFError(char *strMessage)
   {
      USES_CONVERSION;
      if( !m_bSilent ) ::MessageBox(NULL, A2CT(strMessage), NULL, MB_OK | MB_ICONERROR);
   }

   static VOID ADFWarning(char *strMessage)
   {
      strMessage;
#ifdef _DEBUG
      USES_CONVERSION;
      if( !m_bSilent ) ::MessageBox(NULL, A2CT(strMessage), _T("Warning"), MB_OK | MB_ICONWARNING);
#endif
   }

   static VOID ADFVerbose(char *strMessage)
   {
      strMessage;
#ifdef _DEBUG
      USES_CONVERSION;
      if( !m_bSilent ) ::MessageBox(NULL, A2CT(strMessage), _T("Verbose Warning"), MB_OK | MB_ICONINFORMATION);
#endif
   }

   static VOID ADFAccess(SECTNUM physical, SECTNUM logical, BOOL write)
   {
      physical;
      logical;
      write;
   }

   static VOID ADFProgress(int perCentDone)
   {
      perCentDone;
   }

};

__declspec(selectany) BOOL CAdf::m_bSilent = FALSE;

class CAdfFile
{
friend CAdfVolume;
protected:
   File *m_file;

public:
   CAdfFile()
   {
      m_file = NULL;
   }
   virtual ~CAdfFile()
   {
      if( m_file!=NULL ) adfCloseFile(m_file);
   }

   BOOL IsOpen() const { return m_file!=NULL; };

   UINT GetType() const { return m_file->fileHdr->secType; };
   
   BOOL IsDirectory() const { return _IsDirectory(GetType()); };
   
   BOOL IsLink() const { return _IsLink(GetType()); };

   ULONG Read(LPVOID pv, ULONG cb)
   {
      return (ULONG)adfReadFile(m_file, cb, (unsigned char *)pv);
   }

   ULONG Write(LPCVOID pv, ULONG cb)
   {
      return (ULONG)adfWriteFile(m_file, cb, (unsigned char *)pv);
   }

   BOOL Eof()
   {
      return adfEndOfFile(m_file);
   }

   BOOL Seek(DWORD dwPos)
   {
      adfFileSeek(m_file, dwPos);
      return TRUE; // Erh?
   }

   DWORD GetPos() const
   {
      return m_file->pos;
   }

   DWORD GetAccess() const
   {
      return m_file->fileHdr->access;
   }

   static void ConvertAccess(DWORD dwAccess, LPTSTR pstrAccess)
   {
      USES_CONVERSION;
      _tcscpy( pstrAccess, A2CT(adfAccess2String(dwAccess)) );
   }

   FILETIME GetFileTime()
   {
      int y,m,d;
      adfDays2Date(m_file->fileHdr->days, &y, &m, &d);
      SYSTEMTIME st;
      st.wYear = (WORD)y;
      st.wMonth = (WORD)m;
      st.wDay = (WORD)d;
      st.wHour = (WORD)((m_file->fileHdr->mins / 60)+1);
      st.wMinute = (WORD)((m_file->fileHdr->mins % 60)+1);
      st.wSecond = (WORD)((m_file->fileHdr->ticks / 50));
      FILETIME ft;
      ::SystemTimeToFileTime(&st, &ft);
      FILETIME ftLocal;
      ::FileTimeToLocalFileTime(&ft, &ftLocal);
      return ftLocal;
   }

   void Flush()
   {
      adfFlushFile(m_file);
   }

   void Close()
   {
      adfCloseFile(m_file);
      m_file = NULL;
   }

   void GetName(LPTSTR pstrName, UINT cbChars)
   {
      USES_CONVERSION;
      cbChars--;
      _tcsncpy( pstrName, m_file->fileHdr->fileName==NULL ? _T("") : A2CT(m_file->fileHdr->fileName), cbChars);
      if( m_file->fileHdr->nameLen < (int)cbChars ) cbChars = (UINT)m_file->fileHdr->nameLen;
      pstrName[cbChars] = _T('\0');
   }

   DWORD GetSize() const
   {
      return m_file->fileHdr->byteSize;
   }

   DWORD GetActualSize(DWORD dwDataBlockSize=LOGICAL_BLOCK_SIZE)
   {
      return _CalcBlocksNeeded(GetSize(), dwDataBlockSize);
   }

   static DWORD _CalcBlocksNeeded(DWORD dwFileSize, DWORD dwDataBlockSize=LOGICAL_BLOCK_SIZE)
   {
      return (DWORD)adfFileRealSize(dwFileSize, dwDataBlockSize, NULL, NULL);
   }

   static BOOL _IsDirectory(DWORD dwAccess)
   {
      return dwAccess==ST_DIR || dwAccess==ST_LDIR;
   }

   static BOOL _IsLink(DWORD dwAccess)
   {
      return dwAccess==ST_LDIR || dwAccess==ST_LFILE || dwAccess==ST_LSOFT;
   }
};

class CAdfDirList
{
friend CAdfVolume;
protected:
   List *m_list;

public:
   CAdfDirList()
   {
      m_list = NULL;
   }
   virtual ~CAdfDirList()
   {
      if( m_list!=NULL ) adfFreeDirList(m_list);
   }
   operator List *() const
   {
      return m_list;
   };
};

class CAdfDelList
{
friend CAdfVolume;
protected:
   List *m_list;

public:
   CAdfDelList()
   {
      m_list = NULL;
   }
   virtual ~CAdfDelList()
   {
      if( m_list!=NULL ) adfFreeDelList(m_list);
   }
   operator List *() const
   {
      return m_list;
   };
};

class CAdfVolume
{
friend CAdfDevice;

protected:
   Volume *m_vol;

public:
   CAdfVolume()
   {
      m_vol = NULL;
   }
   virtual ~CAdfVolume()
   {
      if( m_vol!=NULL ) adfUnMount(m_vol);
   }

   BOOL IsOpen() const { return m_vol!=NULL; };

   void GetName(LPTSTR pstrName, UINT cbChars)
   {
      USES_CONVERSION;
      cbChars--;
      _tcsncpy( pstrName, m_vol->volName==NULL ? _T("") : A2CT(m_vol->volName), cbChars);
      pstrName[cbChars] = _T('\0');
   }
   
   SECTNUM GetDirectoryPtr() const { return m_vol->curDirPtr; };
   
   SECTNUM GetFirstBlock() const { return m_vol->firstBlock; };
   
   SECTNUM GetLastBlock() const { return m_vol->lastBlock; };
   
   SECTNUM GetRootBlock() const { return m_vol->rootBlock; };
   
   DWORD GetDataBlockSize() const { return m_vol->datablockSize; };
   
   DWORD GetFreeBlockCount() { return adfCountFreeBlocks(m_vol); };
   
   BOOL IsBootable() const { return m_vol->bootCode; };
   
   BOOL IsReadOnly() const { return m_vol->readOnly; };
   
   int GetDOSType() const { return (int)(m_vol->dosType); };

   BOOL GetFile(LPCTSTR pstrFileName, LPCTSTR pstrAccess, CAdfFile &AmigaFile)
   {
      USES_CONVERSION;
      
      File * file = adfOpenFile(m_vol, 
         T2A(const_cast<LPTSTR>(pstrFileName)), 
         T2A(const_cast<LPTSTR>(pstrAccess)));
      if( file==NULL ) return FALSE;
      AmigaFile.m_file = file;
      return TRUE;
   }

   BOOL CreateDirectory(SECTNUM nSector, LPCTSTR pstrName)
   {
      USES_CONVERSION;
      return adfCreateDir(m_vol, nSector, T2A(const_cast<LPTSTR>(pstrName)))==RC_OK;
   }

   BOOL GetDirctory(SECTNUM nSector, CAdfDirList &DirList)
   {
      List *dir = adfGetDirEnt(m_vol, nSector);
      if( dir==NULL ) return TRUE;   // Empty?
      DirList.m_list = dir;
      return TRUE;
   }
   
   BOOL GetCurrentDirctory(CAdfDirList &DirList)
   {
      return GetDirctory(GetDirectoryPtr(), DirList);
   }

   BOOL ChangeDirectoryParent()
   {
      return( adfParentDir(m_vol)==RC_OK );
   }

   BOOL ChangeDirectory(LPCTSTR pstrPath)
   {
      USES_CONVERSION;
      if( *pstrPath==_T('\0') ) return TRUE;
      // In the case where only one subdir is requested we just pass
      // the value.  This is the only type ADFlib supports.
      if( _tcschr(pstrPath, _T('/'))==NULL ) {
         return adfChangeDir(m_vol, T2A(const_cast<LPTSTR>(pstrPath)))==RC_OK;
      }
      // If a complete path is given, we need to do the parsing
      // ourselves! So start by going to root dir..
      if( *pstrPath == _T('/') ) {
         pstrPath++;
         adfToRootDir(m_vol);
      }
      // Otherwise parse remaining sub-dirs
      LPTSTR p;
      while( (p = _tcschr(pstrPath, _T('/')))!=NULL ) {
         TCHAR szPath[MAXNAMELEN+1];
         int size = p - pstrPath;
         _tcsncpy( szPath, pstrPath, size);
         szPath[ size ] = _T('\0');
         if( adfChangeDir(m_vol, T2A(szPath))!=RC_OK ) return FALSE;
         pstrPath = p+1;
      }
      if( *pstrPath!=_T('\0') ) {
         return adfChangeDir(m_vol, T2A(const_cast<LPTSTR>(pstrPath)))==RC_OK;
      }
      return TRUE;
   }

   BOOL GetDeletedEntries(CAdfDelList &DelList)
   {
      List *dir = adfGetDelEnt(m_vol);
      if( dir==NULL ) return TRUE;   // Empty?
      DelList.m_list = dir;
      return TRUE;
   }

   BOOL CheckDeletedFile(SECTNUM nSector, int nLevel=0)
   {
      return adfCheckEntry(m_vol, nSector, nLevel)==RC_OK;
   }

   BOOL UndeleteFile(SECTNUM nParentSector, SECTNUM nSector)
   {
      return adfUndelEntry(m_vol, nParentSector, nSector)==RC_OK;
   }

   BOOL SetFileAccess(LPCTSTR pstrName, SECTNUM nSector, DWORD dwAccess)
   {
      USES_CONVERSION;
      return adfSetEntryAccess(m_vol,  nSector, T2A(const_cast<LPTSTR>(pstrName)), (long)dwAccess)==RC_OK;
   }

   BOOL SetFileComment(LPCTSTR pstrName, SECTNUM nSector, LPCTSTR pstrComment)
   {
      USES_CONVERSION;
      return adfSetEntryComment(m_vol, nSector, T2A(const_cast<LPTSTR>(pstrName)), T2A(const_cast<LPTSTR>(pstrComment)))==RC_OK;
   }

   BOOL Rename(SECTNUM nOldSector, LPCTSTR pstrOldName,
               SECTNUM nNewSector, LPCTSTR pstrNewName)
   {
      USES_CONVERSION;
      return adfRenameEntry(m_vol, 
                            nOldSector, T2A(const_cast<LPTSTR>(pstrOldName)), 
                            nNewSector, T2A(const_cast<LPTSTR>(pstrNewName)))==RC_OK;
   }

   BOOL Delete(SECTNUM nParentSector, LPCTSTR pstrName)
   {
      USES_CONVERSION;
      return adfRemoveEntry(m_vol, nParentSector, T2A(const_cast<LPTSTR>(pstrName)))==RC_OK;
   }
};


class CAdfDevice
{
protected:
   Device *m_dev;

public:
   CAdfDevice()
   {
      m_dev = NULL;
   }
   virtual ~CAdfDevice()
   {
      if( m_dev!=NULL ) adfUnMountDev(m_dev);
   }

   static BOOL _IsNativeDevice(LPCTSTR pstrDeviceName)
   {
      USES_CONVERSION;
      nativeFunctions *nFct = (struct nativeFunctions *)adfEnv.nativeFct;
      return (*nFct->adfIsDevNative)(T2A(const_cast<LPTSTR>(pstrDeviceName)));
   }

   BOOL IsOpen() const { return m_dev!=NULL; };

   BOOL Open(LPCTSTR pstrFileName, BOOL bReadOnly=FALSE)
   {
      USES_CONVERSION;
      m_dev = adfMountDev(T2A(const_cast<LPTSTR>(pstrFileName)), bReadOnly);
      if( m_dev==NULL ) return FALSE;
      switch( m_dev->devType ) {
      case DEVTYPE_FLOPDD:
      case DEVTYPE_FLOPHD:
      case DEVTYPE_HARDDISK:
      case DEVTYPE_HARDFILE:
         break;
      case -1:
         // Invalid device read! ADFlib will crash on adfUnMountDev()
         // BUG: Causes memory-leak.
         m_dev = NULL;
         return FALSE;
      }
      return TRUE;
   }
   
   BOOL Close()
   {
      if( m_dev==NULL ) return TRUE;
      adfUnMountDev(m_dev);
      return TRUE;
   }

   UINT GetType() const { return m_dev->devType; };
   
   BOOL IsReadOnly() const { return m_dev->readOnly; };
   
   DWORD GetDiskSize() const { return m_dev->size; };
   
   DWORD GetPartitionCount() const { return m_dev->nVol; };

   BOOL Mount(DWORD nPartition, BOOL bReadOnly, CAdfVolume &volume)
   {
      Volume *vol = adfMount(m_dev, (int)nPartition, bReadOnly);
      if( vol==NULL ) return FALSE;      
      volume.m_vol = vol;
      return TRUE;
   }
};


#endif // !defined(AFX_ADFWRAPPER_H__20001226_7276_AC4A_215B_0080AD509054__INCLUDED_)
