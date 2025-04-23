#dump 0x5000 0x501d

# Ports are set to input on reset so if we zero the pins then set
# the ODR both pins and IDR should remain zero.
set mem pa_pins 0x00
set mem PA_ODR 0xff
dump pa_pins PA_IDR

# Now set all the pins then set every other bit of the data direction register.
set mem pa_pins 0xff
set mem PA_DDR 0b10101010

# For the pins set to output the pins and IDR bits shadow the ODR data...
dump pa_pins PA_IDR
set mem pa_pins 0x00
dump pa_pins PA_IDR

# ...and cannot be changed via the IDR...
set mem PA_IDR 0x00

# ...but can be via the ODR.
set mem PA_ODR 0x00
dump pa_pins

# Reset back to defaults. Note that only the pa_pins output are affected.
reset
dump pa_pins PA_ODR PA_IDR PA_DDR PA_CR1 PA_CR2


# Adjust the clock to a suitable speed
set mem CLK_CKDIVR 0b00000001

# Our test VCD provides input on pc_pins.2 and pc_pins.5
# (these should already be inputs after reset, of course)
set mem pc_pins.2 0
set mem pc_pins.5 0

# For which we'll take interrupts on either edge initially
set mem EXTI_CR1[5:4] 0b11
set mem PC_CR2.2 1
set mem PC_CR2.5 1

# So this is our state now
dump pc_pins PC_ODR PC_IDR PC_DDR PC_CR1 PC_CR2 EXTI_CR1 EXTI_CR2 rom[0x50a3][7:0]

# Start the vcd (we set the input file via the Makefile) and
# have it stop execution each time an event occurs.
set hw vcd[0] start
set hw vcd[0] break

# The VCD data starts at time 0 which will, by default, align
# with the current simulator time. We can adjust that by setting
# starttime. This can be done before or after starting the vcd,
# even part way through. Here we push the vcd 50 µs into the
# future.
set hw vcd[0] starttime 50000 ns

# Set up a vcd to monitor what happens
set hw vcd new 1
set hw vcd[1] output "out/test.vcd"
set hw vcd[1] add PC_IDR.5
set hw vcd[1] add PC_IDR.2
set hw vcd[1] add 0x0000
set hw vcd[1] add rom[0x50a3].2
set hw vcd[1] add EXTI_CR1[5:4]
set hw vcd[1] start

# Firstly either edge
run
cont
cont
cont
cont

# Then on the rising edge
set mem EXTI_CR1[5:4] 0b01
cont
cont
cont
cont

# Then on the falling edge
set mem EXTI_CR1[5:4] 0b10
cont
cont
cont
cont
cont

# The rest as level (0) triggered
set mem EXTI_CR1[5:4] 0b00
cont
cont
cont
cont
cont
cont
cont
cont
cont
cont
cont
cont
cont
