#include "gen.h"

DEC( 00, NOP		, "nop"		, ' ', 1, false, 1, (void*)"8A421" )
// 01 undefined
DEC( 02, OUTLB		, "outl bus,a"	, ' ', 1, false, 2, (void*)"8----" ) // 48
DEC( 02, OUTDBB		, "out dbb,a"	, ' ', 1, false, 2, (void*)"-A4--" ) //41A,41
DEC( 03, ADDI8		, "add a,'i8'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( 04, JMP0   	, "jmp 'a11'"   , ' ', 2, false, 2, (void*)"8A421" )
DEC( 05, ENI    	, "en i"	, ' ', 1, false, 1, (void*)"8A421" )
// 06 undefined
DEC( 07, DECA		, "dec a"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 08, INS		, "ins a,bus"	, ' ', 1, false, 2, (void*)"8----" ) //48
DEC( 08, INP0		, "in a,p0"	, ' ', 1, false, 2, (void*)"---21" ) //21,22
DEC( 09, IN1    	, "in a,p1"     , ' ', 1, false, 2, (void*)"8A421" )
DEC( 0a, IN2    	, "in a,p2"     , ' ', 1, false, 2, (void*)"8A421" )
// 0b undefined
DEC( 0c, MOVDAP4	, "movd a,p4"	, ' ', 1, false, 2, (void*)"8A421" )
DEC( 0d, MOVDAP5	, "movd a,p5"	, ' ', 1, false, 2, (void*)"8A421" )
DEC( 0e, MOVDAP6	, "movd a,p6"	, ' ', 1, false, 2, (void*)"8A421" )
DEC( 0f, MOVDAP7	, "movd a,p7"	, ' ', 1, false, 2, (void*)"8A421" )

DEC( 10, INCIR0 	, "inc @r0"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 11, INCIR1 	, "inc @r1"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 12, JB0		, "jb0 'a8'"	, ' ', 2, false, 2, (void*)"8A4--" ) //48,41A,41
DEC( 13, ADCI8		, "addc a,'i8'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( 14, CALL0  	, "call 'a11'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( 15, DISI		, "dis i"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 16, JTF		, "jtf 'a8'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( 17, INCA   	, "inc a"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 18, INCR0		, "inc r0"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 19, INCR1		, "inc r1"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 1a, INCR2		, "inc r2"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 1b, INCR3		, "inc r3"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 1c, INCR4		, "inc r4"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 1d, INCR5		, "inc r5"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 1e, INCR6		, "inc r6"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 1f, INCR7		, "inc r7"	, ' ', 1, false, 1, (void*)"8A421" )

DEC( 20, XCHIR0		, "xch a,@r0"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 21, XCHIR1		, "xch a,@r1"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 22, INDBB		, "in a,dbb"	, ' ', 1, false, 2, (void*)"-A4--" ) //41A,41
DEC( 23, MOVAI8		, "mov a,'i8'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( 24, JMP1   	, "jmp 'a11'"   , ' ', 2, false, 2, (void*)"8A421" )
DEC( 25, ENTCNTI 	, "en tcnti"	, ' ', 1, false, 1, (void*)"8A42-" ) //48,41A,41,22
DEC( 26, JNT0		, "jnt0 'a8'"	, ' ', 2, false, 2, (void*)"8A42-" ) //48,41A,41,22
DEC( 27, CLRA		, "clr a"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 28, XCHR0		, "xch a,r0"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 29, XCHR1		, "xch a,r1"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 2a, XCHR2		, "xch a,r2"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 2b, XCHR3		, "xch a,r3"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 2c, XCHR4		, "xch a,r4"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 2d, XCHR5		, "xch a,r5"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 2e, XCHR6		, "xch a,r6"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 2f, XCHR7		, "xch a,r7"	, ' ', 1, false, 1, (void*)"8A421" )

DEC( 30, XCHDIR0	, "xchd a,@r0"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 31, XCHDIR1	, "xchd a,@r1"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 32, JB1		, "jb1 'a8'"	, ' ', 2, false, 2, (void*)"8A4--" ) //48,41A,41
// 33 undefined
DEC( 34, CALL1  	, "call 'a11'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( 35, DISTCNTI	, "dis tcnti"	, ' ', 1, false, 1, (void*)"8A42-" ) //48,41A,41,22
DEC( 36, JT0		, "jt0 'a8'"	, ' ', 2, false, 2, (void*)"8A42-" ) //48,41A,41,22
DEC( 37, CPLA		, "cpl a"	, ' ', 1, false, 1, (void*)"8A421" )
// 38 undefined
// 39 OUTL P1,A
// 3a OUTL P2,A
// 3b undefined
DEC( 3c, MOVDP4A	, "movd p4,a"	, ' ', 1, false, 2, (void*)"8A421" )
DEC( 3d, MOVDP5A	, "movd p5,a"	, ' ', 1, false, 2, (void*)"8A421" )
DEC( 3e, MOVDP6A	, "movd p6,a"	, ' ', 1, false, 2, (void*)"8A421" )
DEC( 3f, MOVDP7A	, "movd p7,a"	, ' ', 1, false, 2, (void*)"8A421" )

DEC( 40, ORLIR0		, "orl a,@r0"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 41, ORLIR1		, "orl a,@r1"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 42, MOVAT		, "mov a,t"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 43, ORLI8		, "orl a,'i8'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( 44, JMP2   	, "jmp 'a11'"   , ' ', 2, false, 2, (void*)"8A421" )
DEC( 45, STRTCNT	, "strt cnt"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 46, JNT1		, "jnt1 'a8'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( 47, SWAPA		, "swap a"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 48, ORLR0		, "orl a,r0"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 49, ORLR1		, "orl a,r1"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 4a, ORLR2		, "orl a,r2"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 4b, ORLR3		, "orl a,r3"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 4c, ORLR4		, "orl a,r4"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 4d, ORLR5		, "orl a,r5"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 4e, ORLR6		, "orl a,r6"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 4f, ORLR7		, "orl a,r7"	, ' ', 1, false, 1, (void*)"8A421" )

DEC( 50, ANLIR0		, "anl a,@r0"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 51, ANLIR1		, "anl a,@r1"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 52, JB2		, "jb2 'a8'"	, ' ', 2, false, 2, (void*)"8A4--" ) //48,41A,41
DEC( 53, ANLI8		, "anl a,'i8'"	, ' ', 2, false, 1, (void*)"8A421" )
DEC( 54, CALL2  	, "call 'a11'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( 55, STRTT		, "strt t"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 56, JT1		, "jt1 'a8'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( 57, DAA		, "da a"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 58, ANLR0		, "anl a,r0"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 59, ANLR1		, "anl a,r1"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 5a, ANLR2		, "anl a,r2"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 5b, ANLR3		, "anl a,r3"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 5c, ANLR4		, "anl a,r4"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 5d, ANLR5		, "anl a,r5"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 5e, ANLR6		, "anl a,r6"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 5f, ANLR7		, "anl a,r7"	, ' ', 1, false, 1, (void*)"8A421" )

DEC( 60, ADDIR0		, "add a,@r0"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 61, ADDIR1		, "add a,@r1"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 62, MOVTA		, "mov t,a"	, ' ', 1, false, 1, (void*)"8A421" )
// 63 undefined
DEC( 64, JMP3   	, "jmp 'a11'"   , ' ', 2, false, 2, (void*)"8A421" )
DEC( 65, STOPTCNT	, "stop tcnt"	, ' ', 1, false, 1, (void*)"8A421" )
// 66 undefined
DEC( 67, RRC		, "rrc a"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 68, ADDR0		, "add a,r0"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 69, ADDR1		, "add a,r1"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 6a, ADDR2		, "add a,r2"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 6b, ADDR3		, "add a,r3"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 6c, ADDR4		, "add a,r4"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 6d, ADDR5		, "add a,r5"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 6e, ADDR6		, "add a,r6"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 6f, ADDR7		, "add a,r7"	, ' ', 1, false, 1, (void*)"8A421" )

DEC( 70, ADCIR0		, "adc a,@r0"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 71, ADCIR1		, "adc a,@r1"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 72, JB3		, "jb3 'a8'"	, ' ', 2, false, 2, (void*)"8A4--" ) //48,41A,41
DEC( 74, CALL3  	, "call 'a11'"	, ' ', 2, false, 2, (void*)"8A421" )
// 75 ENT0 CLK 48
DEC( 76, JF1		, "jf1 'a8'"	, ' ', 2, false, 2, (void*)"8A4--" ) //48,41A,41
DEC( 77, RR		, "rr a"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 78, ADCR0		, "adc a,r0"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 79, ADCR1		, "adc a,r1"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 7a, ADCR2		, "adc a,r2"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 7b, ADCR3		, "adc a,r3"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 7c, ADCR4		, "adc a,r4"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 7d, ADCR5		, "adc a,r5"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 7e, ADCR6		, "adc a,r6"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( 7f, ADCR7		, "adc a,r7"	, ' ', 1, false, 1, (void*)"8A421" )

DEC( 80, MOVXAIR0	, "movx a,@r0"	, ' ', 1, false, 2, (void*)"8----" ) //48
// 80 RAD 22
DEC( 81, MOVXAIR1	, "movx a,@r1"	, ' ', 1, false, 2, (void*)"8----" ) //48
// 82 undefined
DEC( 83, RET		, "ret"		, ' ', 1, false, 2, (void*)"8A421" )
DEC( 84, JMP4   	, "jmp 'a11'"   , ' ', 2, false, 2, (void*)"8A421" )
DEC( 85, CLRF0		, "clr f0"	, ' ', 1, false, 1, (void*)"8A4--" ) //48,41A,41
// 85 SEL AN0 22
// 86 JNI 48
// 87 undefined
// 88 ORL BUS,#i8 48
// 89 ORL P1,#i8 48,41A,41
// 8a ORL P2,#i8 48,41A,41
// 8b undefined
DEC( 8c, ORLDP4A	, "orld p4,a"	, ' ', 1, false, 2, (void*)"8A421" )
DEC( 8d, ORLDP5A	, "orld p5,a"	, ' ', 1, false, 2, (void*)"8A421" )
DEC( 8e, ORLDP6A	, "orld p6,a"	, ' ', 1, false, 2, (void*)"8A421" )
DEC( 8f, ORLDP7A	, "orld p7,a"	, ' ', 1, false, 2, (void*)"8A421" )

DEC( 90, MOVXIR0A	, "movx @r0,a"	, ' ', 1, false, 2, (void*)"8----" ) //48
// 90 OUTL P0,A 22,21
// 90 MOV STS,A 41A
DEC( 91, MOVXIR1A	, "movx @r1,a"	, ' ', 1, false, 2, (void*)"8----" ) //48
DEC( 92, JB4		, "jb4 'a8'"	, ' ', 2, false, 2, (void*)"8A4--" ) //48,41A,41
DEC( 93, RETR		, "retr"	, ' ', 1, false, 2, (void*)"8A4--" ) //48,41A,41
DEC( 94, CALL4  	, "call 'a11'"	, ' ', 2, false, 2, (void*)"8--2-" ) //48,22
DEC( 95, CPLF0		, "cpl f0"	, ' ', 1, false, 1, (void*)"8A4--" ) //48,41A,41
DEC( 96, JNZ		, "jnz 'a8'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( 97, CLRC		, "clr c"	, ' ', 1, false, 1, (void*)"8A421" )
// 98 ANL BUS,#i8 48
// 99 ANL P1,#i8 48,41A,41
// 9a ANL P2,#i8 48,41A,41
DEC( 9c, ANLDP4A	, "anl p4,a"	, ' ', 1, false, 2, (void*)"8A421" )
DEC( 9d, ANLDP5A	, "anl p5,a"	, ' ', 1, false, 2, (void*)"8A421" )
DEC( 9e, ANLDP6A	, "anl p6,a"	, ' ', 1, false, 2, (void*)"8A421" )
DEC( 9f, ANLDP7A	, "anl p7,a"	, ' ', 1, false, 2, (void*)"8A421" )

DEC( a0, MOVIR0A	, "mov @r0,a"	, ' ', 1, false, 2, (void*)"8A421" )
DEC( a1, MOVIR1A	, "mov @r1,a"	, ' ', 1, false, 2, (void*)"8A421" )
// a2 undefined
DEC( a3, MOVPAIA	, "movp a,@a"	, ' ', 1, false, 2, (void*)"8A421" )
DEC( a4, JMP5   	, "jmp 'a11'"   , ' ', 2, false, 2, (void*)"8A421" )
DEC( a5, CLRF1		, "clr f1"	, ' ', 1, false, 1, (void*)"8A4--" ) //48,41A,41
// a6 undefined
DEC( a7, CPLC		, "cpl c"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( a8, MOVR0A		, "mov r0,a"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( a9, MOVR1A		, "mov r1,a"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( aa, MOVR2A		, "mov r2,a"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( ab, MOVR3A		, "mov r3,a"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( ac, MOVR4A		, "mov r4,a"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( ad, MOVR5A		, "mov r5,a"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( ae, MOVR6A		, "mov r6,a"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( af, MOVR7A		, "mov r7,a"	, ' ', 1, false, 1, (void*)"8A421" )

DEC( b0, MOVIR0I8	, "mov @r0,'i8'", ' ', 2, false, 2, (void*)"8A421" )
DEC( b1, MOVIR1I8	, "mov @r1,'i8'", ' ', 2, false, 2, (void*)"8A421" )
DEC( b2, JB5		, "jb5 'a8'"	, ' ', 2, false, 2, (void*)"8A4--" ) //48,41A,41
DEC( b3, JMPPIA		, "jmpp @a"	, ' ', 1, false, 2, (void*)"8A421" )
DEC( b4, CALL5  	, "call 'a11'"	, ' ', 2, false, 2, (void*)"8--2-" ) //48,22
DEC( b5, CPLF1		, "cpl f1"	, ' ', 1, false, 1, (void*)"8A4--" ) //48,41A,41
DEC( b6, JF0		, "jf0 'a8'"	, ' ', 2, false, 2, (void*)"8A4--" ) //48,41A,41
// b7 undefined
DEC( b8, MOVR0I8	, "mov r0,'i8'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( b9, MOVR1I8	, "mov r1,'i8'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( ba, MOVR2I8	, "mov r2,'i8'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( bb, MOVR3I8	, "mov r3,'i8'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( bc, MOVR4I8	, "mov r4,'i8'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( bd, MOVR5I8	, "mov r5,'i8'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( be, MOVR6I8	, "mov r6,'i8'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( bf, MOVR7I8	, "mov r7,'i8'"	, ' ', 2, false, 2, (void*)"8A421" )

// c0 undefined
// c1 undefined
// c2 undefined
// c3 undefined
DEC( c4, JMP6   	, "jmp 'a11'"   , ' ', 2, false, 2, (void*)"8--2-" ) //48,22
DEC( c5, SELRB0		, "sel rb0"	, ' ', 1, false, 1, (void*)"8A4--" ) //48,41A,41
DEC( c6, JZ		, "jz 'a8'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( c7, MOVAF		, "mov a,psw"	, ' ', 1, false, 1, (void*)"8A4--" ) //48,41A,41
DEC( c8, DECR0		, "dec r0"	, ' ', 1, false, 1, (void*)"8A4--" ) //48,41A,41
DEC( c9, DECR1		, "dec r1"	, ' ', 1, false, 1, (void*)"8A4--" ) //48,41A,41
DEC( ca, DECR2		, "dec r2"	, ' ', 1, false, 1, (void*)"8A4--" ) //48,41A,41
DEC( cb, DECR3		, "dec r3"	, ' ', 1, false, 1, (void*)"8A4--" ) //48,41A,41
DEC( cc, DECR4		, "dec r4"	, ' ', 1, false, 1, (void*)"8A4--" ) //48,41A,41
DEC( cd, DECR5		, "dec r5"	, ' ', 1, false, 1, (void*)"8A4--" ) //48,41A,41
DEC( ce, DECR6		, "dec r6"	, ' ', 1, false, 1, (void*)"8A4--" ) //48,41A,41
DEC( cf, DECR7		, "dec r7"	, ' ', 1, false, 1, (void*)"8A4--" ) //48,41A,41

DEC( d0, XRLIR0		, "xrl a,@r0"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( d1, XRLIR1		, "xrl a,@r1"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( d2, JB6		, "jb6 'a8'"	, ' ', 2, false, 2, (void*)"8A4--" ) //48,41A,41
DEC (d3, XRLI8		, "xrl a,'i8'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( d4, CALL6  	, "call 'a11'"	, ' ', 2, false, 2, (void*)"8--2-" ) //48,22
DEC( d5, SELRB1		, "sel rb1"	, ' ', 1, false, 1, (void*)"8A4--" ) //48,41A,41
// d6 JNIBF 41A,41
DEC( d7, MOVFA		, "mov psw,a"	, ' ', 1, false, 1, (void*)"8A4--" ) //48,41A,41
DEC( d8, XRLR0		, "xrl a,r0"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( d9, XRLR1		, "xrl a,r1"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( da, XRLR2		, "xrl a,r2"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( db, XRLR3		, "xrl a,r3"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( dc, XRLR4		, "xrl a,r4"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( dd, XRLR5		, "xrl a,r5"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( de, XRLR6		, "xrl a,r6"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( df, XRLR7		, "xrl a,r7"	, ' ', 1, false, 1, (void*)"8A421" )

// e0 undefined
// e1 undefined
// e2 undefined
DEC( e3, MOVP3AIA	,  "movp3 a,@a"	, ' ', 1, false, 2, (void*)"8A4--" ) //48,41A,41
DEC( e4, JMP7   	, "jmp 'a11'"   , ' ', 2, false, 2, (void*)"8--2-" ) //48,22
DEC( e5, SELMB0		, "sel mb0"	, ' ', 1, false, 1, (void*)"8----" ) //48
// e5 EN DMA 41A
DEC( e6, JNC		, "jnc 'a8'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( e7, RL		, "rl a"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( e8, DJNZR0		, "djnz r0,'a8'", ' ', 2, false, 2, (void*)"8A421" )
DEC( e9, DJNZR1		, "djnz r1,'a8'", ' ', 2, false, 2, (void*)"8A421" )
DEC( ea, DJNZR2		, "djnz r2,'a8'", ' ', 2, false, 2, (void*)"8A421" )
DEC( eb, DJNZR3		, "djnz r3,'a8'", ' ', 2, false, 2, (void*)"8A421" )
DEC( ec, DJNZR4		, "djnz r4,'a8'", ' ', 2, false, 2, (void*)"8A421" )
DEC( ed, DJNZR5		, "djnz r5,'a8'", ' ', 2, false, 2, (void*)"8A421" )
DEC( ee, DJNZR6		, "djnz r6,'a8'", ' ', 2, false, 2, (void*)"8A421" )
DEC( ef, DJNZR7		, "djnz r7,'a8'", ' ', 2, false, 2, (void*)"8A421" )

DEC( f0, MOVAIR0	, "mov a,@r0"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( f1, MOVAIR1	, "mov a,@r1"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( f2, JB7		, "jb7 'a8'"	, ' ', 2, false, 2, (void*)"8A4--" ) //48,41A,41
// f3 undefined
DEC( f4, CALL7  	, "call 'a11'"	, ' ', 2, false, 2, (void*)"8--2-" ) //48,22
DEC( f5, SELMB1		, "sel mb1"	, ' ', 1, false, 1, (void*)"8----" ) //48
// f5 EN FLAGS 41A
DEC( f6, JC		, "jc 'a8'"	, ' ', 2, false, 2, (void*)"8A421" )
DEC( f7, RLC		, "rlc a"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( f8, MOVAR0		, "mov a,r0"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( f9, MOVAR1		, "mov a,r1"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( fa, MOVAR2		, "mov a,r2"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( fb, MOVAR3		, "mov a,r3"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( fc, MOVAR4		, "mov a,r4"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( fd, MOVAR5		, "mov a,r5"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( fe, MOVAR6		, "mov a,r6"	, ' ', 1, false, 1, (void*)"8A421" )
DEC( ff, MOVAR7		, "mov a,r7"	, ' ', 1, false, 1, (void*)"8A421" )
