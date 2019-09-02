;; crt0_asm.s - tlcs90
	
	.t90
	
	.module crt0

	.globl _meminit
	.globl _main
	.globl _xxaddr
	.globl _xxby
	.globl _xxdata

	.globl _ser_putchar_irq
	.globl _ser_puts
	
	;; put as abs in dat at end of file..
	;.globl _ser_irq_flags
	;.globl _ser_char
	
	.globl _xget_byte_asm
	.globl _xset_byte_asm


	 ;; for interrupts the pc and af are pushed on the stack
	
	.globl _irq_info_swi
	.globl _irq_info_intwd
	.globl _irq_info_intt0
	.globl _irq_info_intt1
	.globl _timer1_irq
	.globl _irq_info_intt3
	.globl _irq_info_intt4
	.globl _irq_info_int1
	.globl _irq_info_intt5

	.globl _ticks16
	.globl _xflag
	
	;; NOTE: header might get trashed by linker when code loc ist not used!?
	.area	_HEADER (ABS)
	
	.org 	0

	.include "t90regs.s"

	;; _ser_irq_flags
TX_ACTIVE	.equ 0
RX_READY        .equ 1
	
L0:
	;halt
	;jr    L0
	jp    _astart
	nop  
	nop  
	nop  
	nop  
	nop  
	nop  
	nop  
	nop  
	nop  
	nop  
	nop  
	nop  
	nop
L10:	
	; SWI
	jp    _swi_irq
	nop  
	nop  
	nop  
	nop  
	nop  

L18:	
	nop  ; NMI
	nop  
	nop  
	nop  
	nop  
	nop  
	nop  
	nop  

L20:
	; INTWD
	jp    _intwd_irq
	nop  
	nop  
	nop  
	nop  
	nop
L28:	
	reti ; INT0
	nop  
	nop  
	nop  
	nop  
	nop  
	nop  
	nop  
L30:	
	; INTT0
	jp    _intt0_irq
	nop  
	nop  
	nop  
	nop  
	nop
L38:	
	; INTT1
	jp    _intt1_irq
	nop  
	nop  
	nop  
	nop  
	nop
L40:
	; INTAD 
	reti 
	nop  
	nop  
	nop  
	nop  
	nop  
	nop  
	nop
L48:	
	; INTT3
	jp    _intt3_irq
	nop  
	nop  
	nop  
	nop  
	nop
L50:	
	; INTT4
	jp    _intt4_irq
	nop  
	nop  
	nop  
	nop  
	nop
L58:	
	; INT1
	jp    _int1_irq
	nop  
	nop  
	nop  
	nop  
	nop
L60:	
	; INTT5
	jp    _intt5_irq
	nop  
	nop  
	nop  
	nop  
	nop
L68:	
	reti ; INT2
	nop  
	nop  
	nop  
	nop  
	nop  
	nop  
	nop
L70:	
	jp _serial_rx_irq
	nop  
	nop  
	nop  
	nop  
	nop
L78:	
	jp _serial_tx_irq
	nop  
	nop  
	nop  
	nop  
	nop
L80:	
	nop  

	.org 0x100
	
_astart:
	ld    sp, #0x0FFA0	; stack
	
	;halt
	;swi
	
	or    (P01CR), #0x04
	
	ld    (P3), #0xF7	
	ld    (P3CR), #0xA5	; P30 = RXD, P31 = IN:PaperOut CN3, P32 = TXD, P33 = OUT:CN2.LF P34:IN, P37:IN
	
	ld    (P4), #0x00
	ld    (P4CR), #0x03	; address output
	ld    (P7), #0x03
	ld    (P67CR), #0xF0
	ld    (SMMOD), #0xA0
	ld    (SMCR), #0x00
	ld    (T4MOD), #0x35
	ld    (P8), #0x08
	ld    (P8CR), #0x08
	ld    (T4FFCR), #0x01
	
	ld    (TMOD), #0x40
	
	;ld    (SCMOD), #0x03	; 9600
	ld    (SCMOD), #0x01	; 19200
	or    (SCMOD), #0x08	; 8-bit data
	or    (SCMOD), #0x20	; receiver enable

	ld    (SCCR), #0x00
	
	ld    (WDMOD), #0xC0
	ld    (TRUN), #0x20	; PRRUN, start prescaler
	or    (TRUN), #0xE0	; BRATE, 19200 baud

	;; enable rx/tx irqs
	set   0,(INTEL)		; tx
	set   1,(INTEL)		; rx

	;call _boot1
	
	ld    hl, #0xFF20	; data start
	ld    a, #0x00
	
