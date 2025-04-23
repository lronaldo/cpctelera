# Deleting all files created by building the program
# --------------------------------------------------
clean:
	rm -f *core *[%~] *.[oa]
	rm -f .[a-z]*~
	make -C bin clean

# Deleting all files created by configuring or building the program
# -----------------------------------------------------------------
distclean: clean
	make -C bin distclean
	rm -f config.cache config.log config.status Makefile Makefile.common
	rm -f sdccconf.h main.mk *.dep *.depcc *.depcxx
	rm -f ports.all ports.build

# Like clean but some files may still exist
# -----------------------------------------
mostlyclean: clean


# Deleting everything that can reconstructed by this Makefile. It deletes
# everything deleted by distclean plus files created by bison, etc.
# -----------------------------------------------------------------------
realclean: distclean
