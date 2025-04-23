	.area _DATA
loc1:	.ds 1
loc2:	.ds 1

	.area _CODE
	int reset

reset:
	ld a, #0
	ld loc1, a
	ld loc2, a

loop:
	jp loop
