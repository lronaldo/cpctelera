/* */

#ifndef FLASH_HEADER
#define FLASH_HEADER

extern void flash_punlock(void);
extern void flash_dunlock(void);
extern void flash_plock(void);
extern void flash_dlock(void);
extern void flash_lock(void);

extern void flash_byte_mode(void);
extern void flash_word_mode(void);
extern void flash_erase_mode(void);

extern uint8_t flash_wait_finish(void);
extern uint8_t flash_erase(volatile uint8_t *addr, volatile uint8_t *iapsr);

#endif

/* End of stm8.src/test/flash.h */
