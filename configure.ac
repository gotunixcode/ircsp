AC_INIT([IRCSP], [5.0.0], [software@gotunix.net])
AC_PREREQ([0.0.0])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([IRCSP], [5.0.0])
#AM_CONFIG_HEADER(./config.h)
AC_CONFIG_HEADERS(./config.h)

AC_DEFUN([CONFIGURE_PART],[
  AC_MSG_RESULT()
  AC_MSG_RESULT([${T_MD}$1${T_ME}])
])


CONFIGURE_PART(Checking System)
#AC_CANONICAL_BUILD
#AC_CANONICAL_HOST
#AC_ISC_POSIX

AC_GNU_SOURCE
AC_C_BIGENDIAN
#AC_PROG_CC_C99
#AM_PROG_CC_C_O

CONFIGURE_PART(Checking OS and Arch)
# Determine the operating system
AC_MSG_CHECKING([host operating system])
OSNAME=`uname -s`
case "$OSNAME" in
        FreeBSD*)
                AC_MSG_RESULT(FreeBSD)
		AC_DEFINE([FREEBSD], [1], [Compiled on FreeBSD])
        ;;
        Linux*)
                AC_MSG_RESULT(Linux)
		AC_DEFINE([LINUX], [1], [Compiled on Linux])
        ;;
        *)
                AC_MSG_RESULT(no)
        ;;
esac


AC_MSG_CHECKING([host operating system architecture])
ARCH=`uname -m`
case "$ARCH" in
        amd64*)
                AC_MSG_RESULT(x64)
                AC_DEFINE([X64], [1], [64Bit])
                AC_SUBST(ARCH,x64)
        ;;
        x86_64*)
                AC_MSG_RESULT(x64)
                AC_DEFINE([X64], [1], [64Bit])
                AC_SUBST(ARCH,x64)
        ;;
        *)
                AC_MSG_RESULT(x86)
                AC_DEFINE([X86], [1], [32Bit])
                AC_SUBST(ARCH,x86)
        ;;
esac


CONFIGURE_PART(Checking Build Environment)
AC_C_CONST
AC_FUNC_ALLOCA
#AC_C___ATTRIBUTE__

AC_PROG_CC
AC_PROG_CPP
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB
AC_PROG_LIBTOOL
AC_ISC_POSIX
AC_HEADER_STDC

AC_TYPE_SIGNAL

AC_C_INLINE
AC_CHECK_SIZEOF(int, 4)
AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(wchar_t, 4)

AC_CHECK_SIZEOF([int *])
AC_CHECK_SIZEOF([long *])
AC_CHECK_SIZEOF([wchar_t *])
AC_CHECK_SIZEOF([int_t *])
AC_CHECK_SIZEOF([int32 *])
AC_CHECK_PROG(strip, strip, strip, :)

CONFIGURE_PART(Checking Headers)
AC_CHECK_HEADERS(math.h)
AC_CHECK_HEADERS(dlfcn.h)
AC_CHECK_HEADERS(stdio.h)
AC_CHECK_HEADERS(stdlib.h)
AC_CHECK_HEADERS(stdarg.h)
AC_CHECK_HEADERS(assert.h)
AC_CHECK_HEADERS(unistd.h)
AC_CHECK_HEADERS(string.h)
AC_CHECK_HEADERS(signal.h)
AC_CHECK_HEADERS(sys/types.h)
AC_CHECK_HEADERS(sys/stat.h)
AC_CHECK_HEADERS(sys/socket.h)
AC_CHECK_HEADERS(sys/time.h)
AC_CHECK_HEADERS(sys/wait.h)
AC_CHECK_HEADERS(netinet/in.h)
AC_CHECK_HEADERS(arpa/inet.h)
AC_CHECK_HEADERS(errno.h)
AC_CHECK_HEADERS(netdb.h)
AC_CHECK_HEADERS(ctype.h)
AC_CHECK_HEADERS(time.h)
AC_CHECK_HEADERS(stdint.h)
AC_CHECK_HEADERS(sys/resource.h)
AC_CHECK_HEADERS(stddef.h)
AC_CHECK_HEADERS(limits.h)
AC_CHECK_HEADERS(strings.h)
AC_CHECK_HEADERS(sys/select.h)



CONFIGURE_PART(Checking Functions)

AC_CHECK_FUNCS(getcwd)
AC_CHECK_FUNCS(mkdir)
AC_CHECK_FUNCS(snprintf)
AC_CHECK_FUNCS(strcasecmp)
AC_CHECK_FUNCS(strcasestr)
AC_CHECK_FUNCS(strcasencmp)
AC_CHECK_FUNCS(strtok)
AC_CHECK_FUNCS(setenv)
AC_CHECK_FUNCS(unsetenv)
AC_CHECK_FUNCS(strdup)
AC_CHECK_FUNCS(strndup)
AC_CHECK_FUNCS(hstrerror)
AC_CHECK_FUNCS(snprintf)
AC_CHECK_FUNCS(stricmp)
AC_CHECK_FUNCS(strnicmp)
AC_CHECK_FUNCS(strspn)
AC_CHECK_FUNCS(strcspn)
AC_CHECK_FUNCS(strerror)
AC_CHECK_FUNCS(strsignal)

if test "x$requires_libavl" = "xyes" ; then
    AC_CHECK_LIB(ld, dlsym, [], [
        echo "Error! dlsym required."
        exit -1
        ])
fi

#CONFIGURE_PART(DEBUG)
#AC_ARG_ENABLE(debug,
#        AC_HELP_STRING([--enable-debug], [enable debug]),
#        [want_debug=$enableval],
#        [want_debug=no])


