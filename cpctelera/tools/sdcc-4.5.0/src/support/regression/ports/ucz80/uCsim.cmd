set error non-classified off
set error unknown_code off
set error memory off
set error stack off
set hw simif outputs 0xff
step 100000000 vclk
state
quit
