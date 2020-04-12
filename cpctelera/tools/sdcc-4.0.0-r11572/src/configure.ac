#!/bin/sh

AC_PREREQ(2.60)
AC_INIT(.version)
AC_CONFIG_SRCDIR([Makefile.in])

# Determine the host and build type.
# ===========================================================================
AC_CANONICAL_HOST

AC_CONFIG_HEADER([sdccconf.h:sdccconf_in.h sdas/linksrc/asxxxx_config.h])

AC_PROG_AWK

AC_MSG_CHECKING(version of the package)
if test -f ${srcdir}/.version; then
  { read VERSION; } < ${srcdir}/.version
  AC_MSG_RESULT($VERSION)
elif test -f ../.version; then
  { read VERSION; } < ../.version
  AC_MSG_RESULT($VERSION)
else
  VERSION="0.0.0"
  AC_MSG_RESULT(unknown using 0.0.0)
fi
VERSIONHI=`echo $VERSION|$AWK 'BEGIN {FS="."} {print $1}'`
VERSIONLO=`echo $VERSION|$AWK 'BEGIN {FS="."} {print $2}'`
VERSIONP=`echo $VERSION|$AWK 'BEGIN {FS="."} {print $3}'`

AC_SUBST(PACKAGE, [sdcc])
AC_SUBST(VERSION)
AC_SUBST(VERSIONHI)
AC_SUBST(VERSIONLO)
AC_SUBST(VERSIONP)
AC_DEFINE_UNQUOTED(SDCC_VERSION_LO, ${VERSIONLO}, [XXX])
AC_DEFINE_UNQUOTED(SDCC_VERSION_HI, ${VERSIONHI}, [XXX])
AC_DEFINE_UNQUOTED(SDCC_VERSION_P, ${VERSIONP}, [XXX])
AC_DEFINE_UNQUOTED(SDCC_VERSION_STR, "${VERSION}", [XXX])
AC_ARG_PROGRAM
sdcc_cv_version=$VERSION
sdcc_cv_versionhi=$VERSIONHI
sdcc_cv_versionlo=$VERSIONLO
sdcc_cv_versionp=$VERSIONP


# Required programs
# ===========================================================================
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_CHECK_PROG(AUTOCONF, autoconf, autoconf, :)
AC_CHECK_PROG(STRIP, strip, strip, :)
AC_CHECK_PROG(AS, as, as, :)
AC_CHECK_PROG(CP, cp, cp, :)
AC_CHECK_PROG(AR, ar, ar, :)
AC_CHECK_PROG(M4, gm4, gm4, m4)

dnl Don't use AC_PROG_LEX
dnl LEXLIB is not useful in gcc.
AC_CHECK_PROGS(LEX, flex lex, :)

dnl Don't use AC_PROG_YACC
AC_CHECK_PROGS(YACC, 'bison -y' byacc yacc, :)

AC_CHECK_PROGS(PYTHON, python3.6 python2.7 python2 python3, python)

AC_DEFUN([SDCC_REQUIRE_PROG],
[if test "$1" = ":"; then
  AC_MSG_ERROR([Cannot find required program $2.])
 fi
])

SDCC_REQUIRE_PROG($YACC, bison)
SDCC_REQUIRE_PROG($LEX, flex)

AC_LANG_C

AC_CHECK_HEADERS(endian.h sys/endian.h machine/endian.h sys/isa_defs.h)
AC_CHECK_HEADERS(uchar.h)
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS(treedec/combinations.hpp,,AC_MSG_NOTICE([[treedec library missing, falling back to Thorup.]]))
AC_CHECK_HEADERS(gala/graph.h)
AC_CHECK_HEADERS(boost/graph/adjacency_list.hpp,,AC_MSG_ERROR([[boost library not found (boost/graph/adjacency_list.hpp).]]))
AC_LANG_POP([C++])

AC_ARG_WITH([ccache],
    AC_HELP_STRING([--without-ccache], [do not use ccache even if available]),
    [], [])
case x${with_ccache-yes} in
    xyes) AC_CHECK_PROG([CCACHE], [ccache], [ccache], []) ;;
    xno) AC_SUBST([CCACHE], []) ;;
    *) AC_SUBST([CCACHE], [$with_ccache]) ;;
esac


# Checking for functions
# ===========================================================================
AC_CHECK_FUNCS(mkstemp strndup setrlimit backtrace_symbols_fd)


# Macro definitions
# ===========================================================================

# _sdcc_EXECUTE_IFELSE_BODY(COMMAND)
# -----------------------
# Shell function body for _AC_EXECUTE_IFELSE.
m4_define([_sdcc_EXECUTE_IFELSE_BODY],
[  AS_LINENO_PUSH([$[]1])
  AS_IF([_AC_DO_STDERR([$1 $2]) > conftest.out && {
         test ! -s conftest.err
       }],
      [ac_retval=0],
      [ac_retval=1])
  AS_LINENO_POP
])# _sdcc_EXECUTE_IFELSE_BODY

# sdcc_EXECUTE_IFELSE(COMMAND[ACTION-IF-TRUE], [ACTION-IF-FALSE])
# ----------------------------------------------------------------
# Try to execute COMMAND.
AC_DEFUN([sdcc_EXECUTE_IFELSE],
[_$0_BODY($1)
  AS_IF([test "$ac_retval" = 0],
    [$2],
    [$3])
rm conftest.err conftest.out
])# sdcc_EXECUTE_IFELSE

# Check whether the compiler for the current language is SunPRO.

AC_DEFUN([sdcc_IS_SUNPRO],
[AC_CACHE_CHECK([whether we are using the SunPRO _AC_LANG compiler],
  [sdcc_cv_[]_AC_LANG_ABBREV[]_compiler_sunpro],
  [_AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [[#if !defined __SUNPRO_C && !defined __SUNPRO_CC
       choke me
#endif
]])],
    [sdcc_cv_[]_AC_LANG_ABBREV[]_compiler_sunpro=yes],
    [sdcc_cv_[]_AC_LANG_ABBREV[]_compiler_sunpro=no])
])])

# sdcc_IS_VALID_OPTION macro checks if the current compiler, selected by 
# AC_LANG, supports option specified as the 1st parameter. The test fails if
# the compiler returns an error or in case something is writeen to stderr.
# For example: sdcc_VALID_OPT(-fPIC)