#if test "x$want_debug" != "xno" ; then
#        AC_DEFINE([WANT_DEBUG], [1], [Define if you want Debugging])
#	[want_debug=yes]
#fi

#AC_MSG_CHECKING([whether debugging is enabled])
#AC_MSG_RESULT([${want_debug}])


CONFIGURE_PART(Defining Paths)
LOG_PATH="$prefix/logs"
AC_DEFINE_UNQUOTED([LOGPATH], "${LOG_PATH}", [Path to log files])
AC_MSG_CHECKING([for log path])
AC_MSG_RESULT([${LOG_PATH}])

VAR_PATH="$prefix/var"
AC_DEFINE_UNQUOTED([VARPATH], "${VAR_PATH}", [Path to var files])
AC_MSG_CHECKING([for var path])
AC_MSG_RESULT([${VAR_PATH}])

CONF_PATH="$prefix/conf"
AC_DEFINE_UNQUOTED([CONFPATH], "${CONF_PATH}", [Path to config files])
AC_MSG_CHECKING([for config path])
AC_MSG_RESULT([${CONF_PATH}])

MODULE_PATH="$prefix/modules"
AC_DEFINE_UNQUOTED([MODULEPATH], "${MODULE_PATH}", [Path to module files])
AC_MSG_CHECKING([for module path])
AC_MSG_RESULT([${MODULE_PATH}])


CONFIGURE_PART(Defining Files)
MAINLOG_PATH="$prefix/logs/ircsp.log"
AC_DEFINE_UNQUOTED([MAINLOG], "${MAINLOG_PATH}", [Path to MAINLOG])
AC_MSG_CHECKING([for MAINLOG])
AC_MSG_RESULT([${MAINLOG_PATH}])

PIDFILE_PATH="$prefix/var/ircsp.pid"
AC_DEFINE_UNQUOTED([PIDFILE], "${PIDFILE_PATH}", [Path to PIDFILE])
AC_MSG_CHECKING([for PIDFILE])
AC_MSG_RESULT([${PIDFILE_PATH}])


CONFFILE_PATH="$prefix/conf/ircsp.conf"
AC_DEFINE_UNQUOTED([CONFFILE], "${CONFFILE_PATH}", [Path to CONFFILE])
AC_MSG_CHECKING([for CONFFILE])
AC_MSG_RESULT([${CONFFILE_PATH}])


#AC_ARG_ENABLE(ircup10,
#        AC_HELP_STRING([--enable-ircup10], [enable IRCUP10]),
#        [want_ircup10=$enableval],
#        [want_ircup10=no])

#if test "x$want_ircup10" != "xno" ; then
#        AC_DEFINE([WANT_IRCUP10], [1], [Define if you want IRCUP10])
#	protolibs="-L$prefix/lib -lircuP10"
#fi


#AC_MSG_CHECKING([whether ircup10 is enabled])
#AC_MSG_RESULT([${want_ircup10}])

#AC_ARG_ENABLE(efnet,
#        AC_HELP_STRING([--enable-efnet], [enable efnet]),
#        [want_efnet=$enableval],
#        [want_efnet=no])

#if test "x$want_efnet" != "xno" ; then
#        AC_DEFINE([WANT_EFNET], [1], [Define if you want EFNET])
#	protolibs="-L$prefix/lib -lefnet"
#fi


#AC_MSG_CHECKING([whether efnet is enabled])
#AC_MSG_RESULT([${want_efnet}])

## checking for foo linking

CONFIGURE_PART(Checking MySQL)
#AC_DEFUN([CHECK_MYSQL_LIB],
#[
AC_CHECK_PROGS(
    MYSQL_CONFIG,
    mysql_config
)

  #In case it fails to find pthread then exit configure 
  if test "x${MYSQL_CONFIG}" != xmysql_config; then
    echo "------------------------------------------"
    echo " The mysql library and header file is      "
    echo " required to build this project. Stopping "
    echo " Check 'config.log' for more information. "
    echo "------------------------------------------"
    (exit 1); exit 1;
  else
    echo "MySQL seems to be installed..."
    MYSQL_CONFIG_CFLAGS=`$MYSQL_CONFIG --cflags`
    MYSQL_CONFIG_CPPFLAGS=`$MYSQL_CONFIG --include`
    MYSQL_CONFIG_LIBS=`$MYSQL_CONFIG --libs`
    AC_SUBST([MYSQL_CONFIG_CFLAGS])
    AC_SUBST([MYSQL_CONFIG_CPPFLAGS])
    AC_SUBST([MYSQL_CONFIG_LIBS])
  fi
#])

saved_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS $protolibs"
AC_SUBST(protolibs)

CONFIGURE_PART(Writing Makefiles)
AC_OUTPUT([
Makefile
source/Makefile
logs/Makefile
data/Makefile
data/empty/Makefile
conf/Makefile
var/Makefile
var/helpfiles/Makefile
var/helpfiles/nickserv/Makefile
var/helpfiles/operserv/Makefile
var/helpfiles/uworld/Makefile
modules/Makefile
modules/protocol/Makefile
modules/protocol/undernet/Makefile
modules/protocol/undernet/source/Makefile
modules/protocol/bahamut/Makefile
modules/services/Makefile
modules/services/nickserv/Makefile
modules/services/nickserv/source/Makefile
modules/services/operserv/Makefile
modules/services/operserv/source/Makefile
modules/services/chanserv/Makefile
modules/services/chanserv/source/Makefile
modules/services/helpserv/Makefile
modules/services/helpserv/source/Makefile
modules/services/memoserv/Makefile
modules/services/memoserv/source/Makefile
])
