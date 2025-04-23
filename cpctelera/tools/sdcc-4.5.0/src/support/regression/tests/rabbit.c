/*
   rabbit.c

   While looking up information on the Rabbit processors SDCC targets, I stumbled upon
   the (unrelated) Rabbit cipher. For details, see Boesgaard et alii,
   "The Rabbit Stream Cipher - Design and Security Analysis".
   Cryptography is cool, so let's put it into a regression test!
   More seriously: the diffusion design goal of ciphers makes them highly suitable as tests,
   since we get high coverage for exposing errors in the generated code,
   and the Rabbit cipher is small enough to be suitable for some SDCC targets.
   Indeed adding this regression test found issues in the f8 and ds390 ports.

   This test is based on the sample implementation from the Rabbit paper. That is not
   an implementation optimized for 8-bit devices, but good enough as test case.
   Changes were made: the type declarations have been changed to use stdint.h types,
   and type punning that assumed little endian has been replaced.
   In the paper, the code has a non-free license, but it was later released
   to the public domain, text of the message repeated below:

   Hi all!

   On behalf of Cryptico A/S, the company who designed the Rabbit stream cipher, I'm happy to relay the following:

   "Rabbit has been released into the public domain and may be used freely for any purpose."

   So in retrospect, I think that it was a good decision not to make patent issues a key criterion for the eStream portfolio: The patent status can change, the algorithmic properties can't.

   Best regards

   Erik Zenner
*/

#include <testfwk.h>

#if !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_mcs51) // Lack of memory

#ifndef _RABBIT_H
#define _RABBIT_H

#include <stddef.h>

// Type declarations of 32-bit and 8-bit unsigned integers
#if 0
typedef unsigned char rabbit_uint32;
typedef unsigned int rabbit_byte;
#else
#include <stdint.h>
typedef uint32_t rabbit_uint32;
typedef uint8_t rabbit_byte;
static inline rabbit_uint32 rabbit_uint32_from_bytes(const rabbit_byte *p)
{
  return ((rabbit_uint32)(p[0])<< 0) | ((rabbit_uint32)(p[1])<< 8) | ((rabbit_uint32)(p[2])<<16) | ((rabbit_uint32)(p[3])<<24);
}
static inline void rabbit_bytes_from_uint32(rabbit_byte *p, rabbit_uint32 u)
{
  p[0] = (u>> 0)&0xff;
  p[1] = (u>> 8)&0xff;
  p[2] = (u>>16)&0xff;
  p[3] = (u>>24)&0xff;
}
#endif

// Structure to store the instance data (internal state)
typedef struct
{
   rabbit_uint32 x[8];
   rabbit_uint32 c[8];
   rabbit_uint32 carry;
} rabbit_instance;

// All function calls returns zero on success
int rabbit_key_setup(rabbit_instance *p_instance, const rabbit_byte *p_key, size_t key_size);
int rabbit_iv_setup(const rabbit_instance *p_master_instance,
              rabbit_instance *p_instance, const rabbit_byte *p_iv, size_t iv_size);
int rabbit_cipher(rabbit_instance *p_instance, const rabbit_byte *p_src,
              rabbit_byte *p_dest, size_t data_size);

#endif

// Left rotation of a 32-bit unsigned integer
static rabbit_uint32 rabbit_rotl(rabbit_uint32 x, int rot)
{
   return (x<<rot) | (x>>(32-rot));
}

// Square a 32-bit unsigned integer to obtain the 64-bit result and return
// the 32 high bits XOR the 32 low bits
static rabbit_uint32 rabbit_g_func(rabbit_uint32 x)
{
   // Construct high and low argument for squaring
   rabbit_uint32 a = x&0xFFFF;
   rabbit_uint32 b = x>>16;

   // Calculate high and low result of squaring
   rabbit_uint32 h = ((((a*a)>>17) + (a*b))>>15) + b*b;
   rabbit_uint32 l = x*x;

   // Return high XOR low
   return h^l;
}

