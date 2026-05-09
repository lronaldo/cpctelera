/************************************************************************
 * NAME:	svd.h
 *
 * DESCR:	Defines for the svd format.
 ************************************************************************/

typedef enum {
    WD_FM,			/* Western Digital 1791 - single density	*/
    WD_MFM,			/* Western Digital 1793 - double density	*/
    H17_HSFM,			/* Heathkit H8/H17 - hard sector single density	*/
    RX02,			/* DEC RX02 - need more details here!		*/
    BLANK_SECTOR,		/* a simple blank sector			*/
    AGCR6x2,			/* apple GCR 6x2				*/
    AGCR5x3,			/* apple GCR 5x3				*/
    RNIB,			/* apple "raw" format - 26 sectors of nib data	*/
    UNKNOWN_ENCODING_SVD		/* always keep last - just a place holder	*/
} encoding;


#define E_SVD		0x0200
#define E_SVD_UNSUP	1
#define E_SVD_UNSUPTEXT	"Feature not supported in svd format"

#define SVD_HEADERS	9	/* number of bytes of header info in each track	*/
#define SVD_CRCSTART	4	/* byte where CRC starts (starting at zero)	*/

#define HEADER_FIELDS_12	9	/* header fields in v1.2 of SVD format	*/
#define HEADER_FIELDS_TO_CRC12	7
#define HEADER_FIELDS_15	10	/* header fields in v1.5 of SVD format	*/
#define HEADER_FIELDS_TO_CRC15	8

/* the following values are used in the sector mark byte in the hardware to	*/
/*    indicate the encoding of the associated sector.				*/

#define SVD_H17_SECTOR		0x00
#define SVD_DD_SECTOR		0x01
#define SVD_SD_SECTOR		0x02
#define SVD_DDBLANK_SECTOR	0x04
#define SVD_SDBLANK_SECTOR	0x08
#define SVD_AGCR6x2_SECTOR	0x10
#define SVD_AGCR5x3_SECTOR	0x20
#define SVD_RNIB_SECTOR		0x40

/* the following values are used in the sector trailer byte in the hardware	*/
/*    to indicate what the track trailer should be				*/

#define SVD_NO_TRAILER		0x00
#define SVD_DD_TRAILER		0x01
#define SVD_SD_TRAILER		0x02
#define SVD_GCR_TRAILER		0x04

/* trailer byte counts	*/

/* note that for the SD and DD formats, the size DOESN'T include
   the trailer count byte itself...the .asm code takes this into account	*/

#define SVD_SD_TRAILER_BYTES(f)	(256 - ((10*f->sectors)+1))
#define SVD_DD_TRAILER_BYTES(f)	(SVD_SD_TRAILER_BYTES(f))

/* for the AGCR formats, the trailer count byte IS NOT included either	*/
/* see apple.h for definition of these values				*/
/* #define SVD_AGCR6x2_TRAILER_BYTES					*/
/* #define SVD_AGCR5x3_TRAILER_BYTES					*/

/* this maps the internal "floppy.h" style density value to the particular	*/
/* value that is used by the SVD to indicate the density.			*/

#define SVD_ENCODING_FLAG(d)	((d==WD_FM)?SVD_SD_SECTOR:\
                                 (d==WD_MFM)?SVD_DD_SECTOR:\
                                 (d==AGCR6x2)?SVD_AGCR6x2_SECTOR:\
                                 (d==AGCR5x3)?SVD_AGCR5x3_SECTOR:\
                                 (d==RNIB)?SVD_RNIB_SECTOR:\
                                 SVD_H17_SECTOR)

#define SVD_BLANK_FLAG(f)	((f->encoding==WD_FM)?SVD_SDBLANK_SECTOR:SVD_DDBLANK_SECTOR)

#define SVD_TRAILER_FLAG(f)	((f->encoding==WD_FM)?SVD_SD_TRAILER: \
                                 (f->encoding==WD_MFM)?SVD_DD_TRAILER: \
                                 (f->encoding==AGCR6x2 || f->encoding==AGCR5x3)?SVD_GCR_TRAILER:\
                                 SVD_NO_TRAILER)

#define SVD_TRAILER_BYTES(f)	((f->encoding==WD_FM)?SVD_SD_TRAILER_BYTES(f):\
                                 (f->encoding==WD_MFM)?SVD_DD_TRAILER_BYTES(f):\
                                 (f->encoding==AGCR6x2)?SVD_AGCR6x2_TRAILER_BYTES:\
                                 (f->encoding==AGCR5x3)?SVD_AGCR5x3_TRAILER_BYTES:0)

#define SVD_SECTOR_TYPE(d)	((d==SVD_SD_SECTOR)?WD_FM:\
                                 (d==SVD_DD_SECTOR)?WD_MFM:\
                                 (d==SVD_AGCR6x2_SECTOR)?AGCR6x2:\
                                 (d==SVD_AGCR5x3_SECTOR)?AGCR5x3:\
                                 (d==SVD_RNIB_SECTOR)?RNIB:\
                                 (d==SVD_H17_SECTOR)?H17_HSFM:\
                                 (d==SVD_DDBLANK_SECTOR)?BLANK_SECTOR:\
                                 (d==SVD_SDBLANK_SECTOR)?BLANK_SECTOR:\
                                 UNKNOWN_ENCODING)
		
/************************************************/
/* ERROR conditions				*/
/************************************************/

#define E_SVD_BAD_FILE		0x01
#define E_SVD_BAD_VERSION	0x02
#define E_SVD_BAD_FORMAT	0x03


/********************************************************/
/* function prototypes for ALL svd format versions	*/
/********************************************************/

/*int svd_read_20header(int, int *, int *, int *, int *, int *);
int svd_read_20(int, struct floppy *);
int svd_dump_20(struct floppy *, int);

int svd_read_15header(int, int *, int *);
int svd_read_15(int, struct floppy *);
int svd_dump_15(struct floppy *, int);

int svd_read_12header(int, int *, int *);
int svd_read_12(int, struct floppy *);
int svd_dump_12(struct floppy *, int);*/


