# libc/large Makefile

all:
	$(MAKE) -C ../mcs51 all
	cp ../mcs51/*.rel .
	touch dummy.rel

clean:
	rm -f *.rel *.sym *.lst *.asm *~ *.lib

distclean: clean
	rm -f Makefile
