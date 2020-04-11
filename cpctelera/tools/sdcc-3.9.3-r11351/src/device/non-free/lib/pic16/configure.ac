#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([pic16lib], [0.3], [tecodev AT users sourceforge net])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([libdev/pic18f13k50.c])
AC_CONFIG_HEADER([config.h])


# SDCC setup
case $srcdir in
    [\\/]*|?:[\\/]*) abs_srcdir="$srcdir";
        ;;
    *) abs_srcdir="$ac_pwd/$srcdir";
        ;;
esac
case $ac_top_build_prefix in
    [\\/]*|?:[\\/]*)
        abs_top_builddir="$ac_top_build_prefix";
        ;;
    *) abs_top_builddir="$ac_pwd/$ac_top_build_prefix";
        ;;
esac

libdir=$libdir/pic16

AC_SUBST([USE_FLOATS], ["-DUSE_FLOATS=0"])
AC_ARG_ENABLE(
    [floats],
    AS_HELP_STRING([--enable-floats], [Enable output of float-values via printf().]),
    [if test "yes" = $enableval; then USE_FLOATS="-DUSE_FLOATS=1"; fi]
)

# Checks for programs.

# The default architecture can be selected at configure time by setting the
# environment variable ARCH to the desired device (18fxxx).
AC_SUBST(ARCH, [${ARCH:-18f452}])

# We cannot use AC_PROG_CC(sdcc) as sdcc might not be built at configure-time...
AC_SUBST(CC, [\'$abs_top_builddir/../../../../bin/sdcc\'])
AC_PATH_PROG(CCAS, gpasm, :)
AC_PATH_PROG(LD, gplink, :)
AC_PATH_PROG(AR, gplib, :)

case ":$CCAS:$LD:$AR:" in
    *:::*)
        AC_MSG_ERROR([gputils (gpasm, gplink, and gplib) are required but not found.
Either install gputils or reconfigure with --disable-pic14-port and --disable-pic16-port.])
        ;;
esac;

mCCAS="$CCAS";
mLD="$LD";
AC_SUBST(CFLAGS, ["-mpic16 -p$ARCH"])
AC_SUBST(CCASFLAGS, ["-p$ARCH"])
AC_SUBST(ARFLAGS, [-c])
AC_SUBST(CCAS, [\'$CCAS\'])
AC_SUBST(LD, [\'$LD\'])
AC_SUBST(AR, [\'$AR\'])

# $RANLIB is called by the autotools but not provided nor required
AC_SUBST(RANLIB, [:])
AC_SUBST(OBJEXT, [o])

_AM_DEPENDENCIES(CC)
_AM_DEPENDENCIES(CCAS)

LIBDEV="${srcdir}/libdev"
m4_include([supported-devices.ac])
m4_include([processors.ac])

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 libdev/Makefile])
AC_OUTPUT
