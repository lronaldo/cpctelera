PRJ=$1

if [ -z "$PRJ" ]; then
   PRJ=t1
fi

../sz80 -C conf.cmd ${PRJ} <run.cmd