// Calculate the next internal state
static void rabbit_next_state(rabbit_instance *p_instance)
{
   // Temporary data
   rabbit_uint32 g[8], c_old[8], i;

   // Save old counter values
   for (i=0; i<8; i++)
      c_old[i] = p_instance->c[i];

   // Calculate new counter values
   p_instance->c[0] += 0x4D34D34D + p_instance->carry;
   p_instance->c[1] += 0xD34D34D3 + (p_instance->c[0] < c_old[0]);
   p_instance->c[2] += 0x34D34D34 + (p_instance->c[1] < c_old[1]);
   p_instance->c[3] += 0x4D34D34D + (p_instance->c[2] < c_old[2]);
   p_instance->c[4] += 0xD34D34D3 + (p_instance->c[3] < c_old[3]);
   p_instance->c[5] += 0x34D34D34 + (p_instance->c[4] < c_old[4]);
   p_instance->c[6] += 0x4D34D34D + (p_instance->c[5] < c_old[5]);
   p_instance->c[7] += 0xD34D34D3 + (p_instance->c[6] < c_old[6]);
   p_instance->carry = (p_instance->c[7] < c_old[7]);

   // Calculate the g-functions
   for (i=0;i<8;i++)
      g[i] = rabbit_g_func(p_instance->x[i] + p_instance->c[i]);

   // Calculate new state values
   p_instance->x[0] = g[0] + rabbit_rotl(g[7],16) + rabbit_rotl(g[6], 16);
   p_instance->x[1] = g[1] + rabbit_rotl(g[0], 8) + g[7];
   p_instance->x[2] = g[2] + rabbit_rotl(g[1],16) + rabbit_rotl(g[0], 16);
   p_instance->x[3] = g[3] + rabbit_rotl(g[2], 8) + g[1];
   p_instance->x[4] = g[4] + rabbit_rotl(g[3],16) + rabbit_rotl(g[2], 16);
   p_instance->x[5] = g[5] + rabbit_rotl(g[4], 8) + g[3];
   p_instance->x[6] = g[6] + rabbit_rotl(g[5],16) + rabbit_rotl(g[4], 16);
   p_instance->x[7] = g[7] + rabbit_rotl(g[6], 8) + g[5];
}

// Initialize the cipher instance (*p_instance) as function of the key (*p_key)
int rabbit_key_setup(rabbit_instance *p_instance, const rabbit_byte *p_key, size_t key_size)
{
   // Temporary data
   rabbit_uint32 k0, k1, k2, k3, i;

   // Return error if the key size is not 16 bytes
   if (key_size != 16)
      return -1;

   // Generate four subkeys
#if 0
   k0 = *(rabbit_uint32*)(p_key+ 0);
   k1 = *(rabbit_uint32*)(p_key+ 4);
   k2 = *(rabbit_uint32*)(p_key+ 8);
   k3 = *(rabbit_uint32*)(p_key+12);
#else
   k0 = rabbit_uint32_from_bytes(p_key+ 0);
   k1 = rabbit_uint32_from_bytes(p_key+ 4);
   k2 = rabbit_uint32_from_bytes(p_key+ 8);
   k3 = rabbit_uint32_from_bytes(p_key+12);
#endif

   // Generate initial state variables
   p_instance->x[0] = k0;
   p_instance->x[2] = k1;
   p_instance->x[4] = k2;
   p_instance->x[6] = k3;
   p_instance->x[1] = (k3<<16) | (k2>>16);
   p_instance->x[3] = (k0<<16) | (k3>>16);
   p_instance->x[5] = (k1<<16) | (k0>>16);
   p_instance->x[7] = (k2<<16) | (k1>>16);

   // Generate initial counter values
   p_instance->c[0] = rabbit_rotl(k2, 16);
   p_instance->c[2] = rabbit_rotl(k3, 16);
   p_instance->c[4] = rabbit_rotl(k0, 16);
   p_instance->c[6] = rabbit_rotl(k1, 16);
   p_instance->c[1] = (k0&0xFFFF0000) | (k1&0xFFFF);
   p_instance->c[3] = (k1&0xFFFF0000) | (k2&0xFFFF);
   p_instance->c[5] = (k2&0xFFFF0000) | (k3&0xFFFF);
   p_instance->c[7] = (k3&0xFFFF0000) | (k0&0xFFFF);

   // Reset carry bit
   p_instance->carry = 0;

   // Iterate the system four times
   for (i=0; i<4; i++)
      rabbit_next_state(p_instance);

   // Modify the counters
   for (i=0; i<8; i++)
      p_instance->c[(i+4)&0x7] ^= p_instance->x[i];

   // Return success
   return 0;
}