AC_DEFUN([sdcc_IS_VALID_OPTION],
[arg1=`echo $1 | sed -e"s/^-*//" -e"s/-/_/"`
ac_test_[]_AC_LANG_PREFIX[]FLAGS=${[]_AC_LANG_PREFIX[]FLAGS+set}
sdcc_save_[]_AC_LANG_PREFIX[]FLAGS=$[]_AC_LANG_PREFIX[]FLAGS
AC_CACHE_CHECK(whether _AC_LANG accepts $1, sdcc_cv_[]_AC_LANG_ABBREV[]_$arg1,
  [sdcc_save_[]_AC_LANG_ABBREV[]_werror_flag=$ac_[]_AC_LANG_ABBREV[]_werror_flag
   ac_[]_AC_LANG_ABBREV[]_werror_flag=yes
   
   []_AC_LANG_PREFIX[]FLAGS="$1"
   AC_COMPILE_IFELSE([AC_LANG_PROGRAM()],
     [eval sdcc_cv_[]_AC_LANG_ABBREV[]_$arg1=yes],
     [eval sdcc_cv_[]_AC_LANG_ABBREV[]_$arg1=no])
   ac_[]_AC_LANG_ABBREV[]_werror_flag=$sdcc_save_[]_AC_LANG_ABBREV[]_werror_flag])
   []_AC_LANG_PREFIX[]FLAGS=$sdcc_save_[]_AC_LANG_PREFIX[]FLAGS
])

# This macro expands DIR and assigns it to RET.
# If DIR is NONE, then it's replaced by DEFAULT.
#
# Based on AC_DEFINE_DIR
#
# Examples:
#
#  adl_EXPAND(prefix, "/usr/local", expanded_prefix)

AC_DEFUN([adl_EXPAND], [
  test "x$prefix" = xNONE && prefix="$ac_default_prefix"
  test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'
  ac_expand=[$]$1
  test "x$ac_expand" = xNONE && ac_expand="[$]$2"
  ac_expand=`eval echo [$]ac_expand`
  $3=`eval echo [$]ac_expand`
])

# adl_NORMALIZE_PATH
#
#  - empty paths are changed to '.'
#  - trailing slashes are removed
#  - repeated slashes are squeezed except a leading doubled slash '//'
#    (which might indicate a networked disk on some OS).
#
#
# REFERENCE_STRING is used to turn '/' into '\' and vice-versa: if
# REFERENCE_STRING contains some backslashes, all slashes and backslashes
# are turned into backslashes, otherwise they are all turned into slashes.
#
# This makes processing of DOS filenames quite easier, because you can turn a
# filename to the Unix notation, make your processing, and turn it back to
# original notation.

dnl Available from the GNU Autoconf Macro Archive at:
dnl http://www.gnu.org/software/ac-archive/htmldoc/normpath.html
dnl
AC_DEFUN([adl_NORMALIZE_PATH],
[case ":[$]$1:" in
dnl change empty paths to '.'
  ::) $1='.' ;;
dnl strip trailing slashes
  :*[[\\/]]:) $1=`echo "[$]$1" | sed 's,[[\\/]]*[$],,'` ;;
  :*:) ;;
esac
dnl squeze repeated slashes
case ifelse($2,,"[$]$1",$2) in
dnl if the path contains any backslashes, turn slashes into backslashes

dnl Bernhard Held 2003-04-06
dnl This was the original line. It does not:
dnl - convert the first slash
dnl - replace a slash with a double-backslash
dnl *\\*) $1=`echo "[$]$1" | sed 's,\(.\)[[\\/]][[\\/]]*,\1\\\\,g'` ;;
    *\\*) $1=`echo "[$]$1" | sed 's,\(.\)[[\\/]][[\\/]]*,\1\\\\\\\\,g
                                  s,^[[\\/]],\\\\\\\\,'` ;;

dnl if the path contains slashes, also turn backslashes into slashes
 *) $1=`echo "[$]$1" | sed 's,\(.\)[[\\/]][[\\/]]*,\1/,g'` ;;
esac])


# adl_COMPUTE_RELATIVE_PATH
#
# PATH_LIST is a space-separated list of colon-separated triplets of the form
# 'FROM:TO:RESULT'. This function iterates over these triplets and set $RESULT
# to the relative path from $FROM to $TO. Note that $FROM and $TO needs to be
# absolute filenames for this macro to success.

AC_DEFUN([adl_COMPUTE_RELATIVE_PATHS],
[for _lcl_i in $1; do
  _lcl_from=\[$]`echo "[$]_lcl_i" | sed 's,:.*$,,'`
  _lcl_to=\[$]`echo "[$]_lcl_i" | sed 's,^[[^:]]*:,,' | sed 's,:[[^:]]*$,,'`
  _lcl_result_var=`echo "[$]_lcl_i" | sed 's,^.*:,,'`
  adl_RECURSIVE_EVAL([[$]_lcl_from], [_lcl_from])
  adl_RECURSIVE_EVAL([[$]_lcl_to], [_lcl_to])
  _lcl_notation="$_lcl_from$_lcl_to"
  adl_NORMALIZE_PATH([_lcl_from],['/'])
  adl_NORMALIZE_PATH([_lcl_to],['/'])
  adl_COMPUTE_RELATIVE_PATH([_lcl_from], [_lcl_to], [_lcl_result_tmp])
  adl_NORMALIZE_PATH([_lcl_result_tmp],["[$]_lcl_notation"])
  eval $_lcl_result_var='[$]_lcl_result_tmp'
done])

## Note:
## *****
## The following helper macros are too fragile to be used out
## of adl_COMPUTE_RELATIVE_PATHS (mainly because they assume that
## paths are normalized), that's why I'm keeping them in the same file.
## Still, some of them maybe worth to reuse.

dnl adl_COMPUTE_RELATIVE_PATH(FROM, TO, RESULT)
dnl ===========================================
dnl Compute the relative path to go from $FROM to $TO and set the value
dnl of $RESULT to that value.  This function work on raw filenames
dnl (for instead it will considerate /usr//local and /usr/local as
dnl two distinct paths), you should really use adl_COMPUTE_REALTIVE_PATHS
dnl instead to have the paths sanitized automatically.
dnl
dnl For instance:
dnl    first_dir=/somewhere/on/my/disk/bin
dnl    second_dir=/somewhere/on/another/disk/share
dnl    adl_COMPUTE_RELATIVE_PATH(first_dir, second_dir, first_to_second)
dnl will set $first_to_second to '../../../another/disk/share'.
AC_DEFUN([adl_COMPUTE_RELATIVE_PATH],
[adl_COMPUTE_COMMON_PATH([$1], [$2], [_lcl_common_prefix])
adl_COMPUTE_BACK_PATH([$1], [_lcl_common_prefix], [_lcl_first_rel])
adl_COMPUTE_SUFFIX_PATH([$2], [_lcl_common_prefix], [_lcl_second_suffix])
$3="[$]_lcl_first_rel[$]_lcl_second_suffix"])

