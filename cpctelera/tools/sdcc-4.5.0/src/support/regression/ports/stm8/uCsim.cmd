set error non-classified off
set error unknown_code off
set error memory off
set error stack off
set hw simif rom 0x7fff
expr /n sp_limit=0x1000
expr /n uart1_on=0
expr /n uart3_on=0
expr /n dreg0_on=0
expr /n dport0_on=0
expr /n flash0_on=0
expr /n itc0_on=0
expr /n pa0_on=0
expr /n pb0_on=0
expr /n pc0_on=0
expr /n pd0_on=0
expr /n pe0_on=0
expr /n pf0_on=0
expr /n pg0_on=0
expr /n ph0_on=0
expr /n pi0_on=0
expr /n tim11_on=0
expr /n tim22_on=0
expr /n tim33_on=0
expr /n tim44_on=0
expr /n vcd0_on=0
step 100000000 vclk
state
quit
