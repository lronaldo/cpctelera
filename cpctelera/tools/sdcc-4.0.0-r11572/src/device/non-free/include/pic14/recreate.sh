#!/bin/sh

# This script can be used to recreate the device library files from
# gputils' .inc files.
# Usage:
#     mkdir temp && cd temp && ../recreate.sh
#
# You will need to adjust the paths to SDCC and gputils before running!

GPUTILS=$HOME/svn/gputils
SDCC=$HOME/svn/plain

is_in()
{
  local f l j
  f=$1
  shift
  l=$*

  for j in $l; do
    if [ $f = $j ]
    then
      return 0
    fi
  done

  return 1
}

NO_LEGACY_NAMES=

for i in ../pic*.h
do
  if ! is_in $i ../pic14regs.h ../pic16fam.h ]
  then
    test -e $i && grep -q NO_LEGACY_NAMES $i && NO_LEGACY_NAMES="$NO_LEGACY_NAMES $i"
  fi
done

for i in ../pic*.h
do
  if ! is_in $i ../pic14regs.h ../pic16fam.h ]
  then
    if is_in $i $NO_LEGACY_NAMES
    then
      emit_legacy_names=1
    else
      emit_legacy_names=
    fi
    DEV=`echo "$i" | sed -e "s:../pic::;s/\.h//"`;
    echo "Creating ${DEV} ${emit_legacy_names}...";
    "${SDCC}/support/scripts/inc2h.pl" "${DEV}" "${GPUTILS}" "${emit_legacy_names}";
  fi
done
