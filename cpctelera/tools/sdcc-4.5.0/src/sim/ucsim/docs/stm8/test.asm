	; For an STM8S003

	TIM4_CR1              =: 0x5340
	TIM4_IER              =: 0x5343
	TIM4_EGR              =: 0x5345
	TIM4_ARR              =: 0x5348


	.macro codebndry
		.$.end =: .
		.bndry 4
		.$.diff =: . - .$.end
		. = . - .$.diff
		.rept .$.diff
			nop
		.endm
	.endm

	.macro areabndry n
		.$.end =: .
		.bndry n
		.ifne . - .$.end
			. = . - 1
			.byte 0
		.endif
	.endm

	.area _CODE (REL,CON)
	int reset
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
	int no_action
no_action:
	iret
	codebndry
	.area INITIALIZER (REL,CON)
	.area CODE (REL,CON)
	codebndry

reset:
	; PM0044 5.4 Conventions, table 3
	; N.B. PM004 cannot be right. Table 3 disagrees with the section 7 documentation for ADDW.
	; Table 4 has cycle counts for ADDW that disagree with the time graph on the right.
	jra table3
	codebndry
table3:	ldw x,[0x50]
	addw x,#20
	ld a,[0x30]
table3_end:

	; PM0044 5.4.1 Optimized pipeline example, table 6
	; N.B. Table 6 has the final F3 load one cycle too early surely?
	jra table6
	codebndry
table6:	neg a
	xor a, 0x10
	ld a, #20
	sub a, 0x1000
	inc a
	ld xl, a
	srl a
	swap a
	sla 0x15
	cp a, #0xfe
	mov 0x100, #11
	mov 0x101, #22
table6_end:

	; PM0044 5.4.2 Optimize pipeline example - execution from RAM, table 8 (requires copy to RAM)
table8_copy:
	ldw x, #table8_end - table8_start
	jreq table8_copy_end
table8_copy_loop:
	ld a, (table8_start - 1, x)
	ld (0x100 - 1, x), a
	decw x
	jrne table8_copy_loop
table8_copy_end:
	call 0x100

	codebndry
	.area INITIALIZER
	codebndry
table8_start:
	jra table8
	codebndry
table8:	neg a
	xor a, 0x10
	ld a, #20
	sub a, 0x1000
	inc a
	ld xl, a
	srl a
	swap a
	sla 0x15
	cp a, #0xfe
	ret
table8_end:
	codebndry
	.area CODE
	codebndry

	; PM0044 5.4.3 Pipeline with Call/Jump, table 10
	jra table10
	codebndry
table10:
	inc a
	jp label
	ldw x, [0x5432]
label:	neg a
	call label2
	ldw x, [0x5432]
	ldw x, [0x7895]
label2:	incw x
table10_end:

	; PM0044 5.4.4 Pipeline stalled, table 12
	clr a
	ld 0x10, a
	ld 0x20, a		; Table 12 doesn't include any setup...
	jra table12
	codebndry
table12:
	sub sp, #20
	ld a, #20
	btjt 0x10, #5, to	; Table 12 assumes not taken
	inc a
	btjf 0x20, #3, to	; Table 12 assumes taken
	nop
	ldw x, [0x5432]
	ldw x, [0x1234]
to:	incw y
	ld a, (x)
table12_end:

	; DIV tests - available ST docs simply say "2-17 cycles"
	.macro timed_div dividend divisor ?aligned
		jra aligned
		codebndry
aligned:
		ldw x, #dividend
		ld a, #divisor
		div x, a
end_'aligned:
	.endm

	timed_div 0x8000,0x80
	timed_div 0x8000,0x40
	timed_div 0x8000,0x20
	timed_div 0x8000,0x10
	timed_div 0x8000,0x08
	timed_div 0x8000,0x04
	timed_div 0x8000,0x02
	timed_div 0x8000,0x01

	timed_div 0x8000,0x80
	timed_div 0x4000,0x80
	timed_div 0x2000,0x80
	timed_div 0x1000,0x80
	timed_div 0x0800,0x80
	timed_div 0x0400,0x80
	timed_div 0x0200,0x80
	timed_div 0x0100,0x80
	timed_div 0x0080,0x80
	timed_div 0x0040,0x80
	timed_div 0x0020,0x80
	timed_div 0x0010,0x80
	timed_div 0x0008,0x80
	timed_div 0x0004,0x80
	timed_div 0x0002,0x80
	timed_div 0x0001,0x80
	timed_div 0x0000,0x80

	timed_div 63, 8
	timed_div 64, 8
	timed_div 65, 8

	.macro timed_divw dividend divisor ?aligned
		jra aligned
		codebndry
