#ifndef STM8_HEADER
#define STM8_HEADER

#include "stdint.h"

/* DEVICES 
 */

#define DEV_STM8S903	0x00000001
#define DEV_STM8S003	0x00000002
#define DEV_STM8S005	0x00000004
#define DEV_STM8S007	0x00000008
#define DEV_STM8S103	0x00000010
#define DEV_STM8S105	0x00000020
#define DEV_STM8S207	0x00000040
#define DEV_STM8S208	0x00000080
#define DEV_STM8S	(DEV_STM8S903|\
			 DEV_STM8S003|\
			 DEV_STM8S005|\
			 DEV_STM8S007|\
			 DEV_STM8S103|\
			 DEV_STM8S105|\
			 DEV_STM8S207|\
			 DEV_STM8S208)

#define DEV_STM8AF52	0x00000100
#define DEV_STM8AF62_12	0x00000200
#define DEV_STM8AF62_46	0x00000400
#define DEV_STM8AF	(DEV_STM8AF52|\
			 DEV_STM8AF62_12|\
			 DEV_STM8AF62_46)

#define DEV_STM8SAF	(DEV_STM8S|DEV_STM8AF)

#define DEV_STM8AL3xE	0x00010000
#define DEV_STM8AL3x8	0x00020000
#define DEV_STM8AL3x346	0x00040000
#define DEV_STM8AL	(DEV_STM8AL3xE|\
			 DEV_STM8AL3x8|\
			 DEV_STM8AL3x346)

#define DEV_STM8L051	0x01000000
#define DEV_STM8L052C	0x02000000
#define DEV_STM8L052R	0x04000000
#define DEV_STM8L151x23	0x08000000
#define DEV_STM8L15x46	0x10000000
#define DEV_STM8L15x8	0x20000000
#define DEV_STM8L162	0x40000000

#define DEV_STM8L	(DEV_STM8L051|\
			 DEV_STM8L052C|\
			 DEV_STM8L052R|\
			 DEV_STM8L151x23|\
			 DEV_STM8L15x46|\
			 DEV_STM8L15x8|\
			 DEV_STM8L162)

#define DEV_STM8ALL	(DEV_STM8AL|DEV_STM8L)

#define DEV_STM8L101	0x00001000

#define DEV_STM8LDISC	DEV_STM8L15x46
#define DEV_LDISC	DEV_STM8L15x46
#define DEV_STM8SDISC	DEV_STM8S105
#define DEV_SDISC	DEV_STM8S105

#ifndef DEVICE
#define DEVICE DEV_STM8S208
#endif

//#define CLK_DIVR	(*(volatile uint8_t *)0x50c6)
//#define CLK_PCKENR1	(*(volatile uint8_t *)0x50c7)


//#define UART2_SR	(*(volatile uint8_t *)0x5240)
//#define UART2_DR	(*(volatile uint8_t *)0x5241)
//#define UART2_BRR1	(*(volatile uint8_t *)0x5242)
//#define UART2_BRR2	(*(volatile uint8_t *)0x5243)
//#define UART2_CR2	(*(volatile uint8_t *)0x5245)
//#define UART2_CR3	(*(volatile uint8_t *)0x5246)

//#define UART_CR2_TEN (1 << 3)
//#define UART_CR2_REN (1 << 2)
//#define UART_CR2_RIEN (1 << 5)
//#define UART_CR3_STOP2 (1 << 5)
//#define UART_CR3_STOP1 (1 << 4)
//#define UART_SR_TXE (1 << 7)
//#define UART_SR_RXNE (1 << 5)


/* GPIO
 */

struct GPIO_t {
  volatile uint8_t odr;
  volatile uint8_t idr;
  volatile uint8_t ddr;
  volatile uint8_t cr1;
  volatile uint8_t cr2;
};

#define GPIOA ((struct GPIO_t *)0x5000)
#define GPIOB ((struct GPIO_t *)0x5005)
#define GPIOC ((struct GPIO_t *)0x500A)
#define GPIOD ((struct GPIO_t *)0x500F)
#if (DEVICE & DEV_STM8SAF) || \
  (DEVICE & DEV_STM8AL) || \
  (DEVICE & DEV_STM8L052C) || \
  (DEVICE & DEV_STM8L052R) || \
  (DEVICE & DEV_STM8L151x23) || \
  (DEVICE & DEV_STM8L15x46) || \
  (DEVICE & DEV_STM8L15x8) || \
  (DEVICE & DEV_STM8L162)
