#ifndef _DSKGEN_TYPES_H_
#define _DSKGEN_TYPES_H_

#include <string>
#include <algorithm>
#include <sstream>
#include <cstdint>

using namespace std;

#define        DSK_RECORD_SIZE            128

#define        AMSDOS_EMPTY_BYTE        0xE5

typedef enum { HDR_NONE=0, HDR_AMSDOS=1 } HeaderType;

// RAW catalog: it stores in the first sector the following info for each file:
//       Side (1byte), 
//    Initial Track (1 byte) - track where the file starts, 
//    Initial Sector Offset (1 byte) - counting from the first sector based on the disk type, 
//    Length in bytes (3 bytes)
typedef enum { CAT_NONE=0, CAT_RAW=1, CAT_CPM=2 } CatalogType;

typedef enum { DSK_SYSTEM=0, DSK_DATA=1, DSK_IBM=2, DSK_CUSTOM=3 } DiskType;

typedef enum { AMSDOS_FILE_NONE = 0, AMSDOS_FILE_INTERNAL_BASIC = 1, AMSDOS_FILE_BINARY = 2, AMSDOS_FILE_BINARY_PROTECTED = 3, AMSDOS_FILE_SCREEN_IMAGE = 4, AMSDOS_FILE_ASCII = 8 } AmsdosFileType;

CatalogType ParseCatalogType(const string &catStr);
DiskType ParseDiskType(const string &diskStr);
HeaderType ParseHeaderType(const string &hdrStr);
AmsdosFileType ParseAmsdosFileType(const string &fileTypeStr);

#ifndef u8
typedef uint8_t u8;
#endif
#ifndef u16
typedef uint16_t u16;
#endif
#ifndef u32
typedef uint32_t u32;
#endif

#pragma pack(push,1)

struct u24 {
    u16 low;
    u8  hi;
};

struct XDPB {
    u16                recordsPerTrack;    // (spt) Number of 128-byte records on each track;
    u8                 blockShift;            // (bsh) log2 BLS - 7
    u8                 blockMask;            // (blm) BLS / 128 - 1
    u8                 extentMask;            // (exm) Extent mask. DSM < 256 ? BLS/1024 - 1 : BLS/2048 - 1 
    u16                numBlocks;            // (dsm) Total size of disk in blocks excluding reserved tracks.
    u16                dirEntries;            // (drm) Total number of directory entries - 1
    u8                 allocationLo;        // (al01) Bit significant representation of number of directory blocks (#0080 => 1, #00C0 => 2)
    u8                 allocationHi;        // (al01) Bit significant representation of number of directory blocks (#0080 => 1, #00C0 => 2)
    u16                checksumLength;        // (cks) Length of checksum vector. Normally drm/4 + 1, but if checksumming not required, 0
    u16                reservedTracks;        // (off) Number of reserved tracks. This is also the track the directory starts.
    u8                 firstSectorNumber;    // First sector number.
    u8                 sectorsPerTrack;    // Sectors per track.
    u8                 gapRW;                // Gap length (read/write)
    u8                 gapF;                // Gap length (format)
    u8                 fillerByte;            // Filler Byte (for formatting)
    u8                 logSectSize;         // Log2(sectorSize)-7
    u8                 sectSizeInRecords;    // Sector size in records
};

/* DSK Format: http://cpctech.cpc-live.com/docs/dsk.html */
/* See the description of the Extended DSK format at http://cpctech.cpc-live.com/docs/extdsk.html */
struct DskHeader {
    u8                  Id[34];          // "EXTENDED CPC DSK File\r\nDisk-Info\r\n"
    u8                  Creator[14];     // "DSKGEN-C      "
    u8                  Tracks;
    u8                  Sides;
    u16                 Unused;          // 0x1300 = 256 + ( 512 * Sector Count )
    u8                  TrackSizes[204]; // TrackSize is (FileSize + sizeof(DskTrack)) / 256.
                                      // If the disk is two sided, the order is Track 0 Side 0, Track 0 Side 1, Track 1 Side 0... 
                                      // (alternating sides).
};

struct DskSector {
    u8           Track;                // C
    u8           Side;                    // H
    u8           Sector;                // R
    u8           Size;                    // N
    u8           FDCStatusReg1;        // ST1
    u8           FDCStatusReg2;        // ST2
    u16          SizeInBytes;
    u8*          Data;
};

#define DSK_SECTORS_IN_TRACK_HEADER 29

struct DskTrack {
    u8               Id[13];         // "Track-Info\r\n"
    u8               IdPadding[3];     // Not used
    u8               Track;
    u8               Side;
    u8               Padding[2];
    u8               SectSize;       // 2
    u8               SectCount;      // 9
    u8               Gap3;           // 0x4E
    u8               FillerByte;     // 0xE5
    struct DskSector Sectors[DSK_SECTORS_IN_TRACK_HEADER];
};

struct CatalogEntryAmsdos {
    u8        UserNumber;     // Valid values are 0-15
    u8        Name[8];
    u8        Extension[3];
    // File size: 
    u8        ExtentLoByte;   // Valid values are 0-31
    u8        ExtentPadding;
    u8        ExtentHiByte;   // Extent number is 32*ExtentHiByte + ExtentLoByte
    u8        Records;        // Number of records used in this extent. 1 record = 128 bytes.
    u8        Blocks[ 16 ];
};

struct CatalogEntryRaw {
    u8        EmptyByte;            // This byte always set to E5, so that CAT shows empty disc.
    u8        Side;
    u8        InitialTrack;
    u8        InitialSectorOffset;
    u16       LengthInBytes;
    u8        Padding[2];
};

struct AmsdosHeader {
    u8         UserNumber;           // 00    User number
    u8         FileName[11];         // 01-11 Name [01-08] Extension [09-11] 
    u8         NamePadding[4];       // 12-15
    u8         BlockNumber;          // 16    Block number (only for disks)
    u8         LastBlock;            // 17    Flag "Last block" (disk)
    u8         FileType;             // 18    File Type
    u16        Length;               // 19-20 Data length
    u16        LoadAddress;          // 21-22 Data address (where the file is loaded)
    u8         FirstBlock;           // 23    Flag "First block of file" (disk)
    u16        LogicalLength;        // 24-25 Logical length
    u16        EntryAddress;          // 26-27 Entrypoint address (location to run after loading)
    u8         Padding[0x24];        // 28-63
    struct u24 RealLength;           // 64-66 Length of file
    u16        CheckSum;             // 67-68 CheckSum
    u8         FinalPadding[ 0x3B ]; // 69-127 
};

#pragma pack(pop)

#endif