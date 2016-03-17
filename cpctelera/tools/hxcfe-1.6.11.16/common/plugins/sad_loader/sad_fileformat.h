#define SAD_SIGNATURE          "Aley's disk backup"

// Format of a SAD image header (22 bytes)
typedef struct
{
    unsigned char abSignature[sizeof SAD_SIGNATURE - 1];

    unsigned char  bSides;             // Number of sides on the disk
    unsigned char  bTracks;            // Number of tracks per side
    unsigned char  bSectors;           // Number of sectors per track
    unsigned char  bSectorSizeDiv64;   // Sector size divided by 64
}
SAD_HEADER;