#define GPIOE ((struct GPIO_t *)0x5014)
#define GPIOF ((struct GPIO_t *)0x5019)
#endif
#if (DEVICE & DEV_STM8S005) || \
  (DEVICE & DEV_STM8S007) || \
  (DEVICE & DEV_STM8S105) || \
  (DEVICE & DEV_STM8S207) || \
  (DEVICE & DEV_STM8S208) || \
  (DEVICE & DEV_STM8AF52) || \
  (DEVICE & DEV_STM8AF62_46) || \
  (DEVICE & DEV_STM8AL3xE) || \
  (DEVICE & DEV_STM8AL3x8) || \
  (DEVICE & DEV_STM8L052R) || \
  (DEVICE & DEV_STM8L15x8) || \
  (DEVICE & DEV_STM8L162)
#define GPIOG ((struct GPIO_t *)0x501E)
#endif
#if (DEVICE & DEV_STM8S005) || \
  (DEVICE & DEV_STM8S007) || \
  (DEVICE & DEV_STM8S105) || \
  (DEVICE & DEV_STM8S207) || \
  (DEVICE & DEV_STM8S208) || \
  (DEVICE & DEV_STM8AF52) || \
  (DEVICE & DEV_STM8AL3xE) || \
  (DEVICE & DEV_STM8AL3x8) || \
  (DEVICE & DEV_STM8L15x8) || \
  (DEVICE & DEV_STM8L162)
#define  GPIOH ((struct GPIO_t *)0x5023)
#define  GPIOI ((struct GPIO_t *)0x5028)
#endif


/* Timers
 */

/* bits of control 1 register */
#define TIM_CR1_CEN	(1 << 0)

/* Bits of interrupt enable register */
#define TIM_IER_UIE	(1 << 0)

/* Bits of interrupt flag register */
#define TIM_SR1_UIF	(1 << 0)

/* Bits of event generator register */
#define TIM_EGR_UG	(1 << 0)

/* TIM1
 */

#if (DEVICE & DEV_STM8SAF)
struct TIM1_t {
  volatile uint8_t cr1;		//=  0;
  volatile uint8_t cr2;		//=  1;
  volatile uint8_t smcr;	//=  2;
  volatile uint8_t etr;		//=  3;
  volatile uint8_t ier;		//=  4;
  volatile uint8_t sr1;		//=  5;
  volatile uint8_t sr2;		//=  6;
  volatile uint8_t egr;		//=  7;
  volatile uint8_t ccmr1;	//=  8;
  volatile uint8_t ccmr2;	//=  9;
  volatile uint8_t ccmr3;	//= 10;
  volatile uint8_t ccmr4;	//= 11;
  volatile uint8_t ccer1;	//= 12;
  volatile uint8_t ccer2;	//= 13;
  volatile uint8_t cntrh;	//= 14;
  volatile uint8_t cntrl;	//= 15;
  volatile uint8_t pscrh;	//= 16;
  volatile uint8_t pscrl;	//= 17;
  volatile uint8_t arrh;	//= 18;
  volatile uint8_t arrl;	//= 19;
  volatile uint8_t rcr;		//= 20;
  volatile uint8_t ccr1h;	//= 21;
  volatile uint8_t ccr1l;	//= 22;
  volatile uint8_t ccr2h;	//= 23;
  volatile uint8_t ccr2l;	//= 24;
  volatile uint8_t ccr3h;	//= 25;
  volatile uint8_t ccr3l;	//= 26;
  volatile uint8_t ccr4h;	//= 27;
  volatile uint8_t ccr4l;	//= 28;
  volatile uint8_t bkr;		//= 29;
  volatile uint8_t dtr;		//= 30;
  volatile uint8_t oisr;	//= 31;
};
#define TIM1_UP_IRQ	11
#define TIM1_CC_IRQ	12
#elif (DEVICE & DEV_STM8ALL)
struct TIM1_t {
  volatile uint8_t cr1;		//=  0;
  volatile uint8_t cr2;		//=  1;
  volatile uint8_t smcr;	//=  2;
  volatile uint8_t etr;		//=  3;
  volatile uint8_t der;         //=  4;
  volatile uint8_t ier;		//=  5;
  volatile uint8_t sr1;		//=  6;
  volatile uint8_t sr2;		//=  7;
  volatile uint8_t egr;		//=  8;
  volatile uint8_t ccmr1;	//=  9;
  volatile uint8_t ccmr2;	//= 10;
  volatile uint8_t ccmr3;	//= 11;
  volatile uint8_t ccmr4;	//= 12;
  volatile uint8_t ccer1;	//= 13;
  volatile uint8_t ccer2;	//= 14;
  volatile uint8_t cntrh;	//= 15;
  volatile uint8_t cntrl;	//= 16;
  volatile uint8_t pscrh;	//= 17;
  volatile uint8_t pscrl;	//= 18;
  volatile uint8_t arrh;	//= 19;
  volatile uint8_t arrl;	//= 20;
  volatile uint8_t rcr;		//= 21;
  volatile uint8_t ccr1h;	//= 22;
  volatile uint8_t ccr1l;	//= 23;
  volatile uint8_t ccr2h;	//= 24;
  volatile uint8_t ccr2l;	//= 25;
  volatile uint8_t ccr3h;	//= 26;
  volatile uint8_t ccr3l;	//= 27;
  volatile uint8_t ccr4h;	//= 28;
  volatile uint8_t ccr4l;	//= 29;
  volatile uint8_t bkr;		//= 30;
  volatile uint8_t dtr;		//= 31;
  volatile uint8_t oisr;	//= 32;
};
#define TIM1_UP_IRQ	23
#define TIM1_CC_IRQ	24
#endif

