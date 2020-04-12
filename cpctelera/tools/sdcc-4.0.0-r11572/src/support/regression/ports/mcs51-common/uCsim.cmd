set error non-classified off
set error unknown_code off
set error memory off
set error stack off
break xram r 0x7654
run
state
quit
