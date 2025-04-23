# We don't want auto-generated labels in the output.
set option analyzer false

step

# The pipelined examples from PM0044 5.4 are run with fold-on-flush off
# so that they can be compared with the documentation easily.
set hw cpu pipetrace fold off

step
set hw cpu pipetrace title "PM0044 5.4 Conventions, table 3"
set hw cpu pipetrace start "out/test.table3.html"
set hw cpu pipetrace data "<h2>Errata</h2>"
set hw cpu pipetrace data "<ul>"
set hw cpu pipetrace data "<li>The LDW shown in PM0044 table 3 is missing a decode cycle."
set hw cpu pipetrace data "    <br/>"
set hw cpu pipetrace data "    (Confirmed on HW)"
set hw cpu pipetrace data "    </li>"
set hw cpu pipetrace data "<li>The ADDW shown in PM0044 table 3 has one too many decode cycles."
set hw cpu pipetrace data "    <br/>"
set hw cpu pipetrace data "    (Confirmed on HW)"
set hw cpu pipetrace data "    </li>"
set hw cpu pipetrace data "<li>The LD shown in PM0044 table 3 is missing a decode cycle."
set hw cpu pipetrace data "</ul>"
step 3
set hw cpu pipetrace stop

step
set hw cpu pipetrace title "PM0044 5.4.1 Optimized pipeline example - execution from flash, table 6"
set hw cpu pipetrace start "out/test.table6.html"
step 12
set hw cpu pipetrace stop

break 0x0100
cont

step
set hw cpu pipetrace title "PM0044 5.4.2 Optimize pipeline example - execution from RAM, table 8"
set hw cpu pipetrace start "out/test.table8.html"
step 10
set hw cpu pipetrace stop
step


step
set hw cpu pipetrace title "PM0044 5.4.3 Pipeline with Call/Jump, table 10"
set hw cpu pipetrace start "out/test.table10.html"
set hw cpu pipetrace data "<p>PM0044 table 10 shows a fetch stall in the first execution cycle"
set hw cpu pipetrace data "of the call (cycle 7) however it should be possible for a fetch"
set hw cpu pipetrace data "to take place since pushing the return address only busies the"
set hw cpu pipetrace data "data bus (and STM8 is a Harvard architecture with unified address"
set hw cpu pipetrace data "space so the data and program are separate buses).</p>"
set hw cpu pipetrace data "<p>Also note that since the flush happens on the last execute cycle"
set hw cpu pipetrace data "of the call (unlike the jp) there is no overlap and we mark"
set hw cpu pipetrace data "the following cycle as a decode stall. Technically this is correct"
set hw cpu pipetrace data "but table 10 does not and says the call takes 3 cycles which"
set hw cpu pipetrace data "ignores the unavoidable stall cycle. The later instruction documentation"
set hw cpu pipetrace data "for call says it takes 4 cycles which is presumed to include the"
set hw cpu pipetrace data "stall cycle that follows it.</p>"
step 5
set hw cpu pipetrace stop


step 4
set hw cpu pipetrace title "PM0044 5.4.4 Pipeline stalled, table 12"
set hw cpu pipetrace start "out/test.table12.html"
set hw cpu pipetrace data "<h2>Errata</h2>"
set hw cpu pipetrace data "<ul>"
set hw cpu pipetrace data "<li>Table 12 shows the BTJT as taking 1 decode, 2 execute cycles but the"
set hw cpu pipetrace data "    later documentation for the instruction implies the second execution"
set hw cpu pipetrace data "    cycle is only used if the branch is taken - and this isn't."
set hw cpu pipetrace data "    </li>"
set hw cpu pipetrace data "<li>Table 12 has a jump in time (cycles) from 4 straight to 7."
set hw cpu pipetrace data "    </li>"
set hw cpu pipetrace data "<li>The last instruction is shown one cycle early in table 12. The decode stall"
set hw cpu pipetrace data "    should line up with the execution cycle of the previous instruction"
set hw cpu pipetrace data "    as the description above the table says."
set hw cpu pipetrace data "    </li>"
set hw cpu pipetrace data "</ul>"
step 7
set hw cpu pipetrace stop


# Everything else needs to be folded on flush or it could go sideways very quickly.
set hw cpu pipetrace fold on

set hw cpu pipetrace title "DIV tests - available ST docs simply say '2-17 cycles'"
set hw cpu pipetrace start "out/test.div.html"
set hw cpu pipetrace data "<p>The stated cycle count would seem to imply binary long division and"
set hw cpu pipetrace data "   this is how the STM8 emulator in ucsim currently treats div for."
set hw cpu pipetrace data "   cycle counting. However the cycles measured on actual hardware
set hw cpu pipetrace data "   suggest this is not correct."
set hw cpu pipetrace data "</p>"

set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3

set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3

set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3
set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3

set hw cpu pipetrace pause
step
set hw cpu pipetrace resume
step 3

set hw cpu pipetrace stop


step 7

set hw cpu pipetrace title "Interrupted div"
set hw cpu pipetrace start "out/test.int_div.html"
set hw cpu pipetrace data "<p>(Not currently implemented.)</p>"
# FIXME: once we have timer interrupts emulated correctly we need to add
# the interrupt instructions to the following step count.
#step 4
step
set hw cpu pipetrace stop

step

set hw cpu pipetrace title "All instructions and addressing modes"
set hw cpu pipetrace start "out/test.instrs.html"
set hw cpu pipetrace data "<p>Note that timings given in PM0044 assume a 1 cycle overlap"
set hw cpu pipetrace data "with the previous instruction and are specified as being"
set hw cpu pipetrace data "one cycle less than they are in the no prefetch/stall case."
set hw cpu pipetrace data "Instructions that flush the prefetch buffer such as jumps"
set hw cpu pipetrace data "prevent the overlap with the following instruction. The cycle"
set hw cpu pipetrace data "count for these instructions includes the extra cycles for"
set hw cpu pipetrace data "the unavoidable fetches and stalls that follow them.</p>"
cont
set hw cpu pipetrace stop