#if (DEVICE & DEV_STM8S) || (DEVICE & DEV_STM8AF)
#define TIM1 ((struct TIM1_t *)0x5250)
#endif
#if (DEVICE & DEV_STM8AL) || \
  (DEVICE & DEV_STM8L052C) || \
  (DEVICE & DEV_STM8L052R) || \
  (DEVICE & DEV_STM8L15x46) || \
  (DEVICE & DEV_STM8L15x8) || \
  (DEVICE & DEV_STM8L162)
#define TIM1 ((struct TIM1_t *)0x52B0)
#endif

/* TIM2
 */

#if ((DEVICE & DEV_STM8S005) || \
     (DEVICE & DEV_STM8S007) || \
     (DEVICE & DEV_STM8S105) || \
     (DEVICE & DEV_STM8S207) || \
     (DEVICE & DEV_STM8S208) || \
     (DEVICE & DEV_STM8AF52) ||	\
     (DEVICE & DEV_STM8AF62_46))
struct TIM2_t {
  volatile uint8_t cr1;		//=  0;
  volatile uint8_t ier;		//=  1;
  volatile uint8_t sr1;		//=  2;
  volatile uint8_t sr2;		//=  3;
  volatile uint8_t egr;		//=  4;
  volatile uint8_t ccmr1;	//=  5;
  volatile uint8_t ccmr2;	//=  6;
  volatile uint8_t ccmr3;	//=  7;
  volatile uint8_t ccer1;	//=  8;
  volatile uint8_t ccer2;	//=  9;
  volatile uint8_t cntrh;	//= 10;
  volatile uint8_t cntrl;	//= 11;
  volatile uint8_t pscrl;	//= 12;
  volatile uint8_t arrh;	//= 13;
  volatile uint8_t arrl;	//= 14;
  volatile uint8_t ccr1h;	//= 15;
  volatile uint8_t ccr1l;	//= 0x10;
  volatile uint8_t ccr2h;	//= 0x11;
  volatile uint8_t ccr2l;	//= 0x12;
  volatile uint8_t ccr3h;	//= 0x13;
  volatile uint8_t ccr3l;	//= 0x14;
};
#define TIM2_UP_IRQ 13
#define TIM2_CC_IRQ 14
#define TIM2 ((struct TIM2_t *)0x5300)
#elif ((DEVICE & DEV_STM8S003) || \
       (DEVICE & DEV_STM8S103))
