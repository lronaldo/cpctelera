#!/bin/sh

AC_PREREQ(2.60)
AC_INIT
AC_CONFIG_AUX_DIR(src)
AC_CONFIG_SRCDIR(.version)
AC_CONFIG_HEADERS([src/core/utils.src/ddconfig.h:src/core/utils.src/ddconfig_in.h])

: ${CFLAGS="-g"}
: ${CXXFLAGS="-g"}

if test -z "$CFLAGS"; then
    CFLAGS="-g"
fi
if test -z "$CXXFLAGS"; then
    CXXFLAGS="-g"
fi

    
in_sdcc="no"
src_root=$(dirname $0)
dnl echo "src_root=$src_root"
if test "$src_root"x == "x" -o "$src_root" == "." ; then
    src_root=$(pwd)
fi
dnl echo "src_root=$src_root"
last_root=$(basename $src_root)
dnl echo "last_root=$last_root"
if test $last_root == "ucsim" ; then
    up_src=$(dirname $src_root)
    if test $(basename $up_src) == "sim" ; then
	in_sdcc="yes"
    fi
fi
echo "SDCC tree detected: $in_sdcc (CFLAGS=${CFLAGS}, CXXFLAGS=${CXXFLAGS})"



AC_PROG_AWK
AC_PROG_SED

AC_MSG_CHECKING(version of the package)
if test -f ${srcdir}/.version; then
  { read VERSION; } < ${srcdir}/.version
  AC_MSG_RESULT($VERSION)
else
  VERSION="0.0.0"
  AC_MSG_RESULT(unknown using 0.0.0)
fi
VERSIONHI=`echo $VERSION|$AWK 'BEGIN {FS="."} {print $1}'`
VERSIONLO=`echo $VERSION|$AWK 'BEGIN {FS="."} {print $2}'`
VERSIONP=`echo $VERSION|$AWK 'BEGIN {FS="."} {print $3}'`
AC_SUBST(VERSION)
AC_SUBST(VERSIONHI)
AC_SUBST(VERSIONLO)
AC_SUBST(VERSIONP)
AC_DEFINE_UNQUOTED(VERSIONSTR, "${VERSION}", [XXX])
AC_DEFINE_UNQUOTED(VERSIONHI, $VERSIONHI, [XXX])
AC_DEFINE_UNQUOTED(VERSIONLO, $VERSIONLO, [XXX])
AC_DEFINE_UNQUOTED(VERSIONP, $VERSIONP, [XXX])
AC_ARG_PROGRAM
# Some options
#==============
# *nix default: "${datadir}/sdcc/doc"
AC_ARG_VAR(docdir, documentation installation directory)
if test "${docdir}" = ""; then
    docdir="\${datadir}"/sdcc/doc
fi
AC_SUBST(docdir)

######################################################################

AC_ARG_ENABLE(in-sdcc,
	      AS_HELP_STRING([--enable-in-sdcc=ARG],
			     [configure as in SDCC tree]),
	      in_sdcc=$enable_in_sdcc
	      ,
	     )

