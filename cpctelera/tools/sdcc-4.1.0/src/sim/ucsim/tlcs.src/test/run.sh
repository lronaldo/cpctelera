PRJ=$1

if [ -z "$PRJ" ]; then
   PRJ=t1
fi

../stlcs -C conf.cmd ${PRJ}.ihx <run.cmd