struct TIM2_t {
  volatile uint8_t cr1;		//=  0;
  volatile uint8_t _dummy1;	//=  1;
  volatile uint8_t _dummy2;	//=  2;
  volatile uint8_t ier;		//=  3;
  volatile uint8_t sr1;		//=  4;
  volatile uint8_t sr2;		//=  5;
  volatile uint8_t egr;		//=  6;
  volatile uint8_t ccmr1;	//=  7;
  volatile uint8_t ccmr2;	//=  8;
  volatile uint8_t ccmr3;	//=  9;
  volatile uint8_t ccer1;	//= 10;
  volatile uint8_t ccer2;	//= 11;
  volatile uint8_t cntrh;	//= 12;
  volatile uint8_t cntrl;	//= 13;
  volatile uint8_t pscrl;	//= 14;
  volatile uint8_t arrh;	//= 15;
  volatile uint8_t arrl;	//= 16;
  volatile uint8_t ccr1h;	//= 0x11;
  volatile uint8_t ccr1l;	//= 0x12;
  volatile uint8_t ccr2h;	//= 0x13;
  volatile uint8_t ccr2l;	//= 0x14;
  volatile uint8_t ccr3h;	//= 0x15;
  volatile uint8_t ccr3l;	//= 0x16;
};
#define TIM2_UP_IRQ 13
#define TIM2_CC_IRQ 14
#define TIM2 ((struct TIM2_t *)0x5300)
#elif (DEVICE & DEV_STM8ALL)
struct TIM2_t {
  volatile uint8_t cr1;		//=  0;
  volatile uint8_t cr2;		//=  1;
  volatile uint8_t smcr;	//=  2;
  volatile uint8_t etr;		//=  3;
  volatile uint8_t der;		//=  4;
  volatile uint8_t ier;		//=  5;
  volatile uint8_t sr1;		//=  6;
  volatile uint8_t sr2;		//=  7;
  volatile uint8_t egr;		//=  8;
  volatile uint8_t ccmr1;	//=  9;
  volatile uint8_t ccmr2;	//= 0x0a;
  volatile uint8_t ccer1;	//= 0x0b;
  volatile uint8_t cntrh;	//= 0x0c;
  volatile uint8_t cntrl;	//= 0x0d;
  volatile uint8_t pscrl;	//= 0x0e;
  volatile uint8_t arrh;	//= 0x0f;
  volatile uint8_t arrl;	//= 0x10;
  volatile uint8_t ccr1h;	//= 0x11;
  volatile uint8_t ccr1l;	//= 0x12;
  volatile uint8_t ccr2h;	//= 0x13;
  volatile uint8_t ccr2l;	//= 0x14;
  volatile uint8_t bkr;		//= 0x15;
  volatile uint8_t oisr;	//= 0x16;
};
#define TIM2_UP_IRQ 19
#define TIM2_CC_IRQ 20
#define TIM2 ((struct TIM2_t *)0x5250)
#elif (DEVICE & DEV_STM8L101)
struct TIM2_t {
  volatile uint8_t cr1;		//=  0;
  volatile uint8_t cr2;		//=  1;
  volatile uint8_t smcr;	//=  2;
  volatile uint8_t etr;		//=  3;
  volatile uint8_t ier;		//=  4;
  volatile uint8_t sr1;		//=  5;
  volatile uint8_t sr2;		//=  6;
  volatile uint8_t egr;		//=  7;
  volatile uint8_t ccmr1;	//=  8;
  volatile uint8_t ccmr2;	//= 0x09;
  volatile uint8_t ccer1;	//= 0x0a;
  volatile uint8_t cntrh;	//= 0x0b;
  volatile uint8_t cntrl;	//= 0x0c;
  volatile uint8_t pscrl;	//= 0x0d;
  volatile uint8_t arrh;	//= 0x0e;
  volatile uint8_t arrl;	//= 0x0f;
  volatile uint8_t ccr1h;	//= 0x10;
  volatile uint8_t ccr1l;	//= 0x11;
  volatile uint8_t ccr2h;	//= 0x12;
  volatile uint8_t ccr2l;	//= 0x13;
  volatile uint8_t bkr;		//= 0x14;
  volatile uint8_t oisr;	//= 0x15;
};
#define TIM2_UP_IRQ 19
#define TIM2_CC_IRQ 20
#define TIM2 ((struct TIM2_t *)0x5250)
#endif

