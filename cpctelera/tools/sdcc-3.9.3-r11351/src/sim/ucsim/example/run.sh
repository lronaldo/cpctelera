PRJ=$1

if [ -z "$PRJ" ]; then
   PRJ=simif
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
    CMD="../s51.src/s51 ${TYPE} ${CONF} -Z6666 -S in=/dev/null,out=${PRJ}.out -G ${PRJ}.ihx"
    echo $CMD
    $CMD
    cat ${PRJ}.out
    echo
fi
