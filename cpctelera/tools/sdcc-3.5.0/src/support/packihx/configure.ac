AC_INIT(packihx.c)
AC_CONFIG_HEADER(config.h:config_in.h)
AC_PREREQ(2.60)

AC_PROG_CC

AC_CHECK_SIZEOF(char, 1)
AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(long long, 8)

type_name()
{
    if expr "$ac_cv_sizeof_char" '>=' "$1" >/dev/null; then
	echo "char"
	exit
    fi
    if expr "$ac_cv_sizeof_short" '>=' "$1" >/dev/null; then
	echo "short"
	exit
    fi
    if expr "$ac_cv_sizeof_int" '>=' "$1" >/dev/null; then
	echo "int"
	exit
    fi
    if expr "$ac_cv_sizeof_long" '>=' "$1" >/dev/null; then
	echo "long"
	exit
    fi
    if expr "$ac_cv_sizeof_long_long" '>=' "$1" >/dev/null; then
	echo "long long"
	exit
    fi
    echo "long long"
}

# Try to append a c99 flag
AC_PROG_CC_C99
if test "$ac_cv_prog_cc_c99" != "no"; then
  CFLAGS="$ac_cv_prog_cc_c99 ${CFLAGS}"
fi

AC_MSG_CHECKING(type name for byte)
TYPE_BYTE=`type_name 1`
AC_MSG_RESULT($TYPE_BYTE)
AC_MSG_CHECKING(type name for word)
TYPE_WORD=`type_name 2`
AC_MSG_RESULT($TYPE_WORD)
AC_DEFINE_UNQUOTED(TYPE_BYTE, [$TYPE_BYTE], [XXX])
AC_DEFINE_UNQUOTED(TYPE_WORD, [$TYPE_WORD], [XXX])
AC_DEFINE_UNQUOTED(TYPE_UBYTE, [unsigned $TYPE_BYTE], [XXX])
AC_DEFINE_UNQUOTED(TYPE_UWORD, [unsigned $TYPE_WORD], [XXX])

AH_BOTTOM([typedef TYPE_UBYTE Uint8;
typedef TYPE_UWORD Uint16;])

AC_OUTPUT(Makefile)
