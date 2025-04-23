/* bug-3556.c
   ?
 */

#include <testfwk.h>

#include <stdint.h>
#include <stdio.h>
#include <setjmp.h>

jmp_buf buf;

uint8_t pa_odr, pb_idr, dummy;

#define PA_ODR pa_odr
#define PA_DDR dummy
#define PA_CR1 dummy
#define PA_CR2 dummy
#define PB_IDR pb_idr

void setBit(uint8_t* port, uint8_t bit){
    (*(port)) |= (1 << bit);
}

void clearBit(uint8_t* port, uint8_t bit){
    (*(port)) &= ~(1 << bit);
}

void toggleBit(uint8_t* port, uint8_t bit){
    (*(port)) ^= (1 << bit);
}

uint8_t extractBit(uint8_t port, uint8_t bit){
   return ((port & (1 << bit)) >> bit);
}

void writeBit(uint8_t* port, uint8_t bit, uint8_t value){
    if(value){
        setBit(port, bit);
    }
    else{
        clearBit(port, bit);
    }
    longjmp (buf, 1);
}

void m(){
    setBit(&PA_DDR, 1); // configure PA1 as output
    setBit(&PA_CR1, 1); // push-pull mode
    setBit(&PA_CR2, 1); // high speed

    while (1) 
    {
        writeBit(&PA_ODR, 1, extractBit(PB_IDR, 2));
    }
}

void writeBit2(uint8_t* port, uint8_t bit, uint8_t value){
    if(value){
        setBit(port, bit);
    }
    else{
        clearBit(port, bit);
    }
}

void m2(){
	int i = 1;
    setBit(&PA_DDR, 1); // configure PA1 as output
    setBit(&PA_CR1, 1); // push-pull mode
    setBit(&PA_CR2, 1); // high speed

    while (i--) 
    {
        writeBit2(&PA_ODR, 1, extractBit(PB_IDR, 2));
    }
}

void
testBug (void) {
	PB_IDR = 1 << 2;
	PA_ODR = 0;
	if (!setjmp (buf))
		m ();
	ASSERT (PA_ODR == 1 << 1);

	PB_IDR = 0 << 2;
	PA_ODR = 0;
	if (!setjmp (buf))
		m ();
	ASSERT (PA_ODR == 0 << 1);

	PB_IDR = 1 << 2;
	PA_ODR= 0;
	m2 ();
	ASSERT (PA_ODR == 1 << 1);

	PB_IDR = 0 << 2;
	PA_ODR = 0;
	m2 ();
	ASSERT (PA_ODR == 0 << 1);
}