// Initialize the cipher instance (*p_instance) as function of the IV (*p_iv)
// and the master instance (*p_master_instance)
int rabbit_iv_setup(const rabbit_instance *p_master_instance,
rabbit_instance *p_instance, const rabbit_byte *p_iv, size_t iv_size)
{
   // Temporary data
   rabbit_uint32 i0, i1, i2, i3, i;

   // Return error if the IV size is not 8 bytes
   if (iv_size != 8)
      return -1;

   // Generate four subvectors
#if 0
   i0 = *(rabbit_uint32*)(p_iv+0);
   i2 = *(rabbit_uint32*)(p_iv+4);
#else
   i0 = rabbit_uint32_from_bytes(p_iv+0);
   i2 = rabbit_uint32_from_bytes(p_iv+4);
#endif
   i1 = (i0>>16) | (i2&0xFFFF0000);
   i3 = (i2<<16) | (i0&0x0000FFFF);

   // Modify counter values
   p_instance->c[0] = p_master_instance->c[0] ^ i0;
   p_instance->c[1] = p_master_instance->c[1] ^ i1;
   p_instance->c[2] = p_master_instance->c[2] ^ i2;
   p_instance->c[3] = p_master_instance->c[3] ^ i3;
   p_instance->c[4] = p_master_instance->c[4] ^ i0;
   p_instance->c[5] = p_master_instance->c[5] ^ i1;
   p_instance->c[6] = p_master_instance->c[6] ^ i2;
   p_instance->c[7] = p_master_instance->c[7] ^ i3;

   // Copy internal state values
   for (i=0; i<8; i++)
      p_instance->x[i] = p_master_instance->x[i];
   p_instance->carry = p_master_instance->carry;

   // Iterate the system four times
   for (i=0; i<4; i++)
      rabbit_next_state(p_instance);

   // Return success
   return 0;
}

// Encrypt or decrypt a block of data
int rabbit_cipher(rabbit_instance *p_instance, const rabbit_byte *p_src,
rabbit_byte *p_dest, size_t data_size)
{
   // Temporary data
   rabbit_uint32 i;

   // Return error if the size of the data to encrypt is
   // not a multiple of 16
   if (data_size%16)
      return -1;

   for (i=0; i<data_size; i+=16)
   {
      // Iterate the system
      rabbit_next_state(p_instance);

      // Encrypt 16 bytes of data
#if 0
      *(rabbit_uint32*)(p_dest+ 0) = *(rabbit_uint32*)(p_src+ 0) ^ p_instance->x[0] ^
                             (p_instance->x[5]>>16) ^ (p_instance->x[3]<<16);
      *(rabbit_uint32*)(p_dest+ 4) = *(rabbit_uint32*)(p_src+ 4) ^ p_instance->x[2] ^
                             (p_instance->x[7]>>16) ^ (p_instance->x[5]<<16);
      *(rabbit_uint32*)(p_dest+ 8) = *(rabbit_uint32*)(p_src+ 8) ^ p_instance->x[4] ^
                             (p_instance->x[1]>>16) ^ (p_instance->x[7]<<16);
      *(rabbit_uint32*)(p_dest+12) = *(rabbit_uint32*)(p_src+12) ^ p_instance->x[6] ^
                             (p_instance->x[3]>>16) ^ (p_instance->x[1]<<16);
#else
      rabbit_bytes_from_uint32(p_dest+ 0, rabbit_uint32_from_bytes(p_src+ 0) ^ p_instance->x[0] ^
                             (p_instance->x[5]>>16) ^ (p_instance->x[3]<<16));
      rabbit_bytes_from_uint32(p_dest+ 4, rabbit_uint32_from_bytes(p_src+ 4) ^ p_instance->x[2] ^
                             (p_instance->x[7]>>16) ^ (p_instance->x[5]<<16));
      rabbit_bytes_from_uint32(p_dest+ 8, rabbit_uint32_from_bytes(p_src+ 8) ^ p_instance->x[4] ^
                             (p_instance->x[1]>>16) ^ (p_instance->x[7]<<16));
      rabbit_bytes_from_uint32(p_dest+12, rabbit_uint32_from_bytes(p_src+12) ^ p_instance->x[6] ^
                             (p_instance->x[3]>>16) ^ (p_instance->x[1]<<16));
#endif

      // Increment pointers to source and destination data
      p_src += 16;
      p_dest += 16;
   }

   // Return success
   return 0;
}