dnl adl_COMPUTE_COMMON_PATH(LEFT, RIGHT, RESULT)
dnl ============================================
dnl Compute the common path to $LEFT and $RIGHT and set the result to $RESULT.
dnl
dnl For instance:
dnl    first_path=/somewhere/on/my/disk/bin
dnl    second_path=/somewhere/on/another/disk/share
dnl    adl_COMPUTE_COMMON_PATH(first_path, second_path, common_path)
dnl will set $common_path to '/somewhere/on'.
AC_DEFUN([adl_COMPUTE_COMMON_PATH],
[$3=''
_lcl_second_prefix_match=''
while test "[$]_lcl_second_prefix_match" != 0; do
  _lcl_first_prefix=`expr "x[$]$1" : "x\([$]$3/*[[^/]]*\)"`
  _lcl_second_prefix_match=`expr "x[$]$2" : "x[$]_lcl_first_prefix"`
  if test "[$]_lcl_second_prefix_match" != 0; then
    if test "[$]_lcl_first_prefix" != "[$]$3"; then
      $3="[$]_lcl_first_prefix"
    else
      _lcl_second_prefix_match=0
    fi
  fi
done])

dnl adl_COMPUTE_SUFFIX_PATH(PATH, SUBPATH, RESULT)
dnl ==============================================
dnl Substrack $SUBPATH from $PATH, and set the resulting suffix
dnl (or the empty string if $SUBPATH is not a subpath of $PATH)
dnl to $RESULT.
dnl
dnl For instace:
dnl    first_path=/somewhere/on/my/disk/bin
dnl    second_path=/somewhere/on
dnl    adl_COMPUTE_SUFFIX_PATH(first_path, second_path, common_path)
dnl will set $common_path to '/my/disk/bin'.
AC_DEFUN([adl_COMPUTE_SUFFIX_PATH],
[$3=`expr "x[$]$1" : "x[$]$2/*\(.*\)"`])

dnl adl_COMPUTE_BACK_PATH(PATH, SUBPATH, RESULT)
dnl ============================================
dnl Compute the relative path to go from $PATH to $SUBPATH, knowing that
dnl $SUBPATH is a subpath of $PATH (any other words, only repeated '../'
dnl should be needed to move from $PATH to $SUBPATH) and set the value
dnl of $RESULT to that value.  If $SUBPATH is not a subpath of PATH,
dnl set $RESULT to the empty string.
dnl
dnl For instance:
dnl    first_path=/somewhere/on/my/disk/bin
dnl    second_path=/somewhere/on
dnl    adl_COMPUTE_BACK_PATH(first_path, second_path, back_path)
dnl will set $back_path to '../../../'.
AC_DEFUN([adl_COMPUTE_BACK_PATH],
[adl_COMPUTE_SUFFIX_PATH([$1], [$2], [_lcl_first_suffix])
$3=''
_lcl_tmp='xxx'
while test "[$]_lcl_tmp" != ''; do
  _lcl_tmp=`expr "x[$]_lcl_first_suffix" : "x[[^/]]*/*\(.*\)"`
  if test "[$]_lcl_first_suffix" != ''; then
     _lcl_first_suffix="[$]_lcl_tmp"
     $3="../[$]$3"
  fi
done])


dnl adl_RECURSIVE_EVAL(VALUE, RESULT)
dnl =================================
dnl Interpolate the VALUE in loop until it does not change,
dnl and set the result to $RESULT.
dnl WARNING: It is easy to get an infinite loop with some unsane input.
AC_DEFUN([adl_RECURSIVE_EVAL],
[_lcl_receval="$1"
$2=`(test "x$prefix" = xNONE && prefix="$ac_default_prefix"
     test "x$exec_prefix" = xNONE && exec_prefix="${prefix}"
     _lcl_receval_old=''
     while test "[$]_lcl_receval_old" != "[$]_lcl_receval"; do
       _lcl_receval_old="[$]_lcl_receval"
       eval _lcl_receval="\"[$]_lcl_receval\""
     done
     echo "[$]_lcl_receval")`])

dnl adl_NORMALIZE_DEFINE_UNQUOTED(var, DEFINE, REFERENCE_STRING)
AC_DEFUN([adl_NORMALIZE_DEFINE_UNQUOTED], [
  ac_ndu=[$]$1
  adl_NORMALIZE_PATH([ac_ndu], [$]$3)
  AC_DEFINE_UNQUOTED($2, "${ac_ndu}", [XXX])
])

dnl adl_NORMALIZE_PATH_MSG(input_string, var, dir_separator)
dnl ========================================================
dnl call adl_NORMALIZE_PATH and format it for the result message
AC_DEFUN([adl_NORMALIZE_PATH_MSG], [
dnl replace /./ by /
$2=`echo "$1" | sed 's,/\./,/,g'`
adl_NORMALIZE_PATH([$2], [$3])
dnl replace \\ by \
$2=`echo "[$]$2" | sed 's,\\\\\\\\,\\\\,g'`
])

AX_CXX_COMPILE_STDCXX_11

# Checking characteristics of compilers and other programs
# ===========================================================================
if test "$ac_cv_c_compiler_gnu" = "yes"; then
  AC_CACHE_CHECK(whether preprocessor accepts -MM or -M,sdcc_cv_MM,
  echo "#include <stdio.h>" >_test_.c
  echo "" >>_test_.c
  $CPP -v -MM _test_.c 1>&5 2>&5
  if test "$?" = "0"; then
    sdcc_cv_MM="-MM"
  else
    sdcc_cv_MM="-M"
  fi
  rm -f _test_.*)

  # This is the first time when CFLAGS are set/modified!!
  sdcc_IS_VALID_OPTION(-ggdb)
  if test "$sdcc_cv_c_ggdb" = "yes"; then
    CFLAGS="-ggdb ${CFLAGS}"
    CXXFLAGS="-ggdb ${CXXFLAGS}"
  fi

  sdcc_IS_VALID_OPTION(-pipe)
  if test "$sdcc_cv_c_pipe" = "yes"; then
    CFLAGS="-pipe ${CFLAGS}"
    # Don't use "-pipe" on x86 Solaris g++, since src/z80/ralloc2.cc dies with:
    # Assembler: ralloc2.cc
    #         "<stdin>", line 117637 : Internal: Out of symbol table
    if test `uname` != "SunOS" -o \( `uname` = "SunOS" -a `uname -m` != "i86pc" \); then
      CXXFLAGS="-pipe ${CXXFLAGS}"
    fi
  fi

  WALL_FLAG="-Wall -Wno-parentheses"
  C99_FLAG="--std=c99"
  # We use CXX instead of CC here, since some of the C++ source files require a C++ compiler even during dependency generation.
  MAKEDEP="$CXX $sdcc_cv_MM"
