PRJ=$1

if [ -z "$PRJ" ]; then
   PRJ=test_mdu88x
fi

if [ -f ${PRJ}.ihx ]; then
    if [ -f ${PRJ}_conf.cmd ]; then
	CONF="-C ${PRJ}_conf.cmd"
    elif [ -f conf.cmd ]; then
	CONF="-C conf.cmd"
    else
	CONF=""
    fi
    if [ -f ${PRJ}.type ]; then
	TYPE="-t $(cat ${PRJ}.type)"
    else
	TYPE="-t 52"
    fi
    if [ -f ${PRJ}.cmd ]; then
	CMD="../s51 ${TYPE} ${CONF} -S in=/dev/null,out=${PRJ}.out ${PRJ}.ihx"
	echo $CMD
	$CMD <${PRJ}.cmd | tee ${PRJ}.sim
    else
	CMD="../s51 ${TYPE} ${CONF} -Z6666 -S in=/dev/null,out=${PRJ}.out -G ${PRJ}.ihx"
	echo $CMD
	$CMD|tee ${PRJ}.sim
    fi
    cat ${PRJ}.out
fi
