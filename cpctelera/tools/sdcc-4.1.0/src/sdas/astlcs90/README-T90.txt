TLCS90 Port by Rainer Keuchel

The assembler is tested and works. I compared output from ast90 and asl.
A test prog was run on a TMP90C041.


The TLCS90 uses the Z80 instruction set but has different opcode encodings.

The TLCS90 usually has internal ram and io from 0xFF00-0xFFFF.
These memory locations can be accessed by a one byte address
offset from 0xFF00, which reduces instruction size.

The following extra instructions are available:

mul     hl,x
div	hl,x

ldw	load word

incx	only used for direct mem/ports?
decx	only used for direct mem/ports?

incw	only memory?
decw	only memory?

callr


The following instructions are missing:

rst

<io instructions>

**********************************************************************

NOTES:

"(hl+a)"	must currently be written as "a(hl)"
"ex af,af'" 	must currently be written as "ex af, af"
