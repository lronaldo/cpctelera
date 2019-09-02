/* bug-2569.c
   An error in instruction size estimation resulted in an out-of-range relative jump.
 */

#include <testfwk.h>
#include <stdint.h>

#ifdef __SDCC
#pragma std_c99
#endif

typedef unsigned int u16_t;
typedef unsigned char u8_t;

#include <string.h>

#define UIP_ARPTAB_SIZE 8

#define uip_ipaddr_cmp(addr1, addr2) ((addr1)->u16[0] == (addr2)->u16[0] && \
				      (addr1)->u16[1] == (addr2)->u16[1])

typedef struct uip_eth_addr {
  u8_t addr[6];
} uip_eth_addr;

typedef union uip_ip4addr_t {
  u8_t  u8[4];
  u16_t u16[2];
} uip_ipaddr_t;

const uip_ipaddr_t uip_all_zeroes_addr = { { 0x0, } };

struct arp_entry {
  uip_ipaddr_t ipaddr;
  struct uip_eth_addr ethaddr;
  u8_t time;
};

#ifndef __SDCC_pdk14 // Lack of memory
static struct arp_entry arp_table[UIP_ARPTAB_SIZE];

static u8_t i;

static u8_t arptime;

#define UIP_ARP_MAXAGE 120

void
uip_arp_timer(void)
{
  struct arp_entry *tabptr;
  
  ++arptime;
  for(i = 0; i < UIP_ARPTAB_SIZE; ++i) {
    tabptr = &arp_table[i];
    if(uip_ipaddr_cmp(&tabptr->ipaddr, &uip_all_zeroes_addr) &&
       arptime - tabptr->time >= UIP_ARP_MAXAGE) {
      memset(&tabptr->ipaddr, 0, 4);
    }
  }
}
#endif

void testBug (void)
{
}