aligned:
		ldw x, #dividend
		ldw y, #divisor
		divw x, y
end_'aligned:
	.endm

	timed_divw 0x8000, 1

	; Interrupted div
	jra test_idiv
	codebndry
test_idiv:
	bset TIM4_IER, #0
	bset TIM4_EGR, #0
	mov TIM4_ARR, #8
	ldw x, #255
	ld a, #10
	mov TIM4_CR1, #0x0f
	div x, a

	; Exercising all code in stm8.cc + inst.cc
	.macro timed_op op operand1 operand2a operand2b ?aligned
		jra aligned
		codebndry
aligned:
		.narg .narg
		.ifeq .narg - 4
				op operand1,operand2a,operand2b
		.else
			.ifeq .narg - 3
				op operand1,operand2a
			.else
				.ifeq .narg - 2
					op operand1
				.else
					op
				.endif
			.endif
		.endif
end_'aligned:
	.endm

	; a, shortmem, longmem,
	; (X), (shortoff,X), (longoff,X)
	; (Y), (shortoff,Y), (longoff,Y)
	; (shortoff,SP)
	; ([shortptr.w],X), ([longptr.w],X)
	; ([shortptr.w],X), ([longptr.w],X)
	; ([shortptr.w],Y)
	.macro timed_class_1 op
		timed_op op a
		timed_op op 0xf5
		timed_op op 0xf5c2
		.irp reg,X,Y
			timed_op op (reg)
			timed_op op (0xf5,reg)
			timed_op op (0xf5c2,reg)
		.endm
		timed_op op (0xf5,SP)
		timed_op op [0xf5]
		timed_op op [0xf5c2]
		timed_op op ([0xf5],X)
		timed_op op ([0xf5c2],X)
		timed_op op ([0xf5],Y)
	.endm

	; a <- #byte, shortmem, longmem,
	;      (X), (shortoff,X), (longoff,X)
	;      (Y), (shortoff,Y), (longoff,Y)
	;      (shortoff,SP)
	;      ([shortptr.w],X), ([longptr.w],X)
	;      ([shortptr.w],X), ([longptr.w],X)
	;      ([shortptr.w],Y)
	.macro timed_class_2 op
		timed_op op a,#0x55
		timed_op op a,0x10
		timed_op op a,0x1000
		.irp reg,X,Y
			timed_op op a,(reg)
			timed_op op a,(0x10,reg)
			timed_op op a,(0x1000,reg)
		.endm
		timed_op op a,(0x10,SP)
		timed_op op a,[0x10]
		timed_op op a,[0x1000]
		timed_op op a,([0x10],X)
		timed_op op a,([0x1000],X)
		timed_op op a,([0x10],Y)
	.endm

	; SP <- #byte
	.macro timed_class_sp_imm op
		timed_op op sp,#0x55
	.endm

	; X, Y
	.macro timed_class_xy op
		timed_op op X
		timed_op op Y
	.endm

	; Bit addressed
	.macro timed_class_bit op
		timed_op op 0x1000,#2
	.endm

	; Inherent
	.macro timed_class_inh op
		timed_op op
	.endm

	timed_class_2 adc
	timed_class_2 add
	timed_op addw X,#0x1000
	timed_op addw X,0x1000
	timed_op addw X,(0x10,SP)
	timed_op addw Y,#0x1000
	timed_op addw Y,0x1000
	timed_op addw Y,(0x10,SP)
	timed_class_sp_imm addw
	timed_class_2 and
	timed_class_bit bccm
	timed_class_2 bcp
	timed_class_bit bcpl
	;break
	timed_class_bit bres
	timed_class_bit bset
	;btjf    - covered by table 12 above
	;btjt    - covered by table 12 above
	;call    - uses common fetchea code
	;callf
	;callr
	timed_class_inh ccf
	timed_class_1 clr
	timed_class_xy clrw
	timed_class_2 cp
	timed_op cpw X,#0x55
	timed_op cpw X,0x10
	timed_op cpw X,0x1000
	timed_op cpw X,(Y)
	timed_op cpw X,(0x10,Y)
	timed_op cpw X,(0x1000,Y)
	timed_op cpw X,(0x10,SP)
	timed_op cpw X,[0x10]
	timed_op cpw X,[0x1000]
	timed_op cpw X,([0x10],Y)
	timed_op cpw Y,([0x1000],X)
	timed_class_1 cpl
	timed_class_xy cplw
	timed_class_1 dec
	timed_class_xy decw
