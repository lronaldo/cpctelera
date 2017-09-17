PRJ		= t0 t1 t2

all:
	for p in $(PRJ); do \
		$(MAKE) -f $$p.mk ;\
	done

clean:
	for p in $(PRJ); do \
		$(MAKE) -f $$p.mk clean ;\
	done
