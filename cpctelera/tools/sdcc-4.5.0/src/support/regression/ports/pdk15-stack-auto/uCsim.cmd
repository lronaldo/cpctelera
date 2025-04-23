set error non-classified off
set error unknown_code off
set error memory off
set error stack off
set hw simif regs8 0x3f
step 100000000 vclk
state
quit