AC_ARG_ENABLE(mcs51-port,
	      AS_HELP_STRING([--enable-mcs51-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_mcs51_port="yes")

AC_ARG_ENABLE(ds390-port,
	      AS_HELP_STRING([--enable-ds390-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_ds390_port="yes")

AC_ARG_ENABLE(ds400-port,
	      AS_HELP_STRING([--enable-ds400-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_ds400_port="yes")

AC_ARG_ENABLE(xa51-port,
	      AS_HELP_STRING([--enable-xa51-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_xa51_port="no")

AC_ARG_ENABLE(z80-port,
	      AS_HELP_STRING([--enable-z80-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_z80_port="yes")

AC_ARG_ENABLE(z180-port,
	      AS_HELP_STRING([--enable-z180-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_z180_port="yes")

AC_ARG_ENABLE(sm83-port,
	      AS_HELP_STRING([--enable--port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_sm83_port="yes")

AC_ARG_ENABLE(ez80_z80-port,
	      AS_HELP_STRING([--enable-ez80_z80-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_ez80_z80_port="yes")

AC_ARG_ENABLE(z80n-port,
	      AS_HELP_STRING([--enable-z80n-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_z80n_port="yes")

AC_ARG_ENABLE(r800-port,
	      AS_HELP_STRING([--enable-r800-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_r800_port="yes")

AC_ARG_ENABLE(tlcs90-port,
	      AS_HELP_STRING([--enable-tlcs90-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_tlcs90_port="yes")

AC_ARG_ENABLE(r2k-port,
	      AS_HELP_STRING([--enable-r2k-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_r2k_port="yes")

AC_ARG_ENABLE(r2ka-port,
	      AS_HELP_STRING([--enable-r2ka-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_r2ka_port="yes")

AC_ARG_ENABLE(r3ka-port,
	      AS_HELP_STRING([--enable-r3ka-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_r3ka_port="yes")

AC_ARG_ENABLE(hc08-port,
	      AS_HELP_STRING([--enable-hc08-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_hc08_port="yes")

AC_ARG_ENABLE(s08-port,
	      AS_HELP_STRING([--enable-s08-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_s08_port="yes")

AC_ARG_ENABLE(mos6502-port,
	      AS_HELP_STRING([--enable-mos6502-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_mos6502_port="yes")

AC_ARG_ENABLE(mos65c02-port,
	      AS_HELP_STRING([--enable-mos65c02-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_mos65c02_port="yes")

AC_ARG_ENABLE(stm8-port,
	      AS_HELP_STRING([--enable-stm8-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_stm8_port="yes")

AC_ARG_ENABLE(pdk13-port,
	      AS_HELP_STRING([--enable-pdk13-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_pdk13_port="yes")

AC_ARG_ENABLE(pdk14-port,
	      AS_HELP_STRING([--enable-pdk14-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_pdk14_port="yes")

AC_ARG_ENABLE(pdk15-port,
	      AS_HELP_STRING([--enable-pdk15-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_pdk15_port="yes")

AC_ARG_ENABLE(pdk16-port,
	      AS_HELP_STRING([--enable-pdk16-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_pdk16_port="no")

AC_ARG_ENABLE(f8-port,
	      AS_HELP_STRING([--enable-f8-port=ARG],
			     [feature opt passed from SDCC]),
	      ,
	      enable_f8_port="yes")

def_dlso="no"
def_feapp="yes"
def_serio="yes"

def_mcs51="yes"
def_i8048="yes"
def_i8085="yes"
def_xa="yes"
def_avr="yes"
def_z80="yes"
def_tlcs="yes"
def_rxk="yes"
def_m6800="yes"
def_m68hc08="yes"
def_m6809="yes"
def_m68hc11="yes"
def_m68hc12="yes"
def_mos6502="yes"
def_st7="yes"
def_stm8="yes"
def_pdk="yes"
def_pblaze="yes"
def_p1516="yes"
def_f8="yes"
def_oisc="yes"

if test $in_sdcc == "yes" ; then
    def_feapp="no"
    def_serio="no"
    def_i8048="no"
    def_i8085="no"
    def_avr="no"
    def_m6800="no"
    def_m6809="no"
    def_m68hc11="no"
    def_m68hc12="no"
    def_st7="no"
    def_pblaze="no"
    def_p1516="no"
    def_oisc="no"
    SILENT="yes"
    
    def_mcs51="no"
    def_xa="no"
    if test $enable_mcs51_port = "yes"; then
	def_mcs51="yes"
	def_xa="yes"
    fi
    if test $enable_ds390_port = "yes"; then
	def_mcs51="yes"
	def_xa="yes"
    fi
    if test $enable_ds400_port = "yes"; then
	def_mcs51="yes"
	def_xa="yes"
    fi

    if test $enable_xa51_port = "yes"; then
	def_xa="yes"
    fi
    
    def_z80="no"
    if test $enable_z80_port = "yes"; then
	def_z80="yes"
    fi
    if test $enable_z180_port = "yes"; then
	def_z80="yes"
    fi
    if test $enable_sm83_port = "yes"; then
	def_z80="yes"
    fi
    if test $enable_ez80_z80_port = "yes"; then
	def_z80="yes"
    fi
    if test $enable_z80n_port = "yes"; then
	def_z80="yes"
    fi
    if test $enable_r800_port = "yes"; then
	def_z80="yes"
    fi

    def_tlcs="no"
    if test $enable_tlcs90_port = "yes"; then
	def_tlcs="yes"
    fi

    def_rxk="no"
    if test $enable_r2k_port = "yes"; then
	def_rxk="yes"
    fi
    if test $enable_r2ka_port = "yes"; then
	def_rxk="yes"
    fi
    if test $enable_r3ka_port = "yes"; then
	def_rxk="yes"
    fi

    def_m68hc08="no"
    if test $enable_hc08_port = "yes"; then
	def_m68hc08="yes"
    fi
    if test $enable_s08_port = "yes"; then
	def_m68hc08="yes"
    fi

    def_mos6502="no"
    if test $enable_mos6502_port = "yes"; then
	def_mos6502="yes"
    fi
    if test $enable_mos65c02_port = "yes"; then
	def_mos65c02="yes"
    fi

    def_stm8="no"
    if test $enable_stm8_port = "yes"; then
	def_stm8="yes"
    fi

    def_pdk="no"
    if test $enable_pdk13_port = "yes"; then
	def_pdk="yes"
    fi
    if test $enable_pdk14_port = "yes"; then
	def_pdk="yes"
    fi
    if test $enable_pdk15_port = "yes"; then
	def_pdk="yes"
    fi
    if test $enable_pdk16_port = "yes"; then
	def_pdk="yes"
    fi

    def_f8="no"
    if test $enable_f8_port = "yes"; then
	def_f8="yes"
    fi
else
    SILENT="no"
fi
AC_SUBST(SILENT)

######################################################################

AC_ARG_ENABLE(dlso,
	      AS_HELP_STRING([--enable-dlso=ARG],
			     [en/disable compile shared objects]),
	      ,
	      enable_dlso=$def_dlso)

AC_ARG_ENABLE(feapp,
	      AS_HELP_STRING([--enable-feapp=ARG],
			     [en/disable compile of exe wrapper (unfinished)]),
	      ,
	      enable_feapp=$def_feapp)

AC_ARG_ENABLE(serio,
	      AS_HELP_STRING([--enable-serio=ARG],
			     [en/disable compile serio GUI tool (needs curses)]),
	      ,
	      enable_serio=$def_serio)

AC_ARG_ENABLE(mcs51-sim,
	      AS_HELP_STRING([--enable-mcs51-sim=ARG],
			     [en/disable compile mcs51 simulator]),
	      ,
	      enable_mcs51_sim=$def_mcs51)

AC_ARG_ENABLE(i8048-sim,
	      AS_HELP_STRING([--enable-i8048-sim=ARG],
			     [en/disable compile i8048 simulator]),
	      ,
	      enable_i8048_sim=$def_i8048)

AC_ARG_ENABLE(i8085-sim,
	      AS_HELP_STRING([--enable-i8085-sim=ARG],
			     [en/disable compile i8085 simulator]),
	      ,
	      enable_i8085_sim=$def_i8085)

AC_ARG_ENABLE(xa-sim,
	      AS_HELP_STRING([--enable-xa-sim=ARG],
			     [en/disable compile xa simulator]),
	      ,
	      enable_xa_sim=$def_xa)

AC_ARG_ENABLE(avr-sim,
	      AS_HELP_STRING([--enable-avr-sim=ARG],
			     [en/disable compile avr simulator]),
	      ,
	      enable_avr_sim=$def_avr)

AC_ARG_ENABLE(z80-sim,
	      AS_HELP_STRING([--enable-z80-sim=ARG],
			     [en/disable compile z80 simulator]),
	      ,
	      enable_z80_sim=$def_z80)

AC_ARG_ENABLE(tlcs-sim,
	      AS_HELP_STRING([--enable-tlcs-sim=ARG],
			     [en/disable compile tlcs simulator]),
	      ,
	      enable_tlcs_sim=$def_tlcs)

AC_ARG_ENABLE(rxk-sim,
	      AS_HELP_STRING([--enable-rxk-sim=ARG],
			     [en/disable compile Rabbit rXk simulator]),
	      ,
	      enable_rxk_sim=$def_rxk)

AC_ARG_ENABLE(m6800-sim,
	      AS_HELP_STRING([--enable-m6800-sim=ARG],
			     [en/disable compile m6800 simulator]),
	      ,
	      enable_m6800_sim=$def_m6800)

AC_ARG_ENABLE(m68hc08-sim,
	      AS_HELP_STRING([--enable-m68hc08-sim=ARG],
			     [en/disable compile m68hc08 simulator]),
	      ,
	      enable_m68hc08_sim=$def_m68hc08)

AC_ARG_ENABLE(m6809-sim,
	      AS_HELP_STRING([--enable-m6809-sim=ARG],
			     [en/disable compile m6809 simulator]),
	      ,
	      enable_m6809_sim=$def_m6809)

AC_ARG_ENABLE(m68hc11-sim,
	      AS_HELP_STRING([--enable-m68hc11-sim=ARG],
			     [en/disable compile m68hc11 simulator]),
	      ,
	      enable_m68hc11_sim=$def_m68hc11)

AC_ARG_ENABLE(m68hc12-sim,
	      AS_HELP_STRING([--enable-m68hc12-sim=ARG],
			     [en/disable compile m68hc12 simulator]),
	      ,
	      enable_m68hc12_sim=$def_m68hc12)

AC_ARG_ENABLE(mos6502-sim,
	      AS_HELP_STRING([--enable-mos6502-sim=ARG],
			     [en/disable compile mos6502 simulator]),
	      ,
	      enable_mos6502_sim=$def_mos6502)

AC_ARG_ENABLE(st7-sim,
	      AS_HELP_STRING([--enable-st7-sim=ARG],
			     [en/disable compile st7 simulator]),
	      ,
	      enable_st7_sim=$def_st7)

AC_ARG_ENABLE(stm8-sim,
	      AS_HELP_STRING([--enable-stm8-sim=ARG],
			     [en/disable compile stm8 simulator]),
	      ,
	      enable_stm8_sim=$def_stm8)

AC_ARG_ENABLE(pdk-sim,
	      AS_HELP_STRING([--enable-pdk-sim=ARG],
			     [en/disable compile pdk simulator]),
	      ,
	      enable_pdk_sim=$def_pdk)

AC_ARG_ENABLE(pblaze-sim,
	      AS_HELP_STRING([--enable-pblaze-sim=ARG],
			     [en/disable compile picoblaze simulator]),
	      ,
	      enable_pblaze_sim=$def_pblaze)

AC_ARG_ENABLE(p1516-sim,
	      AS_HELP_STRING([--enable-p1516-sim=ARG],
			     [en/disbale compile p1516 simulator]),
	      ,
	      enable_p1516_sim=$def_p1516)

AC_ARG_ENABLE(f8-sim,
	      AS_HELP_STRING([--enable-f8-sim=ARG],
			     [en/disable compile f8 simulator]),
	      ,
	      enable_f8_sim=$def_f8)

AC_ARG_ENABLE(oisc-sim,
	      AS_HELP_STRING([--enable-oisc-sim=ARG],
			     [en/disable compile oisc simulator]),
	      ,
	      enable_oisc_sim=$def_oisc)

######################################################################

AC_SUBST(enable_dlso)
AC_SUBST(enable_feapp)
AC_SUBST(enable_serio)

AC_SUBST(enable_mcs51_sim)
AC_SUBST(enable_i8048_sim)
AC_SUBST(enable_i8085_sim)
AC_SUBST(enable_xa_sim)
AC_SUBST(enable_avr_sim)
AC_SUBST(enable_z80_sim)
AC_SUBST(enable_tlcs_sim)
AC_SUBST(enable_rxk_sim)
AC_SUBST(enable_m6800_sim)
AC_SUBST(enable_m68hc08_sim)
AC_SUBST(enable_m6809_sim)
AC_SUBST(enable_m68hc11_sim)
AC_SUBST(enable_m68hc12_sim)
AC_SUBST(enable_mos6502_sim)
AC_SUBST(enable_st7_sim)
AC_SUBST(enable_stm8_sim)
AC_SUBST(enable_pdk_sim)
AC_SUBST(enable_pblaze_sim)
AC_SUBST(enable_p1516_sim)
AC_SUBST(enable_f8_sim)
AC_SUBST(enable_oisc_sim)


dnl AC_ARG_ENABLE(statistic,
dnl [  --enable-statistic      compile statistical features (slower simulation)],
dnl if test $enable_statistic != "no"; then
dnl    enable_statistic="yes"
dnl fi,
dnl enable_statistic="no")
dnl AC_DEFINE_UNQUOTED(STATISTIC, $enable_statistic, [XXX])


# Required programs
#-------------------
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_INSTALL
AC_PROG_RANLIB
dnl AC_LANG_CPLUSPLUS

AC_CHECK_TOOL(STRIP, strip)
AC_CHECK_TOOL(AR, ar)
dnl AC_CHECK_TOOL(SREC_CAT, srec_cat)

dnl AC_LIBTOOL_DLOPEN
dnl dnl AC_LIBTOOL_WIN32_DLL
dnl AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

AC_PROG_LEX(noyywrap)
if test ${LEX} = "flex"; then
        AC_DEFINE(HAVE_FLEX, 1, [XXX])
fi
AC_PROG_YACC


AC_LANG([C++])

# Checking for header files.
# ===========================================================================

#if test -d /stuff/include; then
#  CPPFLAGS="$CPPFLAGS -I/stuff/include"
#fi

dnl AC_HEADER_STDC
AC_CHECK_HEADERS(endian.h sys/endian.h machine/endian.h sys/isa_defs.h)
AC_CHECK_HEADERS(getopt.h)
AC_CHECK_HEADERS(unistd.h)
AC_CHECK_HEADERS(termios.h)
AC_CHECK_HEADERS(inttypes.h)
AC_CHECK_HEADERS(cstdint.h)
AC_CHECK_HEADERS(stdint.h)
AC_CHECK_HEADERS(sys/wait.h)
AC_CHECK_HEADERS(wincon.h)
AC_CHECK_HEADERS(sys/select.h)

AC_CHECK_HEADER(curses.h,
  AC_DEFINE(HAVE_CURSES_H, 1, [XXX])
  curses_h_ok=yes,
  curses_h_ok=no
)

ucsim_cv_socket="unknown"
AC_CHECK_HEADER(sys/socket.h,
  AC_DEFINE(HAVE_SYS_SOCKET_H, 1, [XXX])
  AC_DEFINE(SOCKET_AVAIL, 1, [XXX])
  AC_DEFINE_UNQUOTED(UCSOCKET_T, int, [XXX])
  ucsim_cv_socket="<sys/socket.h>"
)

WINSOCK_AVAIL=0
if test $ucsim_cv_socket = unknown; then
  AC_CHECK_HEADER(winsock2.h,
    AC_DEFINE(HAVE_WINSOCK2_H, 1, [XXX])
    AC_DEFINE(SOCKET_AVAIL, 1, [XXX])
    AC_DEFINE_UNQUOTED(UCSOCKET_T, SOCKET, [XXX])
    ucsim_cv_socket="<winsock2.h>"
    WINSOCK_AVAIL=1
  )
fi
AC_SUBST(WINSOCK_AVAIL)

if test $ucsim_cv_socket != unknown; then
  AC_EGREP_HEADER(socklen_t,
    $ucsim_cv_socket,
    AC_DEFINE_UNQUOTED(SOCKLEN_T, socklen_t, [XXX]),
    AC_DEFINE_UNQUOTED(SOCKLEN_T, uint, [XXX]))
fi
AC_DEFINE_UNQUOTED(HEADER_SOCKET, ${ucsim_cv_socket}, [XXX])

AC_CHECK_HEADER(dlfcn.h)
AC_HEADER_DIRENT

# This must be after CXXCPP

AC_CACHE_CHECK(which header file defines FD_ macros, ucsim_cv_fd,
[ucsim_cv_fd="unknown"
AC_EGREP_CPP(yes, [
#include <winsock2.h>
#ifdef FD_ZERO
yes
#endif], ucsim_cv_fd="<winsock2.h>")
AC_EGREP_CPP(yes, [
#include <sys/time.h>
#ifdef FD_ZERO
yes
#endif], ucsim_cv_fd="<sys/time.h>")
AC_EGREP_CPP(yes, [
#include <sys/types.h>
#ifdef FD_ZERO
yes
#endif], ucsim_cv_fd="<sys/types.h>")
AC_EGREP_CPP(yes, [
#include <sys/select.h>
#ifdef FD_ZERO
yes
#endif], ucsim_cv_fd="<sys/select.h>")
AC_EGREP_CPP(yes, [
#include <gnu/types.h>
#ifdef FD_ZERO
yes
#endif], ucsim_cv_fd="<gnu/types.h>")])

ucsim_cv_header_ok=0
if echo $ucsim_cv_fd|grep time >/dev/null 2>&1; then
AC_DEFINE(FD_NEED_TIME_H, 1, [XXX])
ucsim_cv_header_ok=1
else
AC_DEFINE(FD_NEED_TIME_H, 0, [XXX])
fi

if echo $ucsim_cv_fd|grep sys/types >/dev/null 2>&1; then
AC_DEFINE(FD_NEED_TYPES_H, 1, [XXX])
ucsim_cv_header_ok=1
else
AC_DEFINE(FD_NEED_TYPES_H, 0, [XXX])
fi

if echo $ucsim_cv_fd|grep gnu/types >/dev/null 2>&1; then
AC_DEFINE(FD_NEED_GNUTYPES_H, 1, [XXX])
ucsim_cv_header_ok=1
else
AC_DEFINE(FD_NEED_GNUTYPES_H, 0, [XXX])
fi

if echo $ucsim_cv_fd|grep select >/dev/null 2>&1; then
AC_DEFINE(FD_NEED_SELECT_H, 1, [XXX])
ucsim_cv_header_ok=1
else
AC_DEFINE(FD_NEED_SELECT_H, 0, [XXX])
fi

if echo $ucsim_cv_fd|grep winsock2 >/dev/null 2>&1; then
AC_DEFINE(FD_NEED_WINSOCK2_H, 1, [XXX])
ucsim_cv_header_ok=1
else
AC_DEFINE(FD_NEED_WINSOCK2_H, 0, [XXX])
fi

dnl AC_DEFINE_UNQUOTED(FD_HEADER_OK, ${ucsim_cv_header_ok}, [XXX])
AC_DEFINE_UNQUOTED(HEADER_FD, ${ucsim_cv_fd}, [XXX])


# Checking for functions/libs
# ===========================================================================
if echo $ucsim_cv_socket|grep winsock2 >/dev/null 2>&1; then
  LIBS="-lws2_32 $LIBS"
else
  AC_CHECK_LIB(socket,socket)
  AC_CHECK_LIB(nsl,xdr_short)
  AC_CHECK_LIB(rt,nanosleep)
fi
AC_CHECK_LIB(m,fmod)

AC_CHECK_LIB(dl,dlopen,
dl_ok="yes"
DL="-ldl",
dl_ok="no"
DL="")
AC_SUBST(dl_ok)
AC_SUBST(DL)

CURSES_LIBS=""
if test $curses_h_ok != yes; then
  panel_ok="no"
  curses_ok="no"
else
  AC_CHECK_LIB(panel,panel_above,
    panel_ok="yes"
    curses_ok="no"
    CURSES_LIBS="-lpanel",
    panel_ok="no"
    curses_ok="no"
  )

  if test $curses_ok != yes; then
    AC_CHECK_LIB(curses,nl,
      curses_ok="yes"
      CURSES_LIBS="${CURSES_LIBS} -lcurses",
      curses_ok="no")
  fi

  if test $curses_ok != yes; then
    AC_CHECK_LIB(ncurses,nl,
      curses_ok="yes"
      CURSES_LIBS="${CURSES_LIBS} -lncurses",
      curses_ok="no")
  fi

  AC_CHECK_LIB(tinfo,curs_set,
	       curses_ok="yes"
	       CURSES_LIBS="${CURSES_LIBS} -ltinfo",
	       ,
	       $CURSES_LIBS)
  
fi


AC_SUBST(panel_ok)
AC_SUBST(curses_ok)
AC_SUBST(CURSES_LIBS)

AC_FUNC_FORK
AC_CHECK_FUNCS(_exit dup2 dup3 pipe pipe2 waitpid)
AC_CHECK_FUNCS(vasprintf)
AC_CHECK_FUNCS(vsnprintf)
dnl AC_CHECK_FUNCS(strdup)
dnl AC_CHECK_FUNCS(getline)
dnl AC_CHECK_FUNCS(getdelim)
AC_CHECK_FUNCS(yylex)
AC_CHECK_FUNCS(nanosleep usleep)
AC_CHECK_FUNCS(fmod)

# AC_CACHE_CHECK(whether scanf knows %a, ucsim_cv_scanf_a,
# [echo "a b" >conftest.data
# AC_TRY_RUN([
# #include <stdio.h>
# void main(void)
# {
#   FILE *f;
#   char *a;
#   int  i;
#   if ((f= fopen("conftest.data", "r")) == NULL)
#     exit(1);
#   i= fscanf(f, " %a[a-z] ", &a);
#   fclose(f);
#   if (i == 0)
#     exit(2);
#   if (i == 1)
#     exit(0);
#   exit(3);
# }],
# ucsim_cv_scanf_a="yes",
# ucsim_cv_scanf_a="no",
# ucsim_cv_scanf_a="unknown")
# ])
# if test "$ucsim_cv_scanf_a" = "yes"; then
#   AC_DEFINE(SCANF_A, 1, [XXX])
# fi

# AC_CACHE_CHECK(whether getcwd is GNUish, ucsim_cv_getcwd,
# [AC_TRY_RUN([
# #include <unistd.h>
# void main(void)
# {
#   if ((getcwd(NULL, 0)) == NULL)
#     exit(1);
#   exit(0);
# }],
# ucsim_cv_getcwd="yes",
# ucsim_cv_getcwd="no",
# ucsim_cv_getcwd="unknown")
# ])
# if test "$ucsim_cv_getcwd" = "yes"; then
#   AC_DEFINE(GNU_GETCWD, 1, [XXX])
# fi

#AC_DEFUN([ucsim_ACCEPT_LENGTH_T],
# [AC_CACHE_CHECK([for type of length pointer parameter of accept],
#   ucsim_cv_accept_length_type,
#   [ac_save_CPPFLAGS="$CPPFLAGS"
#    ucsim_cv_accept_length_type=no
#    for ac_val in int size_t socklen_t; do
#      CPPFLAGS="$ac_save_CPPFLAGS -DACCEPT_SOCKLEN_T=$ac_val"
#      AC_TRY_COMPILE([#include <sys/types.h>
#        [struct sockaddr a; $ac_val len; accept (0, &a, &len);],
#        [ucsim_cv_accept_length_type=$ac_val; break])
#    done
#    CPPFLAGS="$ac_save_CPPFLAGS"])
#  if test "$ucsim_cv_accept_length_type" != no; then
#    AC_DEFINE_UNQUOTED(ACCEPT_SOCKLEN_T, $ucsim_cv_accept_length_type,
#      [Define to be the type of length parameter of accept (without the \*').])
#  fi
# ]
#)
##'
#
#ucsim_ACCEPT_LENGTH_T

# Macro definitions
###################

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
[arg1=`echo $1 | sed -e"s/^-*//" -e"s/-/_/g"`
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

# Checking characteristics of compilers and other programs
# --------------------------------------------------------

# Cross compiling overrides
# FIXME
if test "$CC" = "i586-mingw32msvc-gcc"; then
  ac_cv_c_bigendian=no
  ac_cv_sizeof_char=1
  ac_cv_sizeof_short=2
  ac_cv_sizeof_int=4
  ac_cv_sizeof_long=4
  ac_cv_sizeof_long_long=4
fi


# Checking whether byte ordering is bigendian
# --------------------------------------------------------
AC_C_BIGENDIAN

PICOPT=""
SHAREDLIB="no"

if test "$ac_cv_cxx_compiler_gnu" = "yes"; then
  AC_CACHE_CHECK(whether preprocessor accepts -MM or -M,ucsim_cv_MM,
  echo "#include <stdio.h>" >_test_.c
  echo "" >>_test_.c
  $CXXCPP -v -MM _test_.c 1>&5 2>&5
  if test "$?" = "0"; then
    ucsim_cv_MM="-MM"
  else
    ucsim_cv_MM="-M"
  fi
  rm -f _test_.*)

  # This is the first time when CFLAGS are set/modified!!

dnl  sdcc_IS_VALID_OPTION(-ggdb)
#  if test "$sdcc_cv_cxx_ggdb" = "yes"; then
#      CXXFLAGS="$CXXFLAGS -ggdb"
#  fi
dnl  sdcc_IS_VALID_OPTION(-gstabs)
  sdcc_IS_VALID_OPTION(-pipe)
  if test "$sdcc_cv_cxx_pipe" = "yes"; then
    CXXFLAGS="$CXXFLAGS -pipe"
  fi
  
dnl  sdcc_IS_VALID_OPTION(-fhandle-exceptions)
#  if test "$sdcc_cv_cxx_fhandle_exceptions" = "yes"; then
#    CXXFLAGS="$CXXFLAGS -fhandle-exceptions"
#  fi
  
  MAKEDEP="$CXX $ucsim_cv_MM"
  WALL_FLAG="-Wall"

  sdcc_IS_VALID_OPTION(-Wno-overloaded-virtual)
  if test "$sdcc_cv_cxx_Wno_overloaded_virtual" = "yes"; then
      WALL_FLAG="$WALL_FLAG -Wno-overloaded-virtual"
  fi

  sdcc_IS_VALID_OPTION(-Wno-unused-parameters)
  if test "$sdcc_cv_cxx_Wno_unused_parameters" = "yes"; then
      WALL_FLAG="$WALL_FLAG -Wno-unused-parameters"
  fi

  sdcc_IS_VALID_OPTION(-Wno-unused)
  if test "$sdcc_cv_cxx_Wno_unused" = "yes"; then
      WALL_FLAG="$WALL_FLAG -Wno-unused"
  fi
else
  sdcc_IS_SUNPRO
  if test "$sdcc_cv_cxx_compiler_sunpro" = "yes"; then
    CXXFLAGS="-xO2 $CXXFLAGS"
    MAKEDEP="$CXX -xM1"
    WALL_FLAG=
  fi
fi

# pic_flag is defined by AC_PROG_LIBTOOL
PICOPT=$pic_flag

if test "$PICOPT"x != "x"; then
    SHAREDLIB="yes"
fi
AC_SUBST(MAKEDEP)
AC_SUBST(WALL_FLAG)
AC_SUBST(SHAREDLIB)
AC_SUBST(PICOPT)

dlso_ok="no"
if test $SHAREDLIB = "yes" -a $dl_ok = "yes" -a $enable_dlso = "yes"; then
    dlso_ok="yes"
fi
AC_SUBST(dlso_ok)

# Checks for typedefs, structures, and compiler characteristics.
# ===========================================================================

if $CXX -v 2>&1|grep "mingw" >/dev/null 2>&1; then
    ac_cv_sizeof_char=1
    ac_cv_sizeof_short=2
    ac_cv_sizeof_int=4
    ac_cv_sizeof_long=4
    ac_cv_sizeof_long_long=4
else
    AC_CHECK_SIZEOF(char)
    AC_CHECK_SIZEOF(short)
    AC_CHECK_SIZEOF(int)
    AC_CHECK_SIZEOF(long)
    AC_CHECK_SIZEOF(long long)
    AC_CHECK_SIZEOF(void *)
fi

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

specifier_name()
{
    if expr "$ac_cv_sizeof_char" '>=' "$1" >/dev/null; then
        echo "\"\""
        exit
    fi
    if expr "$ac_cv_sizeof_short" '>=' "$1" >/dev/null; then
        echo "\"h\""
        exit
    fi
    if expr "$ac_cv_sizeof_int" '>=' "$1" >/dev/null; then
        echo "\"\""
        exit
    fi
    if expr "$ac_cv_sizeof_long" '>=' "$1" >/dev/null; then
        echo "\"l\""
        exit
    fi
    if expr "$ac_cv_sizeof_long_long" '>=' "$1" >/dev/null; then
        echo "\"L\""
        exit
    fi
    echo "\"L\""
}

AC_MSG_CHECKING(type name for byte)
TYPE_BYTE=`type_name 1`
AC_MSG_RESULT($TYPE_BYTE)
AC_MSG_CHECKING(specifier for byte)
SPEC_BYTE=`specifier_name 1`
AC_MSG_RESULT($SPEC_BYTE)
AC_MSG_CHECKING(type name for word)
TYPE_WORD=`type_name 2`
AC_MSG_RESULT($TYPE_WORD)
AC_MSG_CHECKING(specifier for word)
SPEC_WORD=`specifier_name 2`
AC_MSG_RESULT($SPEC_WORD)
AC_MSG_CHECKING(type name for dword)
TYPE_DWORD=`type_name 4`
AC_MSG_RESULT($TYPE_DWORD)
AC_MSG_CHECKING(specifier for dword)
SPEC_DWORD=`specifier_name 4`
AC_MSG_RESULT($SPEC_DWORD)
AC_MSG_CHECKING(type name for qword)
TYPE_QWORD=`type_name 8`
AC_MSG_RESULT($TYPE_QWORD)
AC_MSG_CHECKING(specifier for qword)
SPEC_QWORD=`specifier_name 8`
AC_MSG_RESULT($SPEC_QWORD)

AC_DEFINE_UNQUOTED(TYPE_BYTE, [$TYPE_BYTE], [XXX])
AC_DEFINE_UNQUOTED(SPEC_BYTE, [$SPEC_BYTE], [XXX])
AC_DEFINE_UNQUOTED(TYPE_WORD, [$TYPE_WORD], [XXX])
AC_DEFINE_UNQUOTED(SPEC_WORD, [$SPEC_WORD], [XXX])
AC_DEFINE_UNQUOTED(TYPE_DWORD, [$TYPE_DWORD], [XXX])
AC_DEFINE_UNQUOTED(SPEC_DWORD, [$SPEC_DWORD], [XXX])
AC_DEFINE_UNQUOTED(TYPE_QWORD, [$TYPE_QWORD], [XXX])
AC_DEFINE_UNQUOTED(SPEC_QWORD, [$SPEC_QWORD], [XXX])
#AC_DEFINE_UNQUOTED(TYPE_UBYTE, [unsigned $TYPE_BYTE], [XXX])
#AC_DEFINE_UNQUOTED(TYPE_UWORD, [unsigned $TYPE_WORD], [XXX])
#AC_DEFINE_UNQUOTED(TYPE_UDWORD, [unsigned $TYPE_DWORD], [XXX])

AH_TOP([#ifndef DDCONFIG_HEADER
#define DDCONFIG_HEADER])
AH_BOTTOM([#endif])
	
if echo $TYPE_DWORD|grep short >/dev/null; then
    A="h"
elif echo $TYPE_DWORD|grep long >/dev/null; then
    A="l"
else
    A=""
fi

if echo $TYPE_WORD|grep short >/dev/null; then M="h"
elif echo $TYPE_WORD|grep long >/dev/null; then M="l"
else M=""
fi

AC_DEFINE_UNQUOTED(_A_, "${A}", [XXX])
AC_DEFINE_UNQUOTED(_M_, "${M}", [XXX])

# Generating output files
# ===========================================================================

AC_CONFIG_FILES(Makefile
src/Makefile
src/common.mk:src/common_in.mk
src/packages.mk:src/packages_in.mk

src/core/cmd.src/Makefile
src/core/motorola.src/Makefile
src/core/utils.src/Makefile
src/core/sim.src/Makefile
src/core/sim.src/test/Makefile
src/core/gui.src/Makefile

src/sims/avr.src/Makefile     src/sims/avr.src/test/Makefile
src/sims/m68hc08.src/Makefile src/sims/m68hc08.src/test/Makefile
src/sims/m6809.src/Makefile   src/sims/m6809.src/test/Makefile
src/sims/m6800.src/Makefile   src/sims/m6800.src/test/Makefile
src/sims/m68hc11.src/Makefile src/sims/m68hc11.src/test/Makefile
src/sims/m68hc12.src/Makefile src/sims/m68hc12.src/test/Makefile
src/sims/mos6502.src/Makefile src/sims/mos6502.src/test/Makefile
src/sims/rxk.src/Makefile     src/sims/rxk.src/test/Makefile
src/sims/pblaze.src/Makefile  src/sims/pblaze.src/test/Makefile
src/sims/i8085.src/Makefile   src/sims/i8085.src/test/Makefile
src/sims/f8.src/Makefile      src/sims/f8.src/test/Makefile
src/sims/i8048.src/Makefile   src/sims/i8048.src/test/Makefile
src/sims/p1516.src/Makefile   src/sims/p1516.src/test/Makefile
src/sims/pdk.src/Makefile     src/sims/pdk.src/test/Makefile
src/sims/s51.src/Makefile     src/sims/s51.src/test/Makefile
src/sims/st7.src/Makefile     src/sims/st7.src/test/Makefile
src/sims/stm8.src/Makefile    src/sims/stm8.src/test/Makefile
src/sims/tlcs.src/Makefile    src/sims/tlcs.src/test/Makefile
src/sims/xa.src/Makefile      src/sims/xa.src/test/Makefile
src/sims/z80.src/Makefile     src/sims/z80.src/test/Makefile
src/sims/oisc.src/Makefile    src/sims/oisc.src/test/Makefile

src/apps/serio.src/Makefile
src/apps/ucsim.src/Makefile

docs/Makefile
)
AC_OUTPUT

echo "Compile for SDCC  $in_sdcc"
echo "Compile dlso      $enable_dlso"
echo "Compile feapp     $enable_feapp"
echo "Compile s51       $enable_mcs51_sim"
echo "Compile avr       $enable_avr_sim"
echo "Compile z80       $enable_z80_sim"
echo "Compile tlcs      $enable_tlcs_sim"
echo "Compile m68hc08   $enable_m68hc08_sim"
echo "Compile xa        $enable_xa_sim"
echo "Compile stm8      $enable_stm8_sim"
echo "Compile pdk       $enable_pdk_sim"
echo "Compile st7       $enable_st7_sim"
echo "Compile p1516     $enable_p1516_sim"
echo "Compile m6809     $enable_m6809_sim"
echo "Compile m6800     $enable_m6800_sim"
echo "Compile m68hc11   $enable_m68hc11_sim"
echo "Compile m68hc12   $enable_m68hc12_sim"
echo "Compile mos6502   $enable_mos6502_sim"
echo "Compile rxk       $enable_rxk_sim"
echo "Compile pblaze    $enable_pblaze_sim"
echo "Compile i8085     $enable_i8085_sim"
echo "Compile f8        $enable_f8_sim"
echo "Compile i8048     $enable_i8048_sim"
echo "Compile oisc      $enable_oisc_sim"
echo "Compile serio     $enable_serio"
echo "v${VERSION}"

#gui.src/portmon.src/Makefile


# End of configure.ac