/* USART
 */

#define USART_CR2_TEN (1 << 3)
#define USART_CR2_REN (1 << 2)
#define USART_CR2_RIEN (1 << 5)
#define USART_CR3_STOP2 (1 << 5)
#define USART_CR3_STOP1 (1 << 4)
#define USART_SR_TXE (1 << 7)
#define USART_SR_RXNE (1 << 5)

/* USART1
 */

struct USART1_saf_t {
  volatile uint8_t sr;
  volatile uint8_t dr;
  volatile uint8_t brr1;
  volatile uint8_t brr2;
  volatile uint8_t cr1;
  volatile uint8_t cr2;
  volatile uint8_t cr3;
  volatile uint8_t cr4;
  volatile uint8_t cr5;
  volatile uint8_t gtr;
  volatile uint8_t pscr;
};

struct USART1_all_t {
  volatile uint8_t sr;
  volatile uint8_t dr;
  volatile uint8_t brr1;
  volatile uint8_t brr2;
  volatile uint8_t cr1;
  volatile uint8_t cr2;
  volatile uint8_t cr3;
  volatile uint8_t cr4;
  volatile uint8_t cr5;
  volatile uint8_t gtr;
  volatile uint8_t pscr;
};

struct USART1_l101_t {
  volatile uint8_t sr;
  volatile uint8_t dr;
  volatile uint8_t brr1;
  volatile uint8_t brr2;
  volatile uint8_t cr1;
  volatile uint8_t cr2;
  volatile uint8_t cr3;
  volatile uint8_t cr4;
};

#if (DEVICE & DEV_STM8S003) || \
  (DEVICE & DEV_STM8S007) || \
  (DEVICE & DEV_STM8S103) || \
  (DEVICE & DEV_STM8S207) || \
  (DEVICE & DEV_STM8S208) || \
  (DEVICE & DEV_STM8S903) || \
  (DEVICE & DEV_STM8AF52)
#define USART1_t USART1_saf_t
#define USART1 ((struct USART1_t *)0x5230)
#endif

#if (DEVICE & DEV_STM8ALL)
#define USART1_t USART1_all_t
#define USART1 ((struct USART1_t *)0x5230)
#endif

#if (DEVICE & DEV_STM8L101)
#define USART1_t USART1_l101_t
#define USART1 ((struct USART1_t *)0x5230)
#endif

/* USART2
 */

struct USART2_saf_t {
  volatile uint8_t sr;
  volatile uint8_t dr;
  volatile uint8_t brr1;
  volatile uint8_t brr2;
  volatile uint8_t cr1;
  volatile uint8_t cr2;
  volatile uint8_t cr3;
  volatile uint8_t cr4;
  volatile uint8_t cr5;
  volatile uint8_t cr6;
  volatile uint8_t gtr;
  volatile uint8_t pscr;
};

struct USART2_all_t {
  volatile uint8_t sr;
  volatile uint8_t dr;
  volatile uint8_t brr1;
  volatile uint8_t brr2;
  volatile uint8_t cr1;
  volatile uint8_t cr2;
  volatile uint8_t cr3;
  volatile uint8_t cr4;
  volatile uint8_t cr5;
  volatile uint8_t gtr;
  volatile uint8_t pscr;
};

#if (DEVICE & DEV_STM8S005) || \
  (DEVICE & DEV_STM8S105) || \
  (DEVICE & DEV_STM8AF62_46)
#define USART2_t USART2_saf_t
#define USART2 ((struct USART2_t *)0x5240)
#endif

#if (DEVICE & DEV_STM8AL3xE) || \
  (DEVICE & DEV_STM8AL3x8) || \
  (DEVICE & DEV_STM8L052R) || \
  (DEVICE & DEV_STM8L15x8) || \
  (DEVICE & DEV_STM8L162)
#define USART2_t USART2_all_t
#define USART2 ((struct USART2_t *)0x53E0)
#endif

/* USART3
 */

struct USART3_saf_t {
  volatile uint8_t sr;
  volatile uint8_t dr;
  volatile uint8_t brr1;
  volatile uint8_t brr2;
  volatile uint8_t cr1;
  volatile uint8_t cr2;
  volatile uint8_t cr3;
  volatile uint8_t cr4;
  volatile uint8_t _dummy;
  volatile uint8_t cr6;
  volatile uint8_t gtr;
  volatile uint8_t pscr;
};

