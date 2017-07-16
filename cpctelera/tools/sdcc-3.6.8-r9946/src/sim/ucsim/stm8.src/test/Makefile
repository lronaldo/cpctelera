PRJ		= t1 t2 t3 t4 s0 s1 s2 s3 \
		  stm8l_disc_serial \
		  stm8l_disc_led \
		  stm8l_disc_tim \
		  stm8l_disc_getbl \
		  stm8l_disc_monitor \
		  stm8s_disc_led \
		  stm8s_disc_tim \
		  stm8s_disc_serial \
		  stm8s_disc_getbl \
		  ss sl xtest

all:
	for p in $(PRJ); do \
		$(MAKE) -f $$p.mk ;\
	done

clean:
	for p in $(PRJ); do \
		$(MAKE) -f $$p.mk clean ;\
	done
