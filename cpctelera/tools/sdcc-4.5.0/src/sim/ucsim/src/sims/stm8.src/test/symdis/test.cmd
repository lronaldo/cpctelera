# Set some core vars
var CLK_PCKENR1       rom[0x50c7][7:0]
var CLK_PCKENR1_UART1 rom[0x50c7].3
var CLK_PCKENR1_TIM2  rom[0x50c7].5
var UART1_SR          rom[0x5230]
var UART1_SR_TXE      rom[0x5230].7
var UART1_SR_TC       rom[0x5230].6
var UART1_SR_RXNE     rom[0x5230].5

dump rom[0x00] 0x3d
dc 0x8000 0x082a4 
