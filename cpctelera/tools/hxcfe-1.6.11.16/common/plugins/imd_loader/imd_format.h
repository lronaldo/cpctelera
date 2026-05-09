// IMD File format :
/*
                                   ImageDisk
                                       A
                                Diskette Imaging
                                    program
                                      for
                                  Soft-Sector
                                    Formats

                                  Version 1.17
                              Revised 27-Jun-2008
                       Copyright 2005-2008 Dave Dunfield
                              All rights reserved.

    6. IMAGE FILE FORMAT

       The overall layout of an ImageDisk .IMD image file is:

         IMD v.vv: dd/mm/yyyy hh:mm:ss          (ASCII Header)
         Comment (ASCII only - unlimited size)  (NOTE:  You can TYPE a .IMD)
         1A byte - ASCII EOF character          (file to see header/comment)
         - For each track on the disk:
            1 byte  Mode value                  (0-5)
            1 byte  Cylinder                    (0-n)
            1 byte  Head                        (0-1)   (see Note)
            1 byte  number of sectors in track  (1-n)
            1 byte  sector size                 (0-6)
            sector numbering map                * number of sectors
            sector cylinder map (optional)      * number of sectors
            sector head map     (optional)      * number of sectors
            sector data records                 * number of sectors
         <End of file>

       6.1 Mode value

          This value indicates the data transfer rate and density  in  which
          the original track was recorded:

             00 = 500 kbps FM   \   Note:   kbps indicates transfer rate,
             01 = 300 kbps FM    >          not the data rate, which is
             02 = 250 kbps FM   /           1/2 for FM encoding.
             03 = 500 kbps MFM
             04 = 300 kbps MFM
             05 = 250 kbps MFM

       6.2 Sector size

          The Sector Size value indicates the actual size of the sector data
          occuring on the track:

            00 =  128 bytes/sector
            01 =  256 bytes/sector
            02 =  512 bytes/sector
            03 = 1024 bytes/sector
            04 = 2048 bytes/sector
            05 = 4096 bytes/sector
            06 = 8192 bytes/sector

       6.3 Head value

          This value indicates the side of the  disk  on  which  this  track
          occurs (0 or 1).

          Since HEAD can only be 0 or 1,  ImageDisk uses the upper  bits  of
          this byte to indicate the presense of optional items in the  track
          data:

             Bit 7 (0x80) = Sector Cylinder Map
             Bit 6 (0x40) = Sector Head     Map
     ImageDisk                                                        Page: 26


       6.4 Sector numbering map

          The sector numbering map contains one byte  entry  containing  the
          physical ID for each sector that occurs in the track.

          Note that these entries may NOT be sequential.  A disk which  uses
          sector interleave will have a sector numbering map  in  which  the
          sector numbers occur in non-sequential order.

          If ImageDisk is unable to obtain all sector numbers  in  a  single
          revolution of the  disk,  it  will  report  "Unable  to  determine
          interleave"  and  rearrange  the  sector  numbers  into  a  simple
          sequential list.

       6.5 Sector Cylinder Map

          This is an optional field.  It's presense is indicated  by  bit  7
          being set in the Head value for the track.

          When present,  it means that the cylinder values  written  to  the
          sectors do NOT match the physical cylinder of the track.

          The Sector Cylinder  Map  has  one  entry  for  each  sector,  and
          contains the logical Cylinder ID for the corresponding  sector  in
          the Sector Numbering Map.

          Reading a disk with non-standard Cylinder ID's  will  require  the
          use of the FULL ANALYSIS setting.

       6.6 Sector Head map

          This is an optional field.  It's presense is indicated  by  bit  6
          being set in the Head value for the track.

          When present, it means that the head values written to the sectors
          do NOT match the physical head selection of the track.

          The Sector Head Map has one entry for each  sector,  and  contains
          the logical Head ID for the corresponding  sector  in  the  Sector
          Numbering Map.

          Reading a disk with non-standard Head ID's may require the use  of
          the FULL ANALYSIS setting.
    ImageDisk                                                        Page: 27


       6.7 Sector Data Records

          For each sector ID occuring in the Sector Numbering Map, ImageDisk
          records a Sector Data Record - these records  occur  in  the  same
          order as the IDs in the Sector Numbering Map:

            00      Sector data unavailable - could not be read
            01 .... Normal data: (Sector Size) bytes follow
            02 xx   Compressed: All bytes in sector have same value (xx)
            03 .... Normal data with "Deleted-Data address mark"
            04 xx   Compressed  with "Deleted-Data address mark"
            05 .... Normal data read with data error
            06 xx   Compressed  read with data error
            07 .... Deleted data read with data error
            08 xx   Compressed, Deleted read with data error
    ImageDisk                                                        Page: 28*/



#define SEC_CYL_MAP  0x80
#define SEC_HEAD_MAP 0x40
typedef struct imd_trackheader_
{
	unsigned char track_mode_code;
	unsigned char physical_cylinder;
	unsigned char physical_head; // (!bits 7 & 6) 
	unsigned char number_of_sector;
	unsigned char sector_size_code;
}imd_trackheader;

