#define QM_HEADER_SIZE 133

/* CRC table for 0x104C11DB7, bit reverse algorithm */
extern const unsigned long crc32r_table[256];
unsigned long get_u32( char* buf, int pos );
unsigned int get_u16( char* buf, int pos );
int get_i16( char* buf, int pos );
void drv_qm_update_crc( unsigned long* crc, unsigned char byte );

