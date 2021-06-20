# tlcs.src/clean.mk
#
# $Id: clean.mk 214 2016-06-18 08:00:47Z  $
#

# Deleting all files created by building the program
# --------------------------------------------------
clean:
	rm -f *core *[%~] *.[oa] *.map
	rm -f .[a-z]*~
	rm -f stlcs$(EXEEXT)


# Deleting all files created by configuring or building the program
# -----------------------------------------------------------------
distclean: clean
	rm -f config.cache config.log config.status
	rm -f Makefile *.dep
	rm -f *.obj *.list *.lst *.hex


# Like clean but some files may still exist
# -----------------------------------------
mostlyclean: clean


# Deleting everything that can reconstructed by this Makefile. It deletes
# everything deleted by distclean plus files created by bison, etc.
# -----------------------------------------------------------------------
realclean: distclean

# End of tlcs.src/clean.mk
