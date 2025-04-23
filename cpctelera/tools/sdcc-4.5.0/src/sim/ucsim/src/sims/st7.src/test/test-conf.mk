# For asl see: http://john.ccac.rwth-aachen.de:8000/as/

AS = asl
ASFLAGS = -L -h -U

%p:	%.asm
	$(AS) $(ASFLAGS) -olist '$*.lst' -o '$@' '$<'

%.ihx:	%.p
	p2hex -F Intel '$<' '$@'
