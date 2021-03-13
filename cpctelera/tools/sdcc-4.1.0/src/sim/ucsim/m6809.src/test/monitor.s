uart		equ	$c000
system_stack	equ	$0200
user_stack	equ	$0400
vector_table	equ	$fff0
rom_start	equ	$e000

;
;		Start of System ROM
;
		org	rom_start

handle_reset	lds	#system_stack
		ldu	#user_stack
		andcc	#$f0
		orcc	#$60
		ldx	#system_ready
		lbsr	putstr

		include	"test.s"

		sync

;-- Status printing software interrupt
status		

		ldx	#str_cc
		lbsr	putstr
		lda	,s
		anda	#$7f
		lbsr	puthexbyte
	
		ldx	#str_a
		lbsr	putstr
		lda	1,s
		lbsr	puthexbyte

		ldx	#str_b
		lbsr	putstr
		lda	2,s
		lbsr	puthexbyte

		ldx	#str_dp
		lbsr	putstr
		lda	3,s
		lbsr	puthexbyte

		ldx	#str_x
		lbsr	putstr
		lda	4,s
		lbsr	puthexbyte
		lda	5,s
		lbsr	puthexbyte

		ldx	#str_y
		lbsr	putstr
		lda	6,s
		lbsr	puthexbyte
		lda	7,s
		lbsr	puthexbyte

		ldx	#str_nl
		lbsr	putstr

		rti

str_cc		fcc	" CC:\0"
str_a		fcc	"  A:\0"
str_b		fcc	"  B:\0"
str_dp		fcc	" DP:\0"
str_x		fcc	"  X:\0"
str_y		fcc	"  Y:\0"

str_nl		fcc	"\015\012\0"

package_io

putstr		pshs	a,x,cc
putstr_loop	lda	,x+
		beq	putstr_done
		bsr	putchar
		bra	putstr_loop
putstr_done	puls	a,x,cc
		rts

puthexbyte	pshs	cc
		rora
		rora
		rora
		rora
		bsr	puthexdigit
		rora
		rora
		rora
		rora
		rora
		bsr	puthexdigit
		puls	cc
		rts

puthexdigit	pshs	a,cc
		anda	#$0f
		adda	#'0'
		cmpa	#'9'
		ble	_puthexdigit1
		adda	#$27
_puthexdigit1	bsr	putchar
		puls	a,cc
		rts

putchar		pshs	a
_putchar1	lda	uart
		bita	#$02
		beq	_putchar1
		puls	a
		sta	uart+1
		rts

getchar		lda	uart
		bita	#$01
		beq	getchar
		lda	uart+1
		rts

package_str

toupper		pshs	cc
		cmpa	#$61
		bmi	toupper_done
		cmpa	#$7a
		bhi	toupper_done
		suba	#$20
toupper_done	puls	cc
		rts

tolower		pshs	cc
		cmpa	#$41
		bmi	tolower_done
		cmpa	#$5a
		bhi	tolower_done
		adda	#$20
tolower_done	puls	cc
		rts

prompt_str	string	"> \0"
system_ready	string	"System loaded and ready\015\012\0"

handle_undef	rti

handle_swi	equ	status
handle_swi2
handle_swi3
handle_nmi	rti

;
;		System vector specification
;
		org	vector_table
		dw	handle_undef	; $fff0
		dw	handle_swi3	; $fff2
		dw	handle_swi2	; $fff4
		dw	handle_undef	; $fff6
		dw	handle_undef	; $fff8
		dw	handle_swi	; $fffa
		dw	handle_nmi	; $fffc
		dw	handle_reset	; $fffe

handle_reset	end
