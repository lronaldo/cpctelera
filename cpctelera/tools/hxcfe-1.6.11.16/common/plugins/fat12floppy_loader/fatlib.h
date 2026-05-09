
#define sos_ui8_t  unsigned char
#define sos_ui16_t unsigned short
#define sos_ui32_t unsigned long


typedef struct FATPARTITION_
{
	int FATType;

	unsigned char * rawdata;


}FATPARTITION;
//

// fat type enum
enum {
	FATLIB_FAT12,
	FATLIB_FAT16,
	FATLIB_FAT32
};

#pragma pack(1)

/** The FAT16 specific BPB */
struct sos_fat16_BPB {
    sos_ui8_t     BS_DrvNum;
    sos_ui8_t       BS_Reserved1;
    sos_ui8_t       BS_BootSig;
    sos_ui32_t      BS_VolID;
    char     BS_VolLab[11];
    char            BS_FilSysType[8];
};


/** The FAT32 specific BPB */
struct sos_fat32_BPB {
    sos_ui32_t     BPB_FATSz32;
    sos_ui16_t     BPB_ExtFlags;
    sos_ui16_t     BPB_FSVer;
    sos_ui32_t     BPB_RootClus;
    sos_ui16_t     BPB_FSInfo;
    sos_ui16_t     BPB_BkBootSec;
    char     BPB_Reserved[12];
    struct sos_fat16_BPB fat16_BPB;
};


/** The FAT first sector structure */
typedef struct fat_boot_sector_ {
    /* BS (Boot Sector) */
    sos_ui8_t     BS_jmpbBoot[3];
    char     BS_OEMName[8];
    /* BPB (BIOS Parameter Block) */
    sos_ui16_t     BPB_BytsPerSec;
    sos_ui8_t     BPB_SecPerClus;
    sos_ui16_t     BPB_RsvdSecCnt;
    sos_ui8_t     BPB_NumFATs;
    sos_ui16_t     BPB_RootEntCnt;
    sos_ui16_t     BPB_TotSec16;
    sos_ui8_t     BPB_Media;
    sos_ui16_t     BPB_FATSz16;
    sos_ui16_t     BPB_SecPerTrk;
    sos_ui16_t     BPB_NumHeads;
    sos_ui32_t     BPB_HiddSec;
    sos_ui32_t     BPB_TotSec32;
    /* BPB specific */
    union {
		struct sos_fat16_BPB fat16_BPB;
		struct sos_fat32_BPB fat32_BPB;
    } BPB_specific;
}fat_boot_sector;



#define ATTR_READ_ONLY 0x01
#define ATTR_HIDDEN 0x02
#define ATTR_SYSTEM 0x04
#define ATTR_VOLUME_ID 0x08
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20
#define ATTR_LONG_NAME \
    (ATTR_READ_ONLY | ATTR_HIDDEN | \
    ATTR_SYSTEM | ATTR_VOLUME_ID)

#define IS_FAT_VALID_CHAR(c) \
    ((c) > 0x20 && (c) != 0x22 && \
    ((c) < 0x2A || (c) > 0x2C) && \
    (c) != 0x2E && (c) != 0x2F && \
    ((c) < 0x3A || (c) > 0x3F) && \
    ((c) < 0x5B || (c) > 0x5D) && \
    (c) != 0x7C)

/** Fat Directory entry structure */
typedef  struct fat_directory_entry_ {

    char DIR_Name[11];

    /* File attributes */

    sos_ui8_t DIR_Attr;
    sos_ui8_t DIR_NTRes;
    sos_ui8_t DIR_CrtTimeTenth;
    sos_ui16_t DIR_CrtTime;
    sos_ui16_t DIR_CrtDate;
    sos_ui16_t DIR_LstAccDate;
    sos_ui16_t DIR_FstClusHI;
    sos_ui16_t DIR_WrtTime;
    sos_ui16_t DIR_WrtDate;
    sos_ui16_t DIR_FstClusLO;
    sos_ui32_t  DIR_FileSize;
} fat_directory_entry;


#pragma pack()
