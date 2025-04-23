# Test vcd operations
# Note that with the default clock settings on the STM8 we have
# a tick of 125 ns.
set hw vcd output "out/test.vcd"
set hw vcd add rom[0x100]
set hw vcd start
info hw vcd

# For ticks (500 ns) of changing data
set mem rom[0x100] 0
tick
set mem rom[0x100] 1
tick
set mem rom[0x100] 2
tick
set mem rom[0x100] 3
tick

# Now pause for 8 ticks (1000 ns).
# Pause duration is not limited so all the following ticks are
# recorded but not values.
info hw vcd
set hw vcd pause
info hw vcd
set mem rom[0x100] 4
tick
set mem rom[0x100] 5
tick
set mem rom[0x100] 6
tick
set mem rom[0x100] 7
tick
set mem rom[0x100] 8
tick
set mem rom[0x100] 9
tick
set mem rom[0x100] 10
tick
set mem rom[0x100] 11
tick

# Resume. States are updated to current values and recording resumes.
info hw vcd
set hw vcd restart
info hw vcd
set mem rom[0x100] 12
tick
set mem rom[0x100] 13
tick
set mem rom[0x100] 14
tick
set mem rom[0x100] 15
tick

# Pause again for 8 ticks (1000 ns) but this time limit the pause duration
# to 300 ns. Note that since we are limited by tick granularity we will
# record the first 3 ticks (375 ns) then ignore the rest.
info hw vcd
set hw vcd pausetime 300 ns
set hw vcd pause
info hw vcd
# vcd[0] should be clocked (on) until we have exceeded the pause limit.
conf
set mem rom[0x100] 16
tick
conf
set mem rom[0x100] 17
tick
conf
set mem rom[0x100] 18
tick
conf
set mem rom[0x100] 19
tick
conf
set mem rom[0x100] 20
tick
set mem rom[0x100] 21
tick
set mem rom[0x100] 22
tick
set mem rom[0x100] 23
tick

# Resume again. The recorded gap is limited by the pausetime. Sadly there
# is no way to skip time in  VCD so the skipped interval is simply lost
# and the x (time) axis is as if it never happened. There are comments
# placed in the VCD but it's up to the viewer whether they are shown at all.
info hw vcd
set hw vcd restart
info hw vcd
set mem rom[0x100] 24
tick
set mem rom[0x100] 25
tick
set mem rom[0x100] 26
tick

# That's all folks!
info hw vcd
set hw vcd stop
info hw vcd
