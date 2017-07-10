clean:
	rm -f *core *[%~] *.[oa] *.output
	rm -f .[a-z]*~ \#*
	rm -f $(top_builddir)/bin/sdcclib$(EXEEXT)

distclean realclean: clean
	rm -f Makefile