else
  sdcc_IS_SUNPRO
  if test "$sdcc_cv_c_compiler_sunpro" = "yes"; then
    C99_FLAG="-xc99"
    CFLAGS="-xO2 ${C99_FLAG} ${CFLAGS}"
    CXXFLAGS="-xO2 ${CXXFLAGS}"
    WALL_FLAG=
    MAKEDEP="$CC ${C99_FLAG} -xM1"
  fi
fi

AC_SUBST(MAKEDEP)
AC_SUBST(WALL_FLAG)
AC_SUBST(C99_FLAG)

# Try to append a c99 flag
# -------------------------------------------------------------------------
AC_PROG_CC_C99
if test "$ac_cv_prog_cc_c99" != "no"; then
  CFLAGS="$ac_cv_prog_cc_c99 ${CFLAGS}"
fi

# Checks for typedefs, structures, and compiler characteristics.
# ===========================================================================
AC_TYPE_SIGNAL

AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(short)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)
AC_C_CHAR_UNSIGNED

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
  echo "long"
}

AC_MSG_CHECKING(type name for byte)
TYPE_CHAR=`type_name 1`
if test "$ac_cv_c_char_unsigned" = "yes"; then
  TYPE_BYTE="signed $TYPE_CHAR"
else
  TYPE_BYTE=$TYPE_CHAR
fi
AC_MSG_RESULT($TYPE_BYTE)
AC_MSG_CHECKING(type name for word)
TYPE_WORD=`type_name 2`
AC_MSG_RESULT($TYPE_WORD)
AC_MSG_CHECKING(type name for dword)
TYPE_DWORD=`type_name 4`
AC_MSG_RESULT($TYPE_DWORD)
AC_MSG_CHECKING(type name for qword)
TYPE_QWORD=`type_name 8`
AC_MSG_RESULT($TYPE_QWORD)

AC_DEFINE_UNQUOTED(TYPE_BYTE,  $TYPE_BYTE,  [XXX])
AC_DEFINE_UNQUOTED(TYPE_WORD,  $TYPE_WORD,  [XXX])
AC_DEFINE_UNQUOTED(TYPE_DWORD, $TYPE_DWORD, [XXX])
AC_DEFINE_UNQUOTED(TYPE_QWORD, $TYPE_QWORD, [XXX])

AC_DEFINE_UNQUOTED(TYPE_UBYTE,  unsigned $TYPE_CHAR,  [XXX])
AC_DEFINE_UNQUOTED(TYPE_UWORD,  unsigned $TYPE_WORD,  [XXX])
AC_DEFINE_UNQUOTED(TYPE_UDWORD, unsigned $TYPE_DWORD, [XXX])
AC_DEFINE_UNQUOTED(TYPE_UQWORD, unsigned $TYPE_QWORD, [XXX])

case "${host}" in
  *x86_64*cygwin*)
    LDFLAGS="$LDFLAGS -Wl,--stack,4194304"
esac

# Checking whether byte ordering is bigendian
# ===========================================================================
AC_C_BIGENDIAN

# Set standard installation paths
# ===========================================================================

# In the Makefiles we need paths with '/' as directory separator, even if
# crosscompiling for Win32.
# And we want to preserve the macros (e.g. ${prefix}) in the Makefiles.
# The variables in the Makefiles are replaced by AC_SUBST()
#
# In sdccconf.h the '/' in paths can be replaced by "\\" (normalized), if
#
# The macros are expanded for the header.
# The variables in the header are replaced by AC_*DEFINE*()
# sdccconf_h_dir_separator contains a backslash.
AC_ARG_VAR([sdccconf_h_dir_separator], [needed in sdccconf.h: either "/" (default) or "\\"])
if test "x${sdccconf_h_dir_separator}" = "x"; then
    sdccconf_h_dir_separator="/"
fi

# Makefiles
###########

# LIB_TYPE:
# *nix default: "RANLIB"

AC_ARG_VAR([LIB_TYPE], [library type: LIB, SDCCLIB, AR or RANLIB (default)])
if test "${LIB_TYPE}" = ""; then
    LIB_TYPE="RANLIB"
fi

# include_dir_suffix:
# *nix default: "sdcc/include"

