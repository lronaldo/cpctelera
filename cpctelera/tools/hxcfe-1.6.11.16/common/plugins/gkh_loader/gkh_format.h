#pragma pack(1)

typedef struct gkh_header_
{
	unsigned long  header_tag;   // "TDDF"
	unsigned char  type;         // I
	unsigned char  version;		 // 0x01
	unsigned short numberoftags;
}gkh_header;


typedef struct image_type_tag_
{
	unsigned char  tagtype;      // 0x0A
	unsigned char  datatype;     // I
	unsigned short nboftrack;		
	unsigned short nbofheads;
	unsigned short nbofsectors;
	unsigned short sectorsize;
}image_type_tag;

typedef struct image_location_tag_
{
	unsigned char  tagtype;      // 0x0B
	unsigned char  datatype;     // I
	unsigned long  longword1;
	unsigned long  fileoffset;
}image_location_tag;

#pragma pack()

