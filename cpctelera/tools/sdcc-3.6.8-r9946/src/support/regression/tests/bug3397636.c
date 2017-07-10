/*
   bug3397636.c
*/

#include <testfwk.h>

#pragma disable_warning 85	// Unref. function arg. buf in cf_rdblk().

typedef  unsigned char  u8;
typedef  unsigned short  u16;

unsigned sf_page_size;

void  sf_read_device_id( void ) {
  sf_page_size = 0x0108;
}

u16 cf_rdblk( u8 *buf, int bufsize ) {
  ASSERT(bufsize == 0x108);
  return 4;
}

void far_cf_read() {
  u8  cf_buf[1];
  
  u16 block=0x0CC0;
  u16 result;
  
  sf_read_device_id( );
  
  while(1) {
    result=cf_rdblk((u8 *)cf_buf, sf_page_size);
    
    if(result==4)
      return;
    block--;
  }
}

void testBug(void) {
  far_cf_read();
}

