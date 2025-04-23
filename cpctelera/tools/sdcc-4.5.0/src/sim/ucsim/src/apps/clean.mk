APPS		= serio ucsim

clean: local_clean sub_clean

local_clean:
	rm -f *~ lib*.a

sub_clean:
	@for app in $(APPS); do \
		$(MAKE) -C $${app}.src -f clean.mk clean ;\
	done

distclean: local_distclean sub_distclean

local_distclean: local_clean

sub_distclean:
	@for app in $(APPS); do \
		$(MAKE) -C $${app}.src -f clean.mk distclean ;\
	done
