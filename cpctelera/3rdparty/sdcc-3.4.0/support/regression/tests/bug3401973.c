/*
 * /cygdrive/c/Sandbox/sdcc/sdcc/trunk/sdcc/bin/sdcc.exe  -mr2k --nostdlib --nostdinc --fverbose-asm -I../include  -c buggy_dequeue.c
 *
 */
#include <testfwk.h>

typedef  unsigned char  u8;
typedef  unsigned short u16;

#define  TCB_STATE_RUN  0
#define  NR_TASKS  16

__xdata struct tcb {
  u8 state;
  u8 thread_id;
  int  *wait;
  
  u16 timer; 
  u8 timeout;
  
  const int  *port; 
} task_table[NR_TASKS];



u8           delaying_task_head;
#define      NULL_IDX  0xff

u16          delay_tick_cnts[NR_TASKS];
u8           delay_next     [NR_TASKS];


void  buggy_dequeue( void ) {
  u8  next_head;
  
  /* wake the task up */
  task_table[ delaying_task_head ].state = TCB_STATE_RUN;
  
  next_head = delay_next[delaying_task_head];
  delay_next[delaying_task_head] = NULL_IDX;
  
  delaying_task_head = next_head;
}

void testBug(void)
{
  delay_tick_cnts[ 0] = 0x7FFF;
  delay_tick_cnts[ 7] = 0x0020;
  delay_tick_cnts[ 8] = 0x0290;
  delay_tick_cnts[10] = 0x1590;
  
  delay_next[ 7] = 10;
  delay_next[10] =  8;
  delay_next[ 8] = NULL_IDX;
  
  delaying_task_head = 7;
  
  buggy_dequeue( );
  
  ASSERT( delaying_task_head == 10 );
  ASSERT( delay_next[ 7] == NULL_IDX );
  ASSERT( delay_next[10] == 8 );
  ASSERT( delay_next[ 8] == NULL_IDX );
}

