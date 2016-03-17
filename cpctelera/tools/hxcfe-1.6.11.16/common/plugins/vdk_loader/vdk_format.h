typedef struct vdk_header_
{
	unsigned short signature;     // "dk" -> 0x6B64
	unsigned short header_size;   //Header size (little-endian)
	unsigned char version;        //Version of VDK format
	unsigned char comp_version;   //Backwards compatibility version
	unsigned char file_source_id; //Identity of file source
	unsigned char file_source_ver;//Version of file source
	unsigned char number_of_track;//Number of tracks
	unsigned char number_of_sides;//Number of sides
	unsigned char flags;		  //Flags
	unsigned char name_len;       //Compression flags and name length
}vdk_header;