clear_ram:
	call  _wd_reset_asm
	
	ld    (hl),a
	inc   hl
	cp    hl, #0xFFC0	; io start, 160 bytes data
	jr    nz, clear_ram
	
	call  _wd_reset_asm

	res   TX_ACTIVE,(_ser_irq_flags)
	
	;call _boot2
	call _meminit
	
	call  _wd_reset_asm
	
        ;; Initialise global variables

	;call _boot3
	
        call gsinit

	;call _boot4
	
	call _main
	halt
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	.area _CODE
	
	;; get far byte using by
	
_xget_byte_asm:
	push bc
	ld	iy, (_xxaddr)
	ld	b, (BY)
	ld	a, (_xxby)
	ld	(BY), a
	ld	a,(iy)
	ld	(BY), b
	ld	(_xxdata), a
	pop	bc
	ret

_xset_byte_asm:
	push bc
	ld	iy, (_xxaddr)
	ld	b, (BY)
	ld	a, (_xxby)
	ld	(BY), a
	ld 	a, (_xxdata)
	ld	(iy), a
	ld	(BY), b
	pop	bc
	ret
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_serial_rx_irq:
	ld a, (SCBUF)
	ld (_ser_char), a
	set RX_READY, (_ser_irq_flags)
	reti 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
_serial_tx_irq:
	res TX_ACTIVE,(_ser_irq_flags)
	ld (IRFH), #0x0F ; clear tx request flag (vector 0x78/8 = 0x0F)
	reti 

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_swi_irq:
	push hl
	ld hl, 4(sp)
	ld (_swi_pc), hl	; pc
	ld hl, 2(sp)
	ld (_swi_af), hl	; af
	call _irq_info_swi
	pop hl
	reti
_intwd_irq:	
	call _irq_info_intwd
	reti
_intt0_irq:	
	call _irq_info_intt0
	reti
_intt1_irq:	
	;call _irq_info_intt1
	;exx

	; hangs when exx is used?
	incw (_ticks16) 

	; hangs when exx is used?
	;; ld hl, (_ticks16)
	;; inc hl
	;; ld (_ticks16), hl
	
	ld (WDCR), #0x4E
	ld (IRFL), #0x10;
	;call _timer1_irq
	;exx
	reti
_intt3_irq:	
	call _irq_info_intt3
	reti
_intt4_irq:	
	call _irq_info_intt4
	reti
_int1_irq:	
	call _irq_info_int1
	reti
_intt5_irq:	
	call _irq_info_intt5
	reti

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
_ser_putchar_irq:
	ld    a, 2(sp)

_ser_putchar_asm:
	
loop1:

	;; check tx active flag
	bit   TX_ACTIVE,(_ser_irq_flags)
	jr    z, send1

	di
	bit   0,(IRFH) ; check irq
	jr    nz, send1
	ei
	
	nop
	nop
	call  _wd_reset_asm
	jr    loop1 ; loop, wait for irq

send1:	
	di
	ld    (IRFH),#0x0F ; clear tx request flag (vector 0x78/8 = 0x0F)
	ld    (SCBUF),a ; send
	set   TX_ACTIVE,(_ser_irq_flags) ; tx active..
	ei
	ret
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
_ser_puts:
	ld hl, 2(sp)
	
ser_puts1:
	ld a,(hl)
	or a, a
	ret z
	
	call _ser_putchar_asm
	inc hl
	jr ser_puts1
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_WatchDogDisable:
	ld    (WDMOD), #0x00 	; wd disable code
	ld    (WDCR), #0xB1 	; wd disable code
	ret
	
_wd_reset_asm:
	ld    (WDCR), #0x4E 	; wd clear code

.if 0
	; STAR ASIC? Not needed..
	push  bc
	push  iy
	
	ld    b,(BY)
	ld    (BY), #0x01
	ld    iy, #0x4001		
	set   1,(iy)
	nop  
	nop  
	res   1,(iy)
	ld    (BY),b
	
	pop   iy
	pop   bc

.endif
	
	ret
	
	;; Ordering of segments for the linker.
	.area	_HOME
	.area	_CODE
	.area	_INITIALIZER
	.area   _GSINIT
	.area   _GSFINAL

	.area	_DATA
	.area	_INITIALIZED
	.area	_BSEG
	.area   _BSS
	.area   _HEAP

	.area   _CODE

	.area   _GSINIT
gsinit::
	ld	bc, #l__INITIALIZER
	ld	a, b
	or	a, c
	jr	Z, gsinit_next
	ld	de, #s__INITIALIZED
	ld	hl, #s__INITIALIZER
	ldir
gsinit_next:

	.area   _GSFINAL
	ret
	
	.area _DATA
; __sfr __at	
_ser_irq_flags	=	0xffbf
_ser_char	=	0xffbe
_swi_pc		=       0xffbc
_swi_af         =       0xffba
_ticks16	=	0xffb8
_xflag		=	0xffb7
	
