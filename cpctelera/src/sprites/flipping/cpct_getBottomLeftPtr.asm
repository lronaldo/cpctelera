;; DE => Pointer
;; BC => Height (C)

;; Save Memory Bank
ld	a, #0xC0	;; [2]
and	d		;; [1] A = Bits 14-15 of DE (Memory 16K Bank 00-11)
ex 	af, af'	;; [1] Save A into A'

;; He1 = Height - 1
;; DE += 0x800 * (He1 % 8)
dec	c		;; [1] C-- (C = Height-1 = He1)
ld 	a, #0x07	;; [2]
and 	c		;; [1] A = He1 % 8
rlca			;; [1] |
rlca			;; [1] |
rlca			;; [1] | A = 8 ( He1 % 8 )

add 	d		;; [1] |
ld 	d, a		;; [1] | DE += 0x100 * 8 (He1 % 8)

;; HL = 80 * int(He1 / 8)
;; (note 80 = 0x50)
ld 	 a, #0xF8	;; [2]
and 	 c		;; [1] A = 8 * int(He1 / 8)
ld	 h, #0x00	;; [2]
ld 	 l, c		;; [1] HL =  8 * int(He1 / 8)
add	hl, hl	;; [3] HL = 16 * int(He1 / 8)
ld	 b, h		;; [1]
ld	 c, l		;; [1] BC = 16 * int(He1 / 8)
add	hl, hl	;; [3] HL = 32 * int(He1 / 8)
add	hl, hl	;; [3] HL = 64 * int(He1 / 8)
add	hl, bc	;; [3] HL = 80 * int(He1 / 8)

;; DE = 0x800 * (He1 % 8) + 0x50 * int(He1 / 8)
add	hl, de	;; [3]

;; End of page?
ex 	af, af'	;; [1] Restore A (Saved memory 16K bank)
xor	 h		;; [1]
and   #0xC0		;; [2] Is HL pointing to the same 16K bank as DE at the start?
ret 	 z		;; [2/4] Yes, return. No, cycle.

;; Cycle and jump to next row
ld	bc, #0xC050	;; [3] Make memory cycle and add 0x50 to jump to the next row
add	hl, bc 	;; [3]
ret			;; [3] Return


