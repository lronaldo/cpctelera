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
	bset loc1, #0
	bres loc1, #7
	inc loc2

	bset loc1, #1
	bres loc1, #0
	inc loc2

	bset loc1, #2
	bres loc1, #1
	inc loc2

	bset loc1, #3
	bres loc1, #2
	inc loc2

	bset loc1, #4
	bres loc1, #3
	inc loc2

	bset loc1, #5
	bres loc1, #4
	inc loc2

	bset loc1, #6
	bres loc1, #5
	inc loc2

	bset loc1, #7
	bres loc1, #6
	inc loc2

	jp loop