struct USART3_all_t {
  volatile uint8_t sr;
  volatile uint8_t dr;
  volatile uint8_t brr1;
  volatile uint8_t brr2;
  volatile uint8_t cr1;
  volatile uint8_t cr2;
  volatile uint8_t cr3;
  volatile uint8_t cr4;
  volatile uint8_t cr5;
  volatile uint8_t gtr;
  volatile uint8_t pscr;
};

#if (DEVICE & DEV_STM8S007) || \
  (DEVICE & DEV_STM8S207) || \
  (DEVICE & DEV_STM8S208) || \
  (DEVICE & DEV_STM8AF52)
#define USART3_t USART3_saf_t
#define USART3 ((struct USART3_t *)0x5240)
#endif

#if (DEVICE & DEV_STM8AL3xE) || \
  (DEVICE & DEV_STM8AL3x8) || \
  (DEVICE & DEV_STM8L052R) || \
  (DEVICE & DEV_STM8L15x8) || \
  (DEVICE & DEV_STM8L162)
#define USART3_t USART3_all_t
#define USART3 ((struct USART3_t *)0x53F0)
#endif

/* USART4
 */

struct USART4_saf_t {
  volatile uint8_t sr;
  volatile uint8_t dr;
  volatile uint8_t brr1;
  volatile uint8_t brr2;
  volatile uint8_t cr1;
  volatile uint8_t cr2;
  volatile uint8_t cr3;
  volatile uint8_t cr4;
  volatile uint8_t cr5;
  volatile uint8_t cr6;
  volatile uint8_t gtr;
  volatile uint8_t pscr;
};

#if (DEVICE & DEV_STM8AF62_12)
#define USART4_t USART4_saf_t
#define USART4 ((struct USART4_t *)0x5230)
#endif

/* USART interrupt numbers */

#if (DEVICE & DEV_STM8SAF)
#if defined USART1
#define USART1_TX_IRQ 17
#define USART1_RX_IRQ 18
#endif
#endif
#if (DEVICE & DEV_STM8ALL) || \
  (DEVICE & DEV_STM8_L101)
#if defined USART1
#define USART1_TX_IRQ 27
#define USART1_RX_IRQ 28
#endif
#endif

#if (DEVICE & DEV_STM8SAF)
#if defined USART2
#define USART2_TX_IRQ 20
#define USART2_RX_IRQ 21
#endif
#endif
#if (DEVICE & DEV_STM8ALL)
#if defined USART2
#define USART2_TX_IRQ 19
#define USART2_RX_IRQ 20
#endif
#endif

#if (DEVICE & DEV_STM8SAF)
#if defined USART3
#define USART3_TX_IRQ 20
#define USART3_RX_IRQ 21
#endif
#endif
#if (DEVICE & DEV_STM8ALL)
#if defined USART3
#define USART3_TX_IRQ 21
#define USART3_RX_IRQ 22
#endif
#endif

#if defined USART4
#define USART3_TX_IRQ 17
#define USART3_RX_IRQ 18
#endif

/* Select first USART as default */

#ifndef USART
#if (DEVICE & DEV_STM8S003) || \
  (DEVICE & DEV_STM8S007) || \
  (DEVICE & DEV_STM8S103) || \
  (DEVICE & DEV_STM8S207) || \
  (DEVICE & DEV_STM8S208) || \
  (DEVICE & DEV_STM8S903) || \
  (DEVICE & DEV_STM8AF52) || \
  (DEVICE & DEV_STM8ALL) || \
  (DEVICE & DEV_STM8L101)
#define USART USART1
#define USART_TX_IRQ USART1_TX_IRQ
#define USART_RX_IRQ USART1_RX_IRQ
#endif
#if (DEVICE & DEV_STM8S005) || \
  (DEVICE & DEV_STM8S105) || \
  (DEVICE & DEV_STM8AF62_46)
