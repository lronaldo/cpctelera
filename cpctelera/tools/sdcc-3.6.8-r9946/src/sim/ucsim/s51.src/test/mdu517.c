#include "c517.h"

#include "mdu517.h"

static uint16_t v;
static uint32_t d;

uint8_t
mdu_32udiv16(uint32_t op1, uint16_t op2, uint32_t *res, uint16_t *rem)
  __reentrant
{
  MD0= op1 & 0xff;
  MD1= (op1 >> 8) & 0xff;
  MD2= (op1 >> 16) & 0xff;
  MD3= (op1 >> 24) & 0xff;

  MD4= op2 & 0xff;
  MD5= (op2 >> 8) & 0xff;

  __asm__ ("nop");

  d= (uint32_t)MD0 + (uint32_t)MD1*256l + (uint32_t)MD2*256l*256l + (uint32_t)MD3*256l*256l*256l;
  if (res)
    *res= d;
  v= MD5*256 + MD4;
  if (rem)
    *rem= v;

  return ARCON & 0xc0;
}

uint8_t
mdu_16udiv16(uint16_t op1, uint16_t op2, uint16_t *res, uint16_t *rem)
  __reentrant
{
  MD0= op1 & 0xff;
  MD1= (op1 >> 8) & 0xff;
  MD4= op2 & 0xff;
  MD5= (op2 >> 8) & 0xff;

  __asm__ ("nop");

  v= MD1*256 + MD0;
  if (res)
    *res= v;
  v= MD5*256 + MD4;
  if (rem)
    *rem= v;

  return ARCON & 0xc0;
}

uint8_t
mdu_16umul16(uint16_t op1, uint16_t op2, uint32_t *res)
  __reentrant
{
  MD0= op1 & 0xff;
  MD4= op2 & 0xff;
  MD1= (op1 >> 8) & 0xff;
  MD5= (op2 >> 8) & 0xff;

  __asm__ ("nop");

  d= (uint32_t)MD0 + (uint32_t)MD1*256l + (uint32_t)MD2*256l*256l + (uint32_t)MD3*256l*256l*256l;
  if (res)
    *res= d;

  return ARCON & 0x80;
}

uint8_t
mdu_norm(uint32_t op, uint32_t *res, uint8_t *nuof_shifts)
  __reentrant
{
  uint8_t a;
  
  MD0= op & 0xff;
  MD1= (op >> 8) & 0xff;
  MD2= (op >> 16) & 0xff;
  MD3= (op >> 24) & 0xff;
  ARCON= 0;
  
  __asm__ ("nop");

  d= (uint32_t)MD0 + (uint32_t)MD1*256l + (uint32_t)MD2*256l*256l + (uint32_t)MD3*256l*256l*256l;
  if (res)
    *res= d;
  a= ARCON;
  if (nuof_shifts)
    *nuof_shifts= a & 0x1f;
  return a & 0xc0;
}

uint8_t
mdu_lshift(uint32_t op, uint8_t shifts, uint8_t right, uint32_t *res)
  __reentrant
{
  MD0= op & 0xff;
  MD1= (op >> 8) & 0xff;
  MD2= (op >> 16) & 0xff;
  MD3= (op >> 24) & 0xff;
  ARCON= (right?0x20:0) + (shifts&0x1f);
  
  __asm__ ("nop");

  d= (uint32_t)MD0 + (uint32_t)MD1*256l + (uint32_t)MD2*256l*256l + (uint32_t)MD3*256l*256l*256l;
  if (res)
    *res= d;
  return ARCON & 0x80;
}

uint8_t
mdu_lshift_left(uint32_t op, uint8_t shifts, uint32_t *res)
  __reentrant
{
  return mdu_lshift(op, shifts, 0, res);
}

uint8_t
mdu_lshift_right(uint32_t op, uint8_t shifts, uint32_t *res)
  __reentrant
{
  return mdu_lshift(op, shifts, 1, res);
}
