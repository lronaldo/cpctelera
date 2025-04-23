clean:
	rm -f *core *[%~] *.[oa] *.output
	rm -f .[a-z]*~ \#*
	rm -f $(TARGET)

distclean realclean: clean
	rm -f *.dep *.depcc *.depcxx
	rm -f config.* Makefile