#include <string.h>

/* To simplify this, we just encrypt zero data with a zero key and initalization vector.
   Which is not as bad as it looks at first sight: the decryption routine is the same as
   the encrpytion, so it does get tested with nonzero data input during decryption. And
   when using an initialization vector, the actual key is derived from it, so we also test
   with a nonzero key. */

const rabbit_byte zero[384/8];
const rabbit_byte s[384/8] = {0x02, 0xF7, 0x4A, 0x1C, 0x26, 0x45, 0x6B, 0xF5, 0xEC, 0xD6, 0xA5, 0x36, 0xF0, 0x54, 0x57, 0xB1,
                              0xA7, 0x8A, 0xC6, 0x89, 0x47, 0x6C, 0x69, 0x7B, 0x39, 0x0C, 0x9C, 0xC5, 0x15, 0xD8, 0xE8, 0x88,
                              0x96, 0xD6, 0x73, 0x16, 0x88, 0xD1, 0x68, 0xDA, 0x51, 0xD4, 0x0C, 0x70, 0xC3, 0xA1, 0x16, 0xF4};
const rabbit_byte i[384/8] = {0xED, 0xB7, 0x05, 0x67, 0x37, 0x5D, 0xCD, 0x7C, 0xD8, 0x95, 0x54, 0xF8, 0x5E, 0x27, 0xA7, 0xC6,
                              0x8D, 0x4A, 0xDC, 0x70, 0x32, 0x29, 0x8F, 0x7B, 0xD4, 0xEF, 0xF5, 0x04, 0xAC, 0xA6, 0x29, 0x5F,
                              0x66, 0x8F, 0xBF, 0x47, 0x8A, 0xDB, 0x2B, 0xE5, 0x1E, 0x6C, 0xDE, 0x29, 0x2B, 0x82, 0xDE, 0x2A};
rabbit_byte buffer[384/8];
rabbit_instance instance;

#endif

void testRabbit(void)
{
#if !defined(__SDCC_pdk13) && !defined(__SDCC_pdk14) && !defined(__SDCC_pdk15) && !defined(__SDCC_mcs51) // Lack of memory
   // Test without IV
   ASSERT(!rabbit_key_setup(&instance, zero, 128/8));
   ASSERT(!rabbit_cipher(&instance, zero, buffer, 384/8));
#if !defined(__SDCC_f8) && !defined(__SDCC_ds390) // bug
   ASSERT(!memcmp(s, buffer, 384/8));
#endif
   ASSERT(!rabbit_key_setup(&instance, zero, 128/8));
   ASSERT(!rabbit_cipher(&instance, buffer, buffer, 384/8));
   ASSERT(!memcmp(zero, buffer, 384/8));

   // Test with IV
   ASSERT(!rabbit_key_setup(&instance, zero, 128/8));
   ASSERT(!rabbit_iv_setup(&instance, &instance, zero, 64/8));
   ASSERT(!rabbit_cipher(&instance, zero, buffer, 384/8));
#if !defined(__SDCC_f8) && !defined(__SDCC_ds390) // bug
   ASSERT(!memcmp(i, buffer, 384/8));
#endif
   ASSERT(!rabbit_key_setup(&instance, zero, 128/8));
   ASSERT(!rabbit_iv_setup(&instance, &instance, zero, 64/8));
   ASSERT(!rabbit_cipher(&instance, buffer, buffer, 384/8));
   ASSERT(!memcmp(zero, buffer, 384/8));
#endif
}

