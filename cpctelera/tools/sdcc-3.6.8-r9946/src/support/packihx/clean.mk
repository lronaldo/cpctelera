clean:
	rm -f *core *[%~] *.[oa] *.output
	rm -f .[a-z]*~ \#*
	rm -f $(TARGET)

distclean realclean: clean
	rm -f config.* Makefile
