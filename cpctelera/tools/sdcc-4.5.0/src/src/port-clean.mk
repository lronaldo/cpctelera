clean:
	rm -f $(LIB) *.o *~ port.a *.lst *.asm *.sym *~ *.cdb *.dep *.depcc *.depcxx *.rul

distclean: clean
	rm -f Makefile
