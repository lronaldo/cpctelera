clean:

distclean:
	rm -f Makefile
	find . -type d -name out -exec rm -rf {} \;
