/* */

#include <string.h>

#include "stm8.h"

#include "flash.h"


// Lock, unklock

void
flash_punlock(void)
{
  FLASH->pukr= 0x56;
  FLASH->pukr= 0xae;
}

void
flash_dunlock(void)
{
  FLASH->pukr= 0xae;
  FLASH->pukr= 0x56;
}

void
flash_plock(void)
{
  FLASH->iapsr&= ~0x02;
}

void
flash_dlock(void)
{
  FLASH->iapsr&= ~0x08;
}

void
flash_lock(void)
{
  FLASH->iapsr&= ~0x0a;
}


// Set programing mode

void
flash_byte_mode(void)
{
  FLASH->cr2= 0;
#if (DEVICE & DEV_STM8SAF)
  FLASH->ncr2= 0xff;
#endif
}

void
flash_word_mode(void)
{
  FLASH->cr2= 0x40;
#if (DEVICE & DEV_STM8SAF)
  FLASH->ncr2= 0xbf;
#endif
}

void
flash_erase_mode(void)
{
  FLASH->cr2= 0x20;
#if (DEVICE & DEV_STM8SAF)
  FLASH->ncr2= 0xdf;
#endif
}


// Check the result

uint8_t
flash_wait_finish(void)
{
  unsigned long int timeout= 0xfffff;
  //volatile
  uint8_t r;

  r= FLASH->iapsr;
  while (((r & 0x05) == 0) &&
	 (timeout != 0))
    {
      timeout--;
      r= FLASH->iapsr;
    }
  if (r & 0x04)
    return 0;
  if (r & 0x01)
    return 1;
  if (timeout == 0)
    return 2;
  return 3;
}

uint8_t
flash_erase_fn(volatile uint8_t *addr, volatile uint8_t *iapsr)
{
  volatile uint8_t r;
  unsigned long timeout= 0xfffff;
  flash_erase_mode();
  *(addr++)= 0;
  *(addr++)= 0;
  *(addr++)= 0;
  *(addr)= 0;
  r= *iapsr;
  while (((r & 0x05) == 0) &&
	 (timeout != 0))
    {
      timeout--;
      r= *iapsr;
      GPIOD->odr^= 1;
    }
  if (r & 0x04)
    return 0;
  if (r & 0x01)
    return 1;
  if (timeout == 0)
    return 2;
  return 3;
}

uint8_t flash_op_in_ram[120];

uint8_t
flash_erase(volatile uint8_t *addr, volatile uint8_t *iapsr)
{
  uint8_t r;
  typedef uint8_t (*ft)(volatile uint8_t *addr, volatile uint8_t *iapsr);
  ft f= (ft)flash_op_in_ram;
  memcpy(flash_op_in_ram, &flash_erase_fn, 119);
  r= (*f)(addr, iapsr);
  return r;
}


/* End of stm8.src/test/flash.c */
