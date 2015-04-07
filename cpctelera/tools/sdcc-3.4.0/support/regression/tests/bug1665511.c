/*
   bug1665511.c
*/

#include <testfwk.h>

/* force alignment on sparc machines with gcc compiler */
#if defined(PORT_HOST) && defined(__sparc) && defined(__GNUC__)
# define LONG_ALIGNED __attribute__ ((aligned (4)))
#else
# define LONG_ALIGNED
#endif

typedef union
{
  unsigned char bytes[4];
  long l;
} IP_V4, *PIP_V4;

typedef union _short_map
{
  unsigned char chars[2];
  short shorts [1];
} Short_Map, *PShort_Map;

typedef Short_Map Port;
typedef Port *PPort;

struct sockaddr
{
  unsigned char bogus_ptr[3];              ///< Overhead for TNI native interface.
  unsigned char sin_addr[16] LONG_ALIGNED; ///< IP address. IPv4 address is in sin_addr[12-15] with MSB at sin_addr[12].
  unsigned char sin_port_high;             ///< most significant byte of port number for the socket.
  unsigned char sin_port_low;              ///< least significant byte of port number for the socket.
  unsigned char sin_family;                ///< Ignored by DS80C400 implementation.
};

typedef unsigned int SocketHandle;

#define INVALID_HANDLE_VALUE (SocketHandle) -1

int accept(SocketHandle handle, struct sockaddr * address, unsigned int size)
{
  handle;
  address;
  size;
  return 0x1234;
}

SocketHandle Accept(SocketHandle handle, PIP_V4 ip, PPort port)
{
  struct sockaddr address;
  int RetCode;

  RetCode = accept(handle, &address, sizeof(address));
  if (RetCode != INVALID_HANDLE_VALUE)
  {
    ip->l = ((PIP_V4) &address.sin_addr[12])->l;
    port->chars[0] = address.sin_port_low;
    port->chars[1] = address.sin_port_high;
  };

  return RetCode;
}

void testBug(void)
{
  IP_V4 ip = {{1, 2, 3, 4}};
  Port port = {{5, 6}};
  ASSERT(Accept(1, &ip, &port) == 0x1234);
}
