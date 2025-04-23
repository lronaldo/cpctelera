set error non-classified off
set error unknown_code off
set error memory off
set error stack off
set hw simif rom 0x7fff
vcd0_on=0
dreg0_on=0
step 100000000 vclk
state
quit
