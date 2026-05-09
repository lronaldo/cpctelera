#define TD0_SIG_NORMAL          "TD"    // Normal compression (RLE)
#define TD0_SIG_ADVANCED        "td"    // Huffman compression also used for everything after TD0_HEADER

typedef struct _TELEDISK_HEADER
{
 unsigned char TXT[2];
 unsigned char SeqVal; // Volume sequence (zero for the first)
 unsigned char ChkSig; // Check signature for multi-volume sets (all must match)
 unsigned char TDVer;  // Teledisk version used to create the file (11 = v1.1)
 unsigned char Dens;   // Source disk density (0 = 250K bps,  1 = 300K bps,  2 = 500K bps ; +128 = single-density FM)
 unsigned char DrvType;// Source drive type (1 = 360K, 2 = 1.2M, 3 = 720K, 4 = 1.44M)
 unsigned char TrkDens;// 0 = source matches media density, 1 = double density, 2 = quad density)
 unsigned char DosMode;// Non-zero if disk was analysed according to DOS allocation
 unsigned char Surface;// Disk sides stored in the image
 unsigned short CRC;   // 16-bit CRC for this header
} TELEDISK_HEADER;



// Optional comment block, present if bit 7 is set in bTrackDensity above
typedef struct _TELEDISK_COMMENT
{
    unsigned short CRC;  // 16-bit CRC covering the comment block
    unsigned short Len;  // Comment block length
    unsigned char  bYear, bMon, bDay;  // Date of disk creation
    unsigned char  bHour, bMin, bSec;  // Time of disk creation
//  BYTE    abData[];           // Comment data, in null-terminated blocks
}TELEDISK_COMMENT;

typedef struct _TELEDISK_TRACK_HEADER
{
 unsigned char SecPerTrk;	// Number of sectors in track
 unsigned char PhysCyl;		// Physical track we read from
 unsigned char PhysSide;	// Physical side we read from
 unsigned char CRC;			// Low 8-bits of track header CRC
} TELEDISK_TRACK_HEADER;

typedef struct _TELEDISK_SECTOR_HEADER
{
 unsigned char Cyl;			// Track number in ID field
 unsigned char Side;		// Side number in ID field
 unsigned char SNum;		// Sector number in ID field
 unsigned char SLen;		// Sector size indicator:  (128 << bSize) gives the real size
 unsigned char Syndrome;	// Flags detailing special sector conditions
 unsigned char CRC;			// Low 8-bits of sector header CRC
}TELEDISK_SECTOR_HEADER;

