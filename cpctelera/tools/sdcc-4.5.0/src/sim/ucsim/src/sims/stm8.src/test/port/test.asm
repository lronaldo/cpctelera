	.area _DATA
count:	.ds 1

	.area _CODE
	int reset
	int unused
	int unused
	int unused
	int unused
	int unused
	int unused
	int irq_exti2 ; Port C external interrupts

unused:
	iret

irq_exti2:
	inc count
	iret

reset:
	ld a, #0
	ld count, a
	rim

loop:
	jp loop