#define USART USART2
#define USART_TX_IRQ USART2_TX_IRQ
#define USART_RX_IRQ USART2_RX_IRQ
#endif
#if (DEVICE & DEV_STM8AF62_12)
#define USART USART4
#define USART_TX_IRQ USART4_TX_IRQ
#define USART_RX_IRQ USART4_RX_IRQ
#endif
#endif
#if ((DEVICE & DEV_STM8S003) || \
     (DEVICE & DEV_STM8S005) || \
     (DEVICE & DEV_STM8S103) || \
     (DEVICE & DEV_STM8S105) || \
     (DEVICE & DEV_STM8S903) || \
     (DEVICE & DEV_STM8AF62_12) || \
     (DEVICE & DEV_STM8AF62_46))
#define USART_TX_GPIO GPIOD
#define USART_RX_GPIO GPIOD
#define USART_TX_PIN  5
#define USART_RX_PIN  6
#endif
#if ((DEVICE & DEV_STM8S007) ||\
     (DEVICE & DEV_STM8S207) ||\
     (DEVICE & DEV_STM8S208) ||\
     (DEVICE & DEV_STM8AF52))
#define USART_TX_GPIO GPIOA
#define USART_RX_GPIO GPIOA
#define USART_TX_PIN  5
#define USART_RX_PIN  4
#endif
#if (DEVICE & DEV_STM8AL) ||\
  (DEVICE & DEV_STM8L052C) ||\
  (DEVICE & DEV_STM8L052R) ||\
  (DEVICE & DEV_STM8L151x23) ||\
  (DEVICE & DEV_STM8L15x46) ||\
  (DEVICE & DEV_STM8L15x8) ||\
  (DEVICE & DEV_STM8L162) ||\
  (DEVICE & DEV_STM8L101)
#define USART_TX_GPIO GPIOC
#define USART_RX_GPIO GPIOC
#define USART_TX_PIN  3
#define USART_RX_PIN  2
#endif
#if (DEVICE & DEV_STM8L051)
/* non-default AF only because C2 and C3 are not available */
#define USART_TX_GPIO
#define USART_RX_GPIO
#define USART_TX_PIN 
#define USART_RX_PIN 
#endif

/* CLK
 */

#if (DEVICE & DEV_STM8SAF)
struct CLK_t {
  uint8_t ickr;
  uint8_t eckr;
  uint8_t _dummy1;
  uint8_t cmsr;
  uint8_t swr;
  uint8_t swcr;
  uint8_t ckdivr;
  uint8_t pckenr1;
  uint8_t cssr;
  uint8_t ccor;
  uint8_t pckenr2;
  uint8_t _dummy2;
  uint8_t hsitrimr;
  uint8_t swimccr;
};
#endif
#if (DEVICE & DEV_STM8ALL)
struct CLK_t {
  uint8_t ckdivr;
  uint8_t crtcr;
  uint8_t ickcr;
  uint8_t pckenr1;
  uint8_t pckenr2;
  uint8_t ccor;
  uint8_t eckcr;
  uint8_t scsr;
  uint8_t swr;
  uint8_t swcr;
  uint8_t cssr;
  uint8_t cbeepr;
  uint8_t hsicalr;
  uint8_t hsitrimr;
  uint8_t hsiunclkr;
  uint8_t regcsr;
  uint8_t pckenr3;
};
#endif
#if (DEVICE & DEV_STM8L101)
struct CLK_t {
  uint8_t ckdivr;
  uint8_t _dummy1;
  uint8_t _dummy2;
  uint8_t pckenr;
  uint8_t _dummy3;
  uint8_t ccor;
};
#endif

#define CLK ((struct CLK_t *)0x50C0)

/* UID
 */

#if (DEVICE & DEV_STM8S103) || \
  (DEVICE & DEV_STM8S903) || \
  (DEVICE & DEV_STM8AF62_12)
#define UID ((uint8_t*)0x4865)
#endif
#if (DEVICE & DEV_STM8AL) || \
  (DEVICE & DEV_STM8L151x23) || \
  (DEVICE & DEV_STM8L15x46) ||	\
  (DEVICE & DEV_STM8L15x8) || \
  (DEVICE & DEV_STM8L162)
#define UID ((uint8_t*)0x4926)
#endif
#if (DEVICE & DEV_STM8L101)
#define UID ((uint8_t*)0x4925)
#endif
  
#define EI __asm__("rim")
#define DI __asm__("sim")

#endif
