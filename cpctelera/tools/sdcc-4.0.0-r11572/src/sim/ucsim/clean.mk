# Deleting all files created by building the program
# --------------------------------------------------
clean:
	rm -f *core *[%~] *.[oa] *.so ucsim.map
	rm -f  ucsim$(EXEEXT) relay$(EXEEXT) ucsim.exe relay.exe
	rm -f .[a-z]*~ ptt


# Deleting all files created by configuring or building the program
# -----------------------------------------------------------------
distclean: clean
	rm -f config.cache config.log config.status
	rm -f ddconfig.h main.mk Makefile *.dep packages.mk
	rm -rf autom4te.cache
	rm -f GPATH GRTAGS GSYMS GTAGS


# Like clean but some files may still exist
# -----------------------------------------
mostlyclean: clean


# Deleting everything that can reconstructed by this Makefile. It deletes
# everything deleted by distclean plus files created by bison, etc.
# -----------------------------------------------------------------------
realclean: distclean
