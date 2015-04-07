/*
  bug3475630.c

  Register allocator allocated variable to iy, which is needed for stack access when accessing local variables unreachable by the frame pointer.
*/

#include <testfwk.h>

#include <stdio.h>

/*
 * Raw directory item (32 bytes).
 *
*/
typedef struct {
    unsigned char   ftype;
    unsigned char   fname[16];
    unsigned char   str_terminator; // 0x0d
    unsigned char   unused1[2];
    unsigned int    fsize;
    unsigned int    fstrt;
    unsigned int    fexec;
    unsigned char   unused2[4];
    unsigned int    block;
} FS_MZ_DITEM;


/*
 * Raw directory sektor.
 *
 * Has 8 directory items (256 bytes).
*/
typedef struct {
    FS_MZ_DITEM     ditem[8];
} FS_MZ_BLDIR;


/*
 * Elementary file descriptor
 *
*/
typedef struct {
    unsigned char   ftype;
    unsigned char   fname[17];
    unsigned int    fsize;
    unsigned int    fstrt;
    unsigned int    fexec;
    unsigned int    block;    
} FS_MZ_FHEADER;


/*
 * Directory handler with cache for 8 file descriptors.
 *
*/
typedef struct {
    unsigned char   position;
    FS_MZ_FHEADER   fheader[8];
} FS_MZDIR;


#pragma disable_warning 85 // Unreferenced function arguments in fd_read_sector(), my_strncpy().
extern char fd_read_sector ( unsigned int block, void* dma )
{
    unsigned char i;
    FS_MZ_BLDIR *bl_dir = dma;

    for(i = 0; i < 8; i++)
    {
        bl_dir->ditem[i].ftype = i + 0;
        bl_dir->ditem[i].fsize = i + 1;
        bl_dir->ditem[i].fstrt = i + 2;
        bl_dir->ditem[i].fexec = i + 3;
        bl_dir->ditem[i].block = i + 4;
    }

    return(0);
}

char* my_strncpy ( char *dst, const char *src, size_t n, char terminator )
{
    return(0);
}

/*
 * Read raw directory data and transform into FS_MZDIR *dir.
 *
 */
char fs_read_directory_block ( FS_MZDIR *dir, unsigned int block )
{
    char            res;
#if defined(__SDCC_mcs51)
    static __xdata FS_MZ_BLDIR     bl_dir;
#else
    FS_MZ_BLDIR     bl_dir;
#endif
    FS_MZ_DITEM     *dir_item;
    FS_MZ_FHEADER   *fil_header;
    unsigned char   i;

    res = fd_read_sector ( block, &bl_dir );
    if ( res ) return ( res );

    for ( i = 0; i < 8; i++ ) {

        dir_item = &bl_dir.ditem[i];
        fil_header = &dir->fheader[i];

        fil_header->ftype = dir_item->ftype;
        fil_header->fsize = dir_item->fsize;
        fil_header->fstrt = dir_item->fstrt;
        fil_header->fexec = dir_item->fexec;
        fil_header->block = dir_item->block;
        
        my_strncpy ( fil_header->fname, dir_item->fname, sizeof ( fil_header->fname ), 0x0d );
    };

    return ( 0 );
}

__xdata FS_MZDIR dir;

void testBug(void)
{
    unsigned char i;

    fs_read_directory_block(&dir, 0);
    
    for(i = 0; i < 8; i++)
    {
        ASSERT(dir.fheader[i].ftype == i + 0);
        ASSERT(dir.fheader[i].fsize == i + 1);
        ASSERT(dir.fheader[i].fstrt == i + 2);
        ASSERT(dir.fheader[i].fexec == i + 3);
        ASSERT(dir.fheader[i].block == i + 4);
    }
}