;	;div     - covered above
;	;divw    - covered above
	timed_op exg a,xl
	timed_op exg a,yl
	timed_op exg a,0x1000
	timed_op exgw x,y
	;halt
	timed_class_1 inc
	timed_class_xy incw
	;int
	;iret
	;jp      -covered by table 10 above
	;jpf
	;jra
	;jrxx
	timed_class_2 ld
	timed_op ld 0x10,a
	timed_op ld 0x1000,a
	timed_op ld (X),a
	timed_op ld (0x10,X),a
	timed_op ld (0x1000,X),a
	timed_op ld (Y),a
	timed_op ld (0x10,Y),a
	timed_op ld (0x1000,Y),a
	timed_op ld (0x10,SP),a
	timed_op ld [0x10],a
	timed_op ld [0x1000],a
	timed_op ld ([0x10],X),a
	timed_op ld ([0x1000],X),a
	timed_op ld ([0x10],Y),a
	timed_op ldf a,0x500000
	timed_op ldf a,(0x500000,X)
	timed_op ldf a,(0x500000,Y)
	timed_op ldf a,([0x5000],X)
	timed_op ldf a,([0x5000],Y)
	timed_op ldf a,[0x5000]
	timed_op ldw X,#0x55
	timed_op ldw X,0x10
	timed_op ldw X,0x1000
	timed_op ldw X,(X)
	timed_op ldw X,(0x10,X)
	timed_op ldw X,(0x1000,X)
	timed_op ldw X,(0x10,SP)
	timed_op ldw X,[0x10]
	timed_op ldw X,[0x1000]
	timed_op ldw X,([0x10],X)
	timed_op ldw X,([0x1000],X)
	timed_op mov 0x8000,#0xAA
	timed_op mov 0x80,0x10
	timed_op mov 0x8000,0x1000
	timed_op mul x,a
	timed_class_1 neg
	timed_class_xy negw
	timed_op nop
	timed_class_2 or
	;pop
	;popw
	;push
	;pushw
	timed_class_inh rcf
	;ret
	;retf
;	timed_class_inh rim
	timed_class_1 rlc
	timed_class_xy rlcw
	timed_class_xy rlwa
	timed_class_1 rrc
	timed_class_xy rrcw
	timed_class_xy rrwa
	timed_class_inh rvf
	timed_class_2 sbc
	timed_class_inh scf
;	timed_class_inh sim
	timed_op subw X,#0x5500
	timed_op subw X,0x1000
	timed_op subw X,(0x10,SP)
	timed_op ldw Y,X
	timed_op ldw 0x10,X
	timed_op ldw 0x1000,X
	timed_op ldw (X),Y
	timed_op ldw (0x10,X),Y
	timed_op ldw (0x1000,X),Y
	timed_op ldw (0x10,SP),X
	timed_op ldw [0x10],X
	timed_op ldw [0x1000],X
	timed_op ldw ([0x10],X),Y
	timed_op ldw ([0x1000],X),Y
	timed_class_1 sla
	timed_class_xy slaw
	timed_class_1 sll
	timed_class_xy sllw
	timed_class_1 sra
	timed_class_xy sraw
	timed_class_1 srl
	timed_class_xy srlw
	timed_class_2 sub
	timed_class_sp_imm sub
	timed_class_1 swap
	timed_class_xy swapw
	timed_class_1 tnz
	timed_class_xy tnzw
	;trap
	;wfe
	;wfi
	timed_class_2 xor

	dividend = 0x8000
	.rept 15
		timed_div dividend,0x02
		dividend = dividend >> 1
	.endm

	dividend = 0x10000
	.rept 15
		timed_div dividend-1,0x02
		dividend = dividend >> 1
	.endm

	timed_div 0x2000 0x02
	timed_div 0x2040 0x02
	timed_div 0x2048 0x02
	timed_div 0x2148 0x02
	timed_div 0x21c8 0x02

	timed_div 45831 94

end:
	halt
	.area _DATA
