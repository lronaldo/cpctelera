#include "xc88x.h"

#include "mdu88x.h"

static uint16_t v;
static uint32_t d;

/* unsigned OPs */

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

  MDUCON= 0x10 + 0x02;
  while (MDUSTAT & 0x04) ;
  
  d= (uint32_t)MD0 + (uint32_t)MD1*256l + (uint32_t)MD2*256l*256l + (uint32_t)MD3*256l*256l*256l;
  if (res)
    *res= d;
  v= MD5*256 + MD4;
  if (rem)
    *rem= v;

  return MDUSTAT & 0x02;
}

uint8_t
mdu_16udiv16(uint16_t op1, uint16_t op2, uint16_t *res, uint16_t *rem)
  __reentrant
{
  MD0= op1 & 0xff;
  MD1= (op1 >> 8) & 0xff;
  MD4= op2 & 0xff;
  MD5= (op2 >> 8) & 0xff;

  MDUCON= 0x10 + 0x01;
  while (MDUSTAT & 0x04) ;
  
  v= MD1*256 + MD0;
  if (res)
    *res= v;
  v= MD5*256 + MD4;
  if (rem)
    *rem= v;

  return MDUSTAT & 0x02;
}

uint8_t
mdu_16umul16(uint16_t op1, uint16_t op2, uint32_t *res)
  __reentrant
{
  MD0= op1 & 0xff;
  MD4= op2 & 0xff;
  MD1= (op1 >> 8) & 0xff;
  MD5= (op2 >> 8) & 0xff;

  MDUCON= 0x10 + 0x00;
  while (MDUSTAT & 0x04) ;
  
  d= (uint32_t)MD0 + (uint32_t)MD1*256l + (uint32_t)MD2*256l*256l + (uint32_t)MD3*256l*256l*256l;
  if (res)
    *res= d;

  return MDUSTAT & 0x02;
}

/* signed OPs */

uint8_t
mdu_32sdiv16(int32_t op1, int16_t op2, int32_t *res, int16_t *rem)
  __reentrant
{
  MD0= op1 & 0xff;
  MD1= (op1 >> 8) & 0xff;
  MD2= (op1 >> 16) & 0xff;
  MD3= (op1 >> 24) & 0xff;

  MD4= op2 & 0xff;
  MD5= (op2 >> 8) & 0xff;

  MDUCON= 0x10 + 0x06;
  while (MDUSTAT & 0x04) ;
  
  d= (uint32_t)MD0 + (uint32_t)MD1*256l + (uint32_t)MD2*256l*256l + (uint32_t)MD3*256l*256l*256l;
  if (res)
    *res= d;
  v= MD5*256 + MD4;
  if (rem)
    *rem= v;

  return MDUSTAT & 0x02;
}

uint8_t
mdu_16sdiv16(int16_t op1, int16_t op2, int16_t *res, int16_t *rem)
  __reentrant
{
  MD0= op1 & 0xff;
  MD1= (op1 >> 8) & 0xff;
  MD4= op2 & 0xff;
  MD5= (op2 >> 8) & 0xff;

  MDUCON= 0x10 + 0x05;
  while (MDUSTAT & 0x04) ;
  
  v= MD1*256 + MD0;
  if (res)
    *res= v;
  v= MD5*256 + MD4;
  if (rem)
    *rem= v;

  return MDUSTAT & 0x02;
}

uint8_t
mdu_16smul16(int16_t op1, int16_t op2, int32_t *res)
  __reentrant
{
  MD0= op1 & 0xff;
  MD4= op2 & 0xff;
  MD1= (op1 >> 8) & 0xff;
  MD5= (op2 >> 8) & 0xff;

  MDUCON= 0x10 + 0x04;
  while (MDUSTAT & 0x04) ;
  
  d= (uint32_t)MD0 + (uint32_t)MD1*256l + (uint32_t)MD2*256l*256l + (uint32_t)MD3*256l*256l*256l;
  if (res)
    *res= d;

  return MDUSTAT & 0x02;
}

/* normalize */

uint8_t
mdu_norm(uint32_t op, uint32_t *res, uint8_t *nuof_shifts)
  __reentrant
{
  MD0= op & 0xff;
  MD1= (op >> 8) & 0xff;
  MD2= (op >> 16) & 0xff;
  MD3= (op >> 24) & 0xff;
  
  MDUCON= 0x10 + 0x08;
  while (MDUSTAT & 0x04) ;

  d= (uint32_t)MD0 + (uint32_t)MD1*256l + (uint32_t)MD2*256l*256l + (uint32_t)MD3*256l*256l*256l;
  if (res)
    *res= d;
  if (nuof_shifts)
    *nuof_shifts= MD4 & 0x1f;
  return MDUSTAT & 0x02;
}

/* logical shifts */

uint8_t
mdu_lshift(uint32_t op, uint8_t shifts, uint8_t right, uint32_t *res)
  __reentrant
{
  MD0= op & 0xff;
  MD1= (op >> 8) & 0xff;
  MD2= (op >> 16) & 0xff;
  MD3= (op >> 24) & 0xff;
  MD4= (right?0x20:0) + (shifts&0x1f);
  
  MDUCON= 0x10 + 0x03;
  while (MDUSTAT & 0x04) ;

  d= (uint32_t)MD0 + (uint32_t)MD1*256l + (uint32_t)MD2*256l*256l + (uint32_t)MD3*256l*256l*256l;
  if (res)
    *res= d;
  return MDUSTAT & 0x02;
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

/* arithmetic shifts */

uint8_t
mdu_ashift(int32_t op, int8_t shifts, int8_t right, int32_t *res)
  __reentrant
{
  MD0= op & 0xff;
  MD1= (op >> 8) & 0xff;
  MD2= (op >> 16) & 0xff;
  MD3= (op >> 24) & 0xff;
  MD4= (right?0x20:0) + (shifts&0x1f);
  
  MDUCON= 0x10 + 0x07;
  while (MDUSTAT & 0x04) ;

  d= (uint32_t)MD0 + (uint32_t)MD1*256l + (uint32_t)MD2*256l*256l + (uint32_t)MD3*256l*256l*256l;
  if (res)
    *res= d;
  return MDUSTAT & 0x02;
}

uint8_t
mdu_ashift_left(int32_t op, int8_t shifts, int32_t *res)
  __reentrant
{
  return mdu_lshift(op, shifts, 0, res);
}

uint8_t
mdu_ashift_right(int32_t op, int8_t shifts, int32_t *res)
  __reentrant
{
  return mdu_lshift(op, shifts, 1, res);
}