AC_ARG_VAR([inclib_dir_suffix], [appended to datadir to define SDCC's include/lib directory])
if test "${inclib_dir_suffix}" = ""; then
    inclib_dir_suffix="sdcc"
fi

AC_ARG_VAR([include_dir_suffix], [appended to datadir to define SDCC's include directory])
if test "${include_dir_suffix}" = ""; then
    include_dir_suffix="${inclib_dir_suffix}/include"
fi

AC_ARG_VAR([non_free_inclib_dir_suffix], [appended to datadir to define SDCC's non-free include/lib directory])
if test "${non_free_inclib_dir_suffix}" = ""; then
    non_free_inclib_dir_suffix="sdcc/non-free"
fi

# non_free_include_dir_suffix:
# *nix default: "sdcc/non-free/include"

AC_ARG_VAR([non_free_include_dir_suffix], [appended to datadir to define SDCC's non-free include directory])
if test "${non_free_include_dir_suffix}" = ""; then
    non_free_include_dir_suffix="${non_free_inclib_dir_suffix}/include"
fi

# lib_dir_suffix:
# *nix default: "sdcc/lib"
AC_ARG_VAR([lib_dir_suffix], [appended to datadir to define SDCC's library root directory])
if test "${lib_dir_suffix}" = ""; then
    lib_dir_suffix="${inclib_dir_suffix}/lib"
fi

# non_free_lib_dir_suffix:
# *nix default: "sdcc/non-free/lib"
AC_ARG_VAR([non_free_lib_dir_suffix], [appended to datadir to define SDCC's non-free library root directory])
if test "${non_free_lib_dir_suffix}" = ""; then
    non_free_lib_dir_suffix="${non_free_inclib_dir_suffix}/lib"
fi

# docdir:
# *nix default: "${datadir}/sdcc/doc"
AC_ARG_VAR([docdir], [documentation installation directory])
if test "${docdir}" = ""; then
    docdir="\${datadir}"/sdcc/doc
fi

AC_SUBST(EXEEXT)

# sdccconf.h
############

AC_DEFINE_UNQUOTED(DIR_SEPARATOR_STRING, "${sdccconf_h_dir_separator}", [XXX])
AC_DEFINE_UNQUOTED(DIR_SEPARATOR_CHAR, '${sdccconf_h_dir_separator}', [XXX])

# prefix:
# default: "NONE", ${ac_default_prefix}: "/usr/local"
adl_EXPAND(prefix, ac_default_prefix, expanded_prefix)
adl_NORMALIZE_DEFINE_UNQUOTED(expanded_prefix, PREFIX, sdccconf_h_dir_separator)

# exec_prefix:
# default: "${prefix}"
adl_EXPAND(exec_prefix, expanded_prefix, expanded_exec_prefix)
adl_NORMALIZE_DEFINE_UNQUOTED(expanded_exec_prefix, EXEC_PREFIX, sdccconf_h_dir_separator)

# bindir:
# default: "${exec_prefix}/bin"
adl_EXPAND(bindir, "NONE", expanded_bindir)
adl_NORMALIZE_DEFINE_UNQUOTED(expanded_bindir, BINDIR, sdccconf_h_dir_separator)

# datadir:
# default: "${prefix}/share"
adl_EXPAND(datadir, "NONE", expanded_datadir)
adl_NORMALIZE_DEFINE_UNQUOTED(expanded_datadir, DATADIR, sdccconf_h_dir_separator)

# include/lib suffix
norm_inc_dir_suffix=${include_dir_suffix}
adl_NORMALIZE_PATH([norm_inc_dir_suffix], [$sdccconf_h_dir_separator])
AC_DEFINE_UNQUOTED(INCLUDE_DIR_SUFFIX,
                   DIR_SEPARATOR_STRING "${norm_inc_dir_suffix}", [XXX])
norm_non_free_inc_dir_suffix=${non_free_include_dir_suffix}
adl_NORMALIZE_PATH([norm_non_free_inc_dir_suffix], [$sdccconf_h_dir_separator])
AC_DEFINE_UNQUOTED(NON_FREE_INCLUDE_DIR_SUFFIX,
                   DIR_SEPARATOR_STRING "${norm_non_free_inc_dir_suffix}", [XXX])

norm_lib_dir_suffix=${lib_dir_suffix}
adl_NORMALIZE_PATH([norm_lib_dir_suffix], [$sdccconf_h_dir_separator])
AC_DEFINE_UNQUOTED(LIB_DIR_SUFFIX,
                   DIR_SEPARATOR_STRING "${norm_lib_dir_suffix}", [XXX])
norm_non_free_lib_dir_suffix=${non_free_lib_dir_suffix}
adl_NORMALIZE_PATH([norm_non_free_lib_dir_suffix], [$sdccconf_h_dir_separator])
AC_DEFINE_UNQUOTED(NON_FREE_LIB_DIR_SUFFIX,
                   DIR_SEPARATOR_STRING "${norm_non_free_lib_dir_suffix}", [XXX])

# relative paths
adl_COMPUTE_RELATIVE_PATHS([expanded_bindir:expanded_datadir:bin2data_dir])
adl_NORMALIZE_PATH(bin2data_dir, [$sdccconf_h_dir_separator])
AC_DEFINE_UNQUOTED(BIN2DATA_DIR,
                   DIR_SEPARATOR_STRING "${bin2data_dir}", [XXX])

adl_COMPUTE_RELATIVE_PATHS([expanded_prefix:expanded_bindir:prefix2bin_dir])
adl_NORMALIZE_PATH(prefix2bin_dir, [$sdccconf_h_dir_separator])
AC_DEFINE_UNQUOTED(PREFIX2BIN_DIR,
                   DIR_SEPARATOR_STRING "${prefix2bin_dir}", [XXX])

adl_COMPUTE_RELATIVE_PATHS([expanded_prefix:expanded_datadir:prefix2data_dir])
adl_NORMALIZE_PATH(prefix2data_dir, [$sdccconf_h_dir_separator])
if test "${prefix2data_dir}" = "."; then
    # small optimization for Mingw32; otherwise Borut will complain ;-)
    AC_DEFINE_UNQUOTED(PREFIX2DATA_DIR, "", [XXX])
else
    AC_DEFINE_UNQUOTED(PREFIX2DATA_DIR,
                       DIR_SEPARATOR_STRING "${prefix2data_dir}", [XXX])
fi

# standard libs
AC_DEFINE_UNQUOTED(STD_LIB,       "libsdcc", [XXX])
AC_DEFINE_UNQUOTED(STD_INT_LIB,   "libint", [XXX])
AC_DEFINE_UNQUOTED(STD_LONG_LIB,  "liblong", [XXX])
AC_DEFINE_UNQUOTED(STD_FP_LIB,    "libfloat", [XXX])
AC_DEFINE_UNQUOTED(STD_DS390_LIB, "libds390", [XXX])
AC_DEFINE_UNQUOTED(STD_DS400_LIB, "libds400", [XXX])

# SDCC runtime environment variables
sdcc_dir_name="SDCC_HOME"
AC_DEFINE_UNQUOTED(SDCC_DIR_NAME, "${sdcc_dir_name}", [XXX])

sdcc_include_name="SDCC_INCLUDE"
AC_DEFINE_UNQUOTED(SDCC_INCLUDE_NAME, "${sdcc_include_name}", [XXX])

sdcc_lib_name="SDCC_LIB"
AC_DEFINE_UNQUOTED(SDCC_LIB_NAME, "${sdcc_lib_name}", [XXX])

# add include guard and custom stuff to sdccconf.h
AH_TOP([#ifndef SDCCCONF_HEADER
#define SDCCCONF_HEADER])
AH_BOTTOM([#endif /* SDCCCONF_HEADER */])

# Port selection helper
# ===========================================================================
# macro AC_DO_ENABLER()
#   $1 used to access enable_$1, e.g. enable_doc
#   $2 OPT_DISABLE_$2, normally uppercase of $1, e.g. DOC
#   $3 help string
AC_DEFUN([AC_DO_ENABLER], [
  AC_ARG_ENABLE($1, AC_HELP_STRING([--enable-$1], $3))

  if test "[$]enable_$1" = "yes"; then
    OPT_ENABLE_$2=1
  else
    OPT_ENABLE_$2=0
  fi

  AC_DEFINE_UNQUOTED(OPT_ENABLE_$2, [$]OPT_ENABLE_$2, [XXX])
  AC_SUBST(OPT_ENABLE_$2)
])

# macro AC_DO_DISABLER()
#   $1 used to access disable_$1, e.g. ucsim
#   $2 OPT_DISABLE_$2, normally uppercase of $1, e.g. UCSIM
#   $3 help string
AC_DEFUN([AC_DO_DISABLER], [
  AC_ARG_ENABLE($1, AC_HELP_STRING([--disable-$1], $3))

  dnl the '-' in 'device-lib' needs special handling,
  dnl because the variable is 'enable_device_lib'
  arg1=`echo $1 | sed s/-/_/`

  if test "`eval echo \\$enable_$arg1`" = "no"; then
    OPT_DISABLE_$2=1
  else
    OPT_DISABLE_$2=0
  fi

  AC_DEFINE_UNQUOTED(OPT_DISABLE_$2, [$]OPT_DISABLE_$2, [XXX])
  AC_SUBST(OPT_DISABLE_$2)
])

# macro AC_DO_PORT($1, $2, $3, $4)
#   $1 used to access enable_$2_port, e.g. gbz80
#   $2 port name in ports.all and ports.build, e.g. z80
#   $3 OPT_DISABLE_$3, normally uppercase of $2, e.g. GBZ80
#   $4 help string
AC_DEFUN([AC_DO_PORT], [
  AC_ARG_ENABLE($1-port,
                AC_HELP_STRING([--disable-$1-port], $4))

  if test "[$]enable_$1_port" = "no"; then
    OPT_DISABLE_$3=1
  else
    enable_$1_port="yes"
    OPT_DISABLE_$3=0
  fi

  AC_DEFINE_UNQUOTED(OPT_DISABLE_$3, [$]OPT_DISABLE_$3, [XXX])
  AC_SUBST(OPT_DISABLE_$3)

  echo $2 >>ports.all
  if test [$]OPT_DISABLE_$3 = 0; then
    echo $2 >>ports.build
  fi
])

# macro AC_DO_PORT_ENABLER($1, $2, $3, $4)
#   $1 used to access enable_$2_port, e.g. gbz80
#   $2 port name in ports.all and ports.build, e.g. z80
#   $3 OPT_DISABLE_$3, normally uppercase of $2, e.g. GBZ80
#   $4 help string
AC_DEFUN([AC_DO_PORT_ENABLER], [
  AC_ARG_ENABLE($1-port,
                AC_HELP_STRING([--enable-$1-port], $4))

  if test "[$]enable_$1_port" = "yes"; then
    OPT_DISABLE_$3=0
  else
    enable_$1_port="no"
    OPT_DISABLE_$3=1
  fi

  AC_DEFINE_UNQUOTED(OPT_DISABLE_$3, [$]OPT_DISABLE_$3, [XXX])
  AC_SUBST(OPT_DISABLE_$3)

  echo $2 >>ports.all
  if test [$]OPT_DISABLE_$3 = 0; then
    echo $2 >>ports.build
  fi
])

# Now handle the port selection
# ===========================================================================
rm -f ports.all ports.build

# Supported targets
AC_DO_PORT(mcs51,    mcs51,  MCS51,    [Excludes the Intel mcs51 port])
AC_DO_PORT(z80,      z80,    Z80,      [Excludes the Z80 port])
AC_DO_PORT(z180,     z80,    Z180,     [Excludes the Z180 port])
AC_DO_PORT(r2k,      z80,    R2K,      [Excludes the Rabbit 2000 port])
AC_DO_PORT(r3ka,     z80,    R3KA,     [Excludes the Rabbit 3000A port])
AC_DO_PORT(gbz80,    z80,    GBZ80,    [Excludes the Gameboy gbz80 port])
AC_DO_PORT(tlcs90,   z80,    TLCS90,   [Excludes the TLCS-90 port])
AC_DO_PORT(ez80_z80, z80,    EZ80_Z80, [Excludes the EZ80-Z80 port])
AC_DO_PORT(ds390,    ds390,  DS390,    [Excludes the DS390 port])
AC_DEFINE_UNQUOTED(OPT_DISABLE_TININative, $OPT_DISABLE_DS390, [XXX])
AC_DO_PORT(ds400,    ds390,  DS400,    [Excludes the DS400 port])
AC_DO_PORT(pic14,    pic14,  PIC14,    [Excludes the PIC14 port])
AC_DO_PORT(pic16,    pic16,  PIC16,    [Excludes the PIC16 port])
AC_DO_PORT(hc08,     hc08,   HC08,     [Excludes the HC08 port])
AC_DO_PORT(s08,      hc08,   S08,      [Excludes the S08 port])
AC_DO_PORT(stm8,     stm8,   STM8,     [Excludes the STM8 port])
AC_DO_PORT(pdk13,    pdk,    PDK13,    [Excludes the PDK13 port])
AC_DO_PORT(pdk14,    pdk,    PDK14,    [Excludes the PDK14 port])
AC_DO_PORT(pdk15,    pdk,    PDK15,    [Excludes the PDK15 port])
AC_DO_PORT_ENABLER(pdk16,    pdk,    PDK16,    [Includes the PDK16 port])

# Unsupported targets
####AC_DO_PORT_ENABLER(avr,   avr,   AVR,   [Includes the AVR port (disabled by default)])
OPT_DISABLE_AVR=1
AC_DEFINE_UNQUOTED(OPT_DISABLE_AVR, ${OPT_DISABLE_AVR}, [Define to 1 to disable the AVR port])
AC_SUBST(OPT_DISABLE_AVR)

AC_DO_DISABLER(ucsim,      UCSIM,      [Disables configuring and building of ucsim])
AC_DO_DISABLER(device-lib, DEVICE_LIB, [Disables building device libraries])
AC_DO_DISABLER(packihx,    PACKIHX,    [Disables building packihx])
AC_DO_DISABLER(sdcpp,      SDCPP,      [Disables building sdcpp])
AC_DO_DISABLER(sdcdb,      SDCDB,      [Disables building sdcdb])
AC_DO_DISABLER(sdbinutils, SDBINUTILS, [Disables configuring and building of sdbinutils])
AC_DO_DISABLER(non-free,   NON_FREE,   [Disables non-free runtime library parts])

AC_DO_ENABLER(doc,   DOC,   [Enables building the documentation])
AC_CHECK_PROG([LYX],        [lyx],        [lyx],        [:])
AC_CHECK_PROG([LATEX2HTML], [latex2html], [latex2html], [:])
AC_CHECK_PROG([LATEX],      [latex],      [latex],      [:])
AC_CHECK_PROG([DVIPDFM],    [dvipdfm],    [dvipdfm],    [:])
AC_CHECK_PROG([PDFLATEX],   [pdflatex],   [pdflatex],   [:])
AC_CHECK_PROG([MAKEINDEX],  [makeindex],  [makeindex],  [:])

if test $OPT_ENABLE_DOC = 1; then
  SDCC_REQUIRE_PROG($LYX,        lyx)
  SDCC_REQUIRE_PROG($LATEX2HTML, latex2html)
  SDCC_REQUIRE_PROG($LATEX,      latex)
  SDCC_REQUIRE_PROG($DVIPDFM,    dvipdfm)
  SDCC_REQUIRE_PROG($MAKEINDEX,  makeindex)
fi

AC_DO_ENABLER(libgc, LIBGC, [Use the Bohem memory allocator. Lower runtime footprint.])
if test $OPT_ENABLE_LIBGC = 1; then
  AC_CHECK_LIB(gc, GC_malloc)
  if test $ac_cv_lib_gc_GC_malloc = no; then
    AC_MSG_ERROR([Cannot find library libgc with Bohem memory allocator.])
  fi
fi

#remove duplicates
uniq ports.all ports
mv ports ports.all
uniq ports.build ports
mv ports ports.build

# Generating output files
# ===========================================================================
test $OPT_DISABLE_SDCPP   = 0 && AC_CONFIG_SUBDIRS(support/cpp)
test $OPT_DISABLE_PACKIHX = 0 && AC_CONFIG_SUBDIRS(support/packihx)
test $OPT_DISABLE_UCSIM   = 0 && AC_CONFIG_SUBDIRS(sim/ucsim)
test $OPT_DISABLE_SDCDB   = 0 && AC_CONFIG_SUBDIRS(debugger/mcs51)
test $OPT_DISABLE_SDBINUTILS = 0 && AC_CONFIG_SUBDIRS(support/sdbinutils)
AC_CONFIG_FILES([doc/Makefile])

####test $OPT_DISABLE_AVR = 0 && AC_CONFIG_FILES([src/avr/Makefile])

if test $OPT_DISABLE_DS390 = 0 || test $OPT_DISABLE_DS400; then
  AC_CONFIG_FILES([src/ds390/Makefile
                   sdas/as8xcxxx/Makefile])
  test $OPT_DISABLE_DEVICE_LIB = 0 && AC_CONFIG_FILES([device/lib/ds390/Makefile
                                                       device/lib/ds400/Makefile])
fi

if test $OPT_DISABLE_HC08 = 0 || test $OPT_DISABLE_S08 = 0; then
  AC_CONFIG_FILES([src/hc08/Makefile
                   sdas/as6808/Makefile])
  test $OPT_DISABLE_DEVICE_LIB = 0 && AC_CONFIG_FILES([device/lib/hc08/Makefile
                                                       device/lib/s08/Makefile])
fi

if test $OPT_DISABLE_MCS51 = 0; then
  AC_CONFIG_FILES([src/mcs51/Makefile
                   sdas/as8051/Makefile])
  test $OPT_DISABLE_DEVICE_LIB = 0 && AC_CONFIG_FILES([device/lib/mcs51/Makefile
                                                       device/lib/small/Makefile
                                                       device/lib/medium/Makefile
                                                       device/lib/large/Makefile
                                                       device/lib/huge/Makefile])
fi

if test $OPT_DISABLE_PIC14 = 0; then
  AC_CONFIG_FILES(src/pic14/Makefile)
  test $OPT_DISABLE_DEVICE_LIB = 0 && AC_CONFIG_SUBDIRS(device/lib/pic14)
fi
if test $OPT_DISABLE_PIC14 = 0 && test $OPT_DISABLE_NON_FREE = 0; then
  test $OPT_DISABLE_DEVICE_LIB = 0 && AC_CONFIG_SUBDIRS(device/non-free/lib/pic14)
fi
if test $OPT_DISABLE_PIC16 = 0; then
  AC_CONFIG_FILES(src/pic16/Makefile)
  test $OPT_DISABLE_DEVICE_LIB = 0 && AC_CONFIG_SUBDIRS(device/lib/pic16)
fi
if test $OPT_DISABLE_PIC16 = 0 && test $OPT_DISABLE_NON_FREE = 0; then
  test $OPT_DISABLE_DEVICE_LIB = 0 && AC_CONFIG_SUBDIRS(device/non-free/lib/pic16)
fi

if test $OPT_DISABLE_Z80 = 0 || test $OPT_DISABLE_Z180 = 0 || test $OPT_DISABLE_R2K = 0 || test $OPT_DISABLE_R3KA = 0 || test $OPT_DISABLE_GBZ80 = 0 || test $OPT_DISABLE_TLCS90 = 0 || test $OPT_DISABLE_EZ80_Z80 = 0; then
  AC_CONFIG_FILES([src/z80/Makefile])
fi

if test $OPT_DISABLE_Z80 = 0 || test $OPT_DISABLE_Z180 = 0 || test $OPT_DISABLE_EZ80_Z80 = 0; then
  AC_CONFIG_FILES([sdas/asz80/Makefile])
  test $OPT_DISABLE_DEVICE_LIB = 0 && AC_CONFIG_FILES([device/lib/z80/Makefile
                                                       device/lib/z180/Makefile
                                                       device/lib/ez80_z80/Makefile])
fi

if test $OPT_DISABLE_R2K = 0 || test $OPT_DISABLE_R3KA = 0; then
  AC_CONFIG_FILES([sdas/asrab/Makefile])
  test $OPT_DISABLE_DEVICE_LIB = 0 && AC_CONFIG_FILES([device/lib/r2k/Makefile
                                                       device/lib/r3ka/Makefile])
fi

if test $OPT_DISABLE_TLCS90 = 0; then
  AC_CONFIG_FILES([sdas/astlcs90/Makefile])
  test $OPT_DISABLE_DEVICE_LIB = 0 && AC_CONFIG_FILES([device/lib/tlcs90/Makefile])
fi

if test $OPT_DISABLE_GBZ80 = 0; then
  AC_CONFIG_FILES([sdas/asgb/Makefile])
  test $OPT_DISABLE_DEVICE_LIB = 0 && AC_CONFIG_FILES([device/lib/gbz80/Makefile])
fi

if test $OPT_DISABLE_STM8 = 0; then
  AC_CONFIG_FILES([src/stm8/Makefile
                   sdas/asstm8/Makefile])
  test $OPT_DISABLE_DEVICE_LIB = 0 && AC_CONFIG_FILES([device/lib/stm8/Makefile
                                                       device/lib/stm8-large/Makefile])
fi

if test $OPT_DISABLE_PDK13 = 0 || test $OPT_DISABLE_PDK14 = 0 || test $OPT_DISABLE_PDK15 = 0 || test $OPT_DISABLE_PDK16 = 0; then
  AC_CONFIG_FILES([src/pdk/Makefile])
fi

if test $OPT_DISABLE_PDK13 = 0; then
  AC_CONFIG_FILES([sdas/aspdk13/Makefile])
  test $OPT_DISABLE_DEVICE_LIB = 0 && AC_CONFIG_FILES([device/lib/pdk13/Makefile])
fi

if test $OPT_DISABLE_PDK14 = 0; then
  AC_CONFIG_FILES([sdas/aspdk14/Makefile])
  test $OPT_DISABLE_DEVICE_LIB = 0 && AC_CONFIG_FILES([device/lib/pdk14/Makefile])
fi

if test $OPT_DISABLE_PDK15 = 0; then
  AC_CONFIG_FILES([sdas/aspdk15/Makefile])
  test $OPT_DISABLE_DEVICE_LIB = 0 && AC_CONFIG_FILES([device/lib/pdk15/Makefile
                                                       device/lib/pdk15-stack-auto/Makefile])
fi

if test $OPT_DISABLE_PDK16 = 0; then
  AC_CONFIG_FILES([sdas/aspdk16/Makefile])
fi


test $OPT_DISABLE_DEVICE_LIB = 0 && AC_CONFIG_FILES([device/lib/Makefile])
test $OPT_DISABLE_DEVICE_LIB = 0 && test $OPT_DISABLE_NON_FREE = 0 && AC_CONFIG_FILES([device/non-free/lib/Makefile])

AC_CONFIG_FILES([main.mk:main_in.mk
src/Makefile
device/include/Makefile
sdas/linksrc/Makefile
support/makebin/Makefile
support/regression/Makefile
support/valdiag/Makefile
support/scripts/Makefile
support/regression/ports/host/spec.mk:support/regression/ports/host/spec.mk.in
Makefile
Makefile.common:Makefile.common.in
])
if test $OPT_DISABLE_NON_FREE = 0; then
  AC_CONFIG_FILES([device/non-free/include/Makefile])
fi
AC_OUTPUT

# I found no better place
mkdir -p bin

# Prepare result message
# ======================

# In the C-header we need \\ as dir-separator, but in the message only \
dirch=${sdccconf_h_dir_separator}
test ${dirch} = '\\' && dirch='\'

# calc friendly strings
adl_NORMALIZE_PATH_MSG(/${prefix2bin_dir},                         [binPath],  [$dirch])
adl_NORMALIZE_PATH_MSG(/${prefix2data_dir}/${norm_inc_dir_suffix}, [incPath1], [$dirch])
adl_NORMALIZE_PATH_MSG(/${bin2data_dir}/${norm_inc_dir_suffix},    [incPath2], [$dirch])
adl_NORMALIZE_PATH_MSG(${expanded_datadir}/${norm_inc_dir_suffix}, [incPath3], [$dirch])
adl_NORMALIZE_PATH_MSG(/${prefix2data_dir}/${norm_non_free_inc_dir_suffix}, [nonFreeIncPath1], [$dirch])
adl_NORMALIZE_PATH_MSG(/${bin2data_dir}/${norm_non_free_inc_dir_suffix},    [nonFreeIncPath2], [$dirch])
adl_NORMALIZE_PATH_MSG(${expanded_datadir}/${norm_non_free_inc_dir_suffix}, [nonFreeIncPath3], [$dirch])

adl_NORMALIZE_PATH_MSG(/${prefix2data_dir}/${norm_lib_dir_suffix}, [libPath1], [$dirch])
adl_NORMALIZE_PATH_MSG(/${bin2data_dir}/${norm_lib_dir_suffix},    [libPath2], [$dirch])
adl_NORMALIZE_PATH_MSG(${expanded_datadir}/${norm_lib_dir_suffix}, [libPath3], [$dirch])
adl_NORMALIZE_PATH_MSG(/${prefix2data_dir}/${norm_non_free_lib_dir_suffix}, [nonFreeLibPath1], [$dirch])
adl_NORMALIZE_PATH_MSG(/${bin2data_dir}/${norm_non_free_lib_dir_suffix},    [nonFreeLibPath2], [$dirch])
adl_NORMALIZE_PATH_MSG(${expanded_datadir}/${norm_non_free_lib_dir_suffix}, [nonFreeLibPath3], [$dirch])

AC_MSG_RESULT([
sdcc ${VERSION} is now configured for

  Build:                ${build}
  Host:                 ${host}
  Source directory:     ${srcdir}
  Yacc:                 ${YACC}
  C compiler:           ${CC}
  CFLAGS:               ${CFLAGS}
  C++ compiler:         ${CXX}
  CXXFLAGS:             ${CXXFLAGS}
  CPPFLAGS:             ${CPPFLAGS}
  LDFLAGS:              ${LDFLAGS}
  MAKEDEP:              ${MAKEDEP}

  ENABLED Ports:
    ds390               ${enable_ds390_port}
    ds400               ${enable_ds400_port}
    hc08                ${enable_hc08_port}
    s08                 ${enable_s08_port}
    mcs51               ${enable_mcs51_port}
    pic14               ${enable_pic14_port}
    pic16               ${enable_pic16_port}
    z80                 ${enable_z80_port}
    z180                ${enable_z180_port}
    r2k                 ${enable_r2k_port}
    r3ka                ${enable_r3ka_port}
    gbz80               ${enable_gbz80_port}
    tlcs90              ${enable_tlcs90_port}
    ez80_z80            ${enable_ez80_z80_port}
    stm8                ${enable_stm8_port}
    pdk13               ${enable_pdk13_port}
    pdk14               ${enable_pdk14_port}
    pdk15               ${enable_pdk15_port}
    pdk16               ${enable_pdk16_port}

  Disable non-free lib: ${OPT_DISABLE_NON_FREE}
  Disable packihx:      ${OPT_DISABLE_PACKIHX}
  Disable ucsim:        ${OPT_DISABLE_UCSIM}
  Disable device lib:   ${OPT_DISABLE_DEVICE_LIB}
  Disable sdcpp:        ${OPT_DISABLE_SDCPP}
  Disable sdcdb:        ${OPT_DISABLE_SDCDB}
  Disable sdbinutil:    ${OPT_DISABLE_SDBINUTILS}
  Enable documentation: ${OPT_ENABLE_DOC}
  Enable libgc:         ${OPT_ENABLE_LIBGC}

  Install paths:
    binary files:           ${exec_prefix}
    include/library files:  ${datadir}/${inclib_dir_suffix}
    include files:          ${datadir}/${include_dir_suffix}
    library files:          ${datadir}/${lib_dir_suffix}
    non-free files:         ${datadir}/${non_free_inclib_dir_suffix}
    non-free include files: ${datadir}/${non_free_include_dir_suffix}
    non-free library files: ${datadir}/${non_free_lib_dir_suffix}
    documentation:          ${docdir}

    prefix:             ${prefix}
    datadir:            ${datadir}
    datarootdir:        ${datarootdir}

  Search paths (incomplete, see manual for all search paths):
    binary files:       \$SDCC_HOME${binPath}
    include files:      ${incPath1}
                        path(argv[[0]])${incPath2}
                        ${incPath3}
                        ${nonFreeIncPath1}
                        path(argv[[0]])${nonFreeIncPath2}
                        ${nonFreeIncPath3}
    library files:      \$SDCC_HOME${libPath1}${dirch}<model>
                        path(argv[[0]])${libPath2}${dirch}<model>
                        ${libPath3}${dirch}<model>
                        \$SDCC_HOME${nonFreeLibPath1}${dirch}<model>
                        path(argv[[0]])${nonFreeLibPath2}${dirch}<model>
                        ${nonFreeLibPath3}${dirch}<model>
])
# End of configure/configure.in
