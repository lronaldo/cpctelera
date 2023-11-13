CLEANALLPORTS = ds390 ds400 gbz80 hc08 s08 huge large mcs51 medium pic14 pic16 small z80 z180 ez80_z80 z80n r2k r3ka tlcs90 stm8 stm8-large pdk13 pdk14 pdk15 pdk15-stack-auto
include $(srcdir)/incl.mk

# Deleting all files created by building the program
# --------------------------------------------------
clean:
	rm -f *core *[%~] *.[oa] *.rel *.lst *.sym *.asm *.ihx *.dump* *.cdb
	rm -f .[a-z]*~
	rm -f *.dump*
	rm -rf build
	for model in $(MODELS); do\
	  if [ -d $$model ]; then\
	    $(MAKE) -C $$model clean ;\
	  fi;\
	  rm -rf $${model}-xstack-auto $${model}-stack-auto ;\
	done
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


# Deleting everything that can reconstructed by this Makefile. It deletes
# everything deleted by distclean plus files created by bison, etc.
# -----------------------------------------------------------------------
realclean: distclean
