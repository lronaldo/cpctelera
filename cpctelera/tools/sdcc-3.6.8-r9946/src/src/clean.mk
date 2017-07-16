CLEANALLPORTS = avr ds390 ds400 hc08 mcs51 pic14 pic16 stm8 z80 xa51

# Deleting all files created by building the program
# --------------------------------------------------
clean:
	rm -f *core *[%~] *.[oa] *.output
	rm -f .[a-z]*~ \#*
	rm -f version.h
	rm -f SDCCy.c SDCCy.h SDCClex.c
	rm -f $(top_builddir)/bin/sdcc$(EXEEXT) sdcc$(EXEEXT)
	for port in $(CLEANALLPORTS) ; do\
	  if [ -f $$port/Makefile ]; then\
	    $(MAKE) -C $$port clean ;\
	  fi;\
	done


# Deleting all files created by configuring or building the program
# -----------------------------------------------------------------
distclean: clean
	rm -f Makefile *.dep
	for port in $(CLEANALLPORTS) ; do\
	  if [ -f $$port/Makefile ]; then\
	    $(MAKE) -C $$port distclean ;\
	  fi;\
	done


# Like clean but some files may still exist
# -----------------------------------------
mostlyclean: clean
	rm -f SDCCy.c
	rm -f SDCCy.h
	rm -f SDCClex.c


# Deleting everything that can reconstructed by this Makefile. It deletes
# everything deleted by distclean plus files created by bison, etc.
# -----------------------------------------------------------------------
realclean: distclean
	rm -f SDCCy.c
	rm -f SDCCy.h
	rm -f SDCClex.c
