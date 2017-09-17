#
# configure.in - input script to autoconf to
#                configure directory
#
# This file is part of the GNU PIC Library.
#
# Originally written by
# 	Vangelis Rokas <vrokas@otenet.gr>
#
# Adopted for the SDCC/pic14 library by
#	Raphael Neider <rneider AT web.de>
#
# $Id: configure.ac 9283 2015-07-12 01:14:32Z benshi $
#

# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.60)
AC_INIT

AC_DEFUN([AC_CONFIG_SRCDIR])
#AC_DEFUN(AC_CONFIG_FILES)

# Checks for programs.
AC_CHECK_PROG(GPASM,	gpasm,	gpasm, :)
AC_CHECK_PROG(GPLINK,	gplink,	gplink, :)
AC_CHECK_PROG(GPLIB,	gplib,	gplib, :)

AC_CHECK_PROG(RM,	rm,	[rm -f], :)
AC_CHECK_PROG(CP,	cp,	cp, :)
AC_CHECK_PROG(MV,	mv,	mv, :)
AC_CHECK_PROG(LS,	ls,	ls, :)
AC_CHECK_PROG(SED,	sed,	sed, :)
AC_PROG_EGREP
AC_CHECK_PROG(MKDIR,	mkdir,	[mkdir -p], :)
AC_CHECK_PROG(RMDIR,	rmdir,	rmdir, :)

case ":$GPASM:$GPLINK:$GPLIB" in
    *:::*)
        AC_MSG_ERROR([gputils (gpasm, gplink, and gplib) are required but not found.
Either install gputils or reconfigure with --disable-pic14-port and --disable-pic16-port.])
        ;;
esac;

AC_PROG_MAKE_SET

mCCAS=$GPASM;
mLD=$GPLINK;
AC_MSG_CHECKING([devices supported by gputils])
GOOD_PICS="";
BAD_PICS="";
CHECK=".checkdevices/check";
RESULT="pics.supported";
N_GOOD=0
N_BAD=0
mkdir -p ".checkdevices";
rm -f "$RESULT";
for i in "${srcdir}/libdev/pic1"*.c; do
  p="${i##*pic}";
  p="${p%.c}";
  P=$(echo "$p" | tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ);
  printf "  include \"p%s.inc\"\n  END" "$p" > "${CHECK}.asm";
  if "$mCCAS" -p "$p" -o "${CHECK}.o" -c "${CHECK}.asm" >/dev/null 2>&1 && "$mLD" "${CHECK}.o" >/dev/null 2>&1; then
    GOOD_PICS="$GOOD_PICS $p";
    N_GOOD=`expr $N_GOOD + 1`;
    echo "$P" >> "$RESULT";
  else
    BAD_PICS="$BAD_PICS $p";
    N_BAD=`expr $N_BAD + 1`;
  fi;
done;
AC_MSG_RESULT([[$N_GOOD devices ($GOOD_PICS)]])
AC_SUBST([GOOD_PICS])

if test -n "$BAD_PICS"; then
  AC_MSG_WARN([[
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!!
!!! WARNING: The installed gputils do not support all
!!! PIC devices currently supported by SDCC.
!!! If you continue to build SDCC, library files for the
!!! following devices will not be built, and you will
!!! not be able to compile any projects for these devices:
!!!

$BAD_PICS
==> $N_BAD devices are *not* supported
    $N_GOOD devices are supported

!!!
!!! Please update your gputils to a recent snapshot and
!!! run configure again using the updated gputils. Make
!!! sure to have them in PATH prior to the previously
!!! found ones (or remove the older version completely).
!!!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

]])
  for d in 5 4 3 2 1; do
    echo "Continuing in $d seconds ...";
    sleep 1;
  done;
fi;

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 Makefile.common
                 libdev/Makefile])
AC_OUTPUT
