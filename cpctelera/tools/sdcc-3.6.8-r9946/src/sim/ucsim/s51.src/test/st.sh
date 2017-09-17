TEST=""
S51v5=s515
S51v6=s516
VER=6
S51=$S51v6

PROF=""

while [ -n "$1" ]; do
    case "$1" in
	-6)
	    S51=$S51v6
	    VER=6
	    shift
	    ;;
	-5)
	    S51=$S51v5
	    VER=5
	    shift
	    ;;
	-v)
	    shift
	    S51=s51-${1}
	    VER="${1}"
	    shift
	    ;;
	-X)
	    S51=$S51vX
	    VER=X
	    shift
	    ;;
	-p)
	    PROF="p"
	    shift
	    ;;
	-P)
	    shift
	    TEST="$1"
	    CMD="gprof ${S51}p st${TEST}_v${VER}_gmon.out"
	    echo $CMD
	    $CMD
	    exit 0
	    ;;
	*)
	    TEST="$1"
	    shift
	    ;;
    esac
done

if [ -n "$PROF" ]; then
    S51=${S51}p
fi

echo -e "*\n* st${TEST}\n*\n"

make -f st${TEST}.mk clean all

SIM=st${TEST}${VER}.sim
OUT=st${TEST}${VER}.out
TIM=st${TEST}${VER}.tim
CSV=st${TEST}${VER}.csv
CMD=st${TEST}${VER}.cmd

rm -f $SIM $OUT $TIM $CSV

>$CMD
echo "set hardware simif xram 0xffff" >>$CMD
echo "set hardware simif fout \"st${TEST}${VER}.sout\"" >>$CMD
cat st.cmd >>$CMD

ls -l st${TEST}.ihx

/usr/bin/time -o $TIM -f '
Elapsed\t%e
Kernel\t%S
User\t%U
In\t%I
Out\t%O
SwT\t%c
SwIO\t%w
MaxFlt\t%F
MinFlt\t%R
Swaps\t%W
Mem\t%M
Unshr\t%D
' $S51 -tC52 -Sin=/dev/null,out=$OUT st${TEST}.ihx <$CMD|tee $SIM
tee -a $SIM <$TIM

if [ -n "$PROF" ]; then
    mv gmon.out st${TEST}_v${VER}_gmon.out
fi

E=$(grep Elapsed $SIM)
#echo
#echo $E

E=$(echo $E|cut -d ' ' -f 2)
C=$(grep 'Total time' $SIM|sed 's/^[^(]*(//'|sed 's/ .*//')
S=$(echo "scale=3;${C}/${E}/1000000"|bc)
ST=$(grep 'Simulated ' $SIM|cut -d' ' -f5)
echo "st${TEST} speed= $S Mclk/sec"|tee -a $SIM

echo "st${TEST},${VER},${E},${S}"|tee $CSV

echo $E >st${TEST}${VER}_e.txt
echo $ST >st${TEST}${VER}_st.txt
