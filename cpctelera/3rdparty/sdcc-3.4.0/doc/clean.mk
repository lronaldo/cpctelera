clean:
	rm -rf $(MANUAL).html $(TSS).html $(CDB).html
	rm -rf $(MANUAL).txt $(TSS).txt $(CDB).txt \
		*.pdf *.tex *.aux *.dvi *.idx *.ilg *.out\
		*.ind *.log *.toc *~ \#* *.ps */*.css */*.pl *.gif core *.glo
	rm -rf sdcc-doc sdcc-doc.tar.bz2
	if [ "$(srcdir)" != "." ]; then rm -f *.lyx; fi

# Deleting all files created by configuring or building the program
# -----------------------------------------------------------------
distclean: clean
	rm -f Makefile

# Like clean but some files may still exist
# -----------------------------------------
mostlyclean: clean

# Deleting everything that can reconstructed by this Makefile. It deletes
# everything deleted by distclean plus files created by bison, etc.
# -----------------------------------------------------------------------
realclean: distclean
