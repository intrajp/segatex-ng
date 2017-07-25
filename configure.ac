# configure.ac - process this file with autoconf to produce configure
#
# Copyright (C) 2017 Shintaro Fujiwara 
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301 USA

AC_PREREQ([2.69])
AC_COPYRIGHT(
[Copyright (C) 2017 Shintaro Fujiwara 

This configure script is derived from configure.ac which is free software;
you can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. See the
configure.ac file for more details.])

# initialize and set version and bugreport address
AC_INIT([segatexd], [0.1.3], [shintaro.fujiwara@gmail.com])
RELEASE_MONTH="Jul 2017"
AC_SUBST(RELEASE_MONTH)
AC_CONFIG_MACRO_DIR([m4])

# display notice and initialize automake
AC_MSG_NOTICE([configuring AC_PACKAGE_TARNAME AC_PACKAGE_VERSION])
AM_INIT_AUTOMAKE([1.11.2 --warnings=all])
AC_CONFIG_SRCDIR([src/main.c])

# create a config.h file (Automake will add -DHAVE_CONFIG_H)
#AC_CONFIG_HEADERS([config.h])

# check for programs
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_RANLIB
AM_PROG_CC_C_O
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_LN_S
AM_PROG_AR

# check for debugging options
AC_ARG_ENABLE(debug,
              AS_HELP_STRING([--enable-debug],
                             [enable extensive debugging and logging]),
              [if test "x$enableval" != "xno" ; then CFLAGS="-g -DDEBUG $CFLAGS" ; fi])

# check for extra compiler warnings
DESIRED_CFLAGS=""
AC_ARG_ENABLE(warnings,
              AS_HELP_STRING([--enable-warnings],
                             [enable extra compiler warnings (gcc)]),
              [if test "x$enableval" != "no"
               then
                 CFLAGS="$CFLAGS -pedantic -Wall -Wshadow -Wpointer-arith -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wnested-externs -Waggregate-retur     n -Wmissing-declarations -Wunused -Wformat=2 -Wswitch-default -Wswitch-enum -Wfloat-equal -Wbad-function-cast -Wredundant-decls"
                 DESIRED_CFLAGS="$DESIRED_CFLAGS -Wextra -Wdeclaration-after-statement -Werror-implicit-function-declaration -Werror=implicit"
               fi])
test_gcc_flag() {
    AC_LANG_CONFTEST([AC_LANG_PROGRAM()])
    $CC -c conftest.c $CFLAGS $@ > /dev/null 2> /dev/null
    ret=$?
    rm -f conftest.o
    return $ret
}
for flag in $DESIRED_CFLAGS
do
    AC_MSG_CHECKING([whether $CC accepts $flag])
    if test_gcc_flag $flag
    then
        CFLAGS="$CFLAGS $flag"
        AC_MSG_RESULT([yes])
    else
        AC_MSG_RESULT([no])
    fi
done

# checks for availability of header files
AC_CHECK_HEADERS([ctype.h strings.h pthread.h pthread_np.h fcntl.h limits.h])
AC_CHECK_HEADERS([netinet/ether.h arpa/inet.h netinet/in.h])
AC_CHECK_HEADERS([sys/socket.h sys/un.h sys/ucred.h ucred.h sys/param.h sys/time.h])
AC_CHECK_HEADERS([getopt.h syslog.h stddef.h])

# other general checks
AC_C_INLINE
AC_C_CONST

# checks for availability of common functions
AC_CHECK_FUNCS([sigaction snprintf])
AC_SEARCH_LIBS(socket, socket)
AC_CHECK_FUNCS([strcasecmp strncasecmp strchr strcspn strspn strtol strtoul strtoull strndup])
AC_CHECK_FUNCS([malloc realloc atexit])
AC_FUNC_FORK
AC_CHECK_FUNCS(__assert_fail)
AC_SEARCH_LIBS(clock_gettime, rt)
AC_CHECK_FUNCS([setusershell getusershell endusershell getgrouplist])
AC_CHECK_DECLS([setusershell, getusershell, endusershell])

# check the name of the configuration file
AC_ARG_WITH(segatexd-conf-file,
            AS_HELP_STRING([--with-segatexd-conf-file=PATH],
                           [path to segatexd configuration file @<:@/etc/segatexd.conf@:>@]),
            [ SEGATEXD_CONF_PATH="$with_segatexd_conf_file" ],
            [ SEGATEXD_CONF_PATH="/etc/segatexd.conf" ])
AC_DEFINE_UNQUOTED(SEGATEXD_CONF_PATH, "$SEGATEXD_CONF_PATH", [Path to segatexd configuration file.])
AC_SUBST(SEGATEXD_CONF_PATH)

# where should the pidfile be written
AC_ARG_WITH(segatexd-pidfile,
            AS_HELP_STRING([--with-segatexd-pidfile=PATH],
                           [path to pidfile @<:@/var/run/segatexd/segatexd.pid@:>@]),
            [ SEGATEXD_PIDFILE="$with_segatexd_pidfile" ],
            [ SEGATEXD_PIDFILE="/var/run/segatexd/segatexd.pid" ])
AC_DEFINE_UNQUOTED(SEGATEXD_PIDFILE, "$SEGATEXD_PIDFILE", [The location of the pidfile used for checking availability of the segatexd.])
AC_SUBST(SEGATEXD_PIDFILE)

# where is the socket used for communication
AC_ARG_WITH(segatexd-socket,
            AS_HELP_STRING([--with-segatexd-socket=PATH],
                           [path to socket @<:@/var/run/segatexd/socket@:>@]),
            [ SEGATEXD_SOCKET="$with_segatexd_socket" ],
            [ SEGATEXD_SOCKET="/var/run/segatexd/socket" ])
AC_DEFINE_UNQUOTED(SEGATEXD_SOCKET, "$SEGATEXD_SOCKET", [The location of the socket used for communicating.])
AC_SUBST(SEGATEXD_SOCKET)

## check threading stuff
#AX_PTHREAD(, AC_MSG_ERROR([no support for pthreads]))
#pthread_save_CFLAGS="$CFLAGS"
#pthread_save_LIBS="$LIBS"
#CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
#LIBS="$LIBS $PTHREAD_LIBS"
#AC_CHECK_FUNCS([pthread_mutex_lock pthread_join pthread_timedjoin_np pthread_atfork])
#CFLAGS="$pthread_save_CFLAGS"
#LIBS="$pthread_save_LIBS"

AC_PROG_CC
AC_OUTPUT(Makefile src/Makefile)
