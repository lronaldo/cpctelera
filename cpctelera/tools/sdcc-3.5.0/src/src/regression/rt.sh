# test Script

USAGE="Usage: `basename $0` BASENAME"

if [ $# -lt 1 ] ; then
  echo "$USAGE"
  exit 1
fi

# compile

../../bin/sdcc -I ../../device/include/pic -c -mpic14 -pp16f877 $1.c


# create the executable

gplink --map -c -o $1.o $1.o
./create_stc $1.cod $1.stc

./simulate $1.stc garbage.log
