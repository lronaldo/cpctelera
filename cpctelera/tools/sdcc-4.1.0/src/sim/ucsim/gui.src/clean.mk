# uCsim gui.src/clean.mk

PKGS		= serio.src

# Deleting all files created by building the program
# --------------------------------------------------
clean_local:
	rm -f *core *[%~] *.[oa] a
	rm -f .[a-z]*~

clean: clean_local
	@for pkg in $(PKGS); do\
	  $(MAKE) -C $$pkg -f clean.mk clean;\
	done


# Deleting all files created by configuring or building the program
# -----------------------------------------------------------------
distclean_local: clean_local
	rm -f config.cache config.log config.status
	rm -f Makefile *.dep

distclean: distclean_local
	@for pkg in $(PKGS); do\
	  $(MAKE) -C $$pkg -f clean.mk distclean;\
	done


# Like clean but some files may still exist
# -----------------------------------------
mostlyclean: clean_local
	@for pkg in $(PKGS); do\
	  $(MAKE) -C $$pkg -f clean.mk mostlyclean;\
	done


# Deleting everything that can reconstructed by this Makefile. It deletes
# everything deleted by distclean plus files created by bison, etc.
# -----------------------------------------------------------------------
realclean: distclean_local
	@for pkg in $(PKGS); do\
	  $(MAKE) $$pkg -f clean.mk realclean;\
	done

# End of gui.src/clean.mk
