# configure.ac - process this file with autoconf to produce configure
#
# Copyright (C) 2017-2018 Shintaro Fujiwara 
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
[Copyright (C) 2017-2018 Shintaro Fujiwara 

This configure script is derived from configure.ac which is free software;
you can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. See the
configure.ac file for more details.])

# initialize and set version and bugreport address
AC_INIT([segatexd], [0.1.5], [shintaro.fujiwara@gmail.com])
RELEASE_MONTH="AUg 2017"
AC_SUBST(RELEASE_MONTH)
AC_CONFIG_MACRO_DIR([m4])

# display notice and initialize automake
AC_MSG_NOTICE([configuring AC_PACKAGE_TARNAME AC_PACKAGE_VERSION])
AM_INIT_AUTOMAKE([1.11.2 --warnings=all])
AC_CONFIG_SRCDIR([server/main.c])

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
DESIRED_CFLAGS="-Wall"
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

AC_ARG_WITH(segatexd-data-file,
            AS_HELP_STRING([--with-segatexd-data-file=PATH],
                           [path to segatexd data file @<:@/usr/share/segatexd/segatexd_tcp_send_data.txt@:>@]),
            [ SEGATEXD_DATA_PATH="$with_segatexd_data_file" ],
            [ SEGATEXD_DATA_PATH="/usr/share/segatexd" ])
AC_DEFINE_UNQUOTED(SEGATEXD_DATA_PATH, "$SEGATEXD_DATA_PATH", [Path to segatexd data file.])
AC_SUBST(SEGATEXD_DATA_PATH)

# where should the pidfile be written
AC_ARG_WITH(segatexd-pidfile,
            AS_HELP_STRING([--with-segatexd-pidfile=PATH],
                           [path to pidfile @<:@/var/run/segatexd/segatexd.pid@:>@]),
            [ SEGATEXD_PID_PATH="$with_segatexd_pidfile" ],
            [ SEGATEXD_PID_PATH="/var/run/segatexd" ])
AC_DEFINE_UNQUOTED(SEGATEXD_PID_PATH, "$SEGATEXD_PID_PATH", [The location of the pidfile used for checking availability of the segatexd.])
AC_SUBST(SEGATEXD_PID_PATH)

# where is the socket used for communication
AC_ARG_WITH(segatexd-socket,
            AS_HELP_STRING([--with-segatexd-socket=PATH],
                           [path to socket @<:@/var/run/segatexd/socket@:>@]),
            [ SEGATEXD_SOCKET="$with_segatexd_socket" ],
            [ SEGATEXD_SOCKET="/var/run/segatexd/socket" ])
AC_DEFINE_UNQUOTED(SEGATEXD_SOCKET, "$SEGATEXD_SOCKET", [The location of the socket used for communicating.])
AC_SUBST(SEGATEXD_SOCKET)

######## transcribed from ISC DHCP 4.3.6 code from line 517-518 ########
# see if ifaddrs.h is available
AC_CHECK_HEADERS(ifaddrs.h)
######## end transcribed from ISC DHCP 4.3.6 code from line 517-518 ########

######## transcribed from ISC DHCP 4.3.6 code from line 550-720 ########

# SIOCGLIFCONF uses some transport structures.  Trick is not all platforms
# use the same structures.  We like to use 'struct lifconf' and 'struct
# lifreq', but we'll use these other structures if they're present.  HPUX
# does not define 'struct lifnum', but does use SIOCGLIFNUM - they use an
# int value.
#
AC_MSG_CHECKING([for struct lifnum])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[ #include <sys/types.h>
  #include <sys/socket.h>
  #include <net/if.h>
]], [[ struct lifnum a;
]])],[AC_MSG_RESULT(yes)
         AC_DEFINE([ISC_PLATFORM_HAVELIFNUM], [1],
                   [Define to 1 if the system has 'struct lifnum'.])],[AC_MSG_RESULT(no)])

AC_MSG_CHECKING([for struct if_laddrconf])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[ #include <sys/types.h>
  #include <net/if6.h>
]], [[ struct if_laddrconf a;
]])],[AC_MSG_RESULT(yes)
         AC_DEFINE([ISC_PLATFORM_HAVEIF_LADDRCONF], [1],
                   [Define to 1 if the system has 'struct if_laddrconf'.])],[AC_MSG_RESULT(no)])

AC_MSG_CHECKING([for struct if_laddrreq])
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
 #include <net/if6.h>
]], [[ struct if_laddrreq a;
]])],[AC_MSG_RESULT(yes)
         AC_DEFINE([ISC_PLATFORM_HAVEIF_LADDRREQ], [1],
                   [Define to 1 if the system has 'struct if_laddrreq'.])],[AC_MSG_RESULT(no)])

#
# check for GCC noreturn attribute
#
AC_MSG_CHECKING(for GCC noreturn attribute)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[void foo() __attribute__((noreturn));]])],[AC_MSG_RESULT(yes)
         AC_DEFINE([ISC_DHCP_NORETURN], [__attribute__((noreturn))],
                   [Define to the string for a noreturn attribute.])],[AC_MSG_RESULT(no)
         AC_DEFINE([ISC_DHCP_NORETURN], [],
                   [Define to the string for a noreturn attribute.])])

# Look for optional headers.
AC_CHECK_HEADERS(sys/socket.h net/if_dl.h net/if6.h regex.h)

# Solaris needs some libraries for functions
AC_SEARCH_LIBS(socket, [socket])
AC_SEARCH_LIBS(inet_ntoa, [nsl])

AC_SEARCH_LIBS(inet_aton, [socket nsl], ,
        AC_DEFINE([NEED_INET_ATON], [1],
                  [Define to 1 if the inet_aton() function is missing.]))

# Check for a standalone regex library.
AC_SEARCH_LIBS(regcomp, [regex])

AC_CHECK_FUNCS(strlcat)

# For HP/UX we need -lipv6 for if_nametoindex, perhaps others.
AC_SEARCH_LIBS(if_nametoindex, [ipv6])

# check for /dev/random (declares HAVE_DEV_RANDOM)
AC_MSG_CHECKING(for random device)
AC_ARG_WITH(randomdev,
        AS_HELP_STRING([--with-randomdev=PATH],[Path for random device
                       (default is /dev/random)]),
        use_randomdev="$withval", use_randomdev="unspec")
if test "$use_randomdev" = "unspec"; then
        if test "$cross_compiling" = "yes"; then
                AC_MSG_RESULT(unspecified)
                AC_MSG_ERROR([ need --with-randomdev=PATH or --with-randomdev=no])
        fi
        use_randomdev="/dev/random"
elif test "$use_randomdev" = "yes"; then
        use_randomdev="/dev/random"
fi
if test "$use_randomdev" = "no"; then
        AC_MSG_RESULT(disabled)
        BINDCONFIG="$BINDCONFIG --with-randomdev=no"
else
        if test "$cross_compiling" = "yes"; then
                AC_MSG_RESULT($use_randomdev (unchecked))
        else
                AC_MSG_RESULT($use_randomdev)
                AC_CHECK_FILE($use_randomdev,
                    AC_DEFINE([HAVE_DEV_RANDOM], [1],
                        [Define to 1 if you have the /dev/random or other configured file.]),
                    AC_MSG_ERROR(cannot find $use_randomdev))
        fi
        BINDCONFIG="$BINDCONFIG --with-randomdev=$use_randomdev"
fi

# see if there is a "sa_len" field in our interface information structure
AC_CHECK_MEMBER(struct sockaddr.sa_len,
        AC_DEFINE([HAVE_SA_LEN], [],
                  [Define to 1 if the sockaddr structure has a length field.]),
        ,
        [#include <sys/socket.h>])

# figure out pointer size
SAVE_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -I$srcdir"
AC_CHECK_SIZEOF(struct iaddr *, , [
#include "includes/inet.h"
#include <stdio.h>
])
CFLAGS="$SAVE_CFLAGS"

# Solaris does not have the msg_control or msg_controlen members
# in the msghdr structure unless you define:
#
#   _XOPEN_SOURCE, _XOPEN_SOURCE_EXTENDED, and __EXTENSIONS__
#
# See the "standards" man page for details.
#
# We check for the msg_control member, and if it is not found, we check
# again with the appropriate defines added to the CFLAGS. (In order to
# do this we have to remove the check from the cache, which is what the
# "unset" is for.)
AC_CHECK_MEMBER(struct msghdr.msg_control,,
        [CFLAGS="$CFLAGS -D_XOPEN_SOURCE -D_XOPEN_SOURCE_EXTENDED=1"
         CFLAGS="$CFLAGS -D__EXTENSIONS__"
         unset ac_cv_member_struct_msghdr_msg_control
         AC_CHECK_MEMBER(struct msghdr.msg_control,,
                [AC_MSG_ERROR([Missing msg_control member in
                               msg_control structure.])],
                [
#include <sys/types.h>
#include <sys/socket.h>
                ])
        ],
        [
#include <sys/types.h>
#include <sys/socket.h>
        ])

AC_CHECK_MEMBER(struct tpacket_auxdata.tp_vlan_tci,
    [AC_DEFINE([VLAN_TCI_PRESENT], [1], [tpacket_auxdata.tp_vlan_tci present])]
    ,, [#include <linux/if_packet.h>])

BINDDIR=
BINDSRCDIR=
AC_ARG_WITH(libbind,
        AS_HELP_STRING([--with-libbind=PATH],[bind includes and libraries are in PATH
                        (default is ./bind)]),
        use_libbind="$withval", use_libbind="no")
case "$use_libbind" in
yes)
        BINDDIR="\${top_srcdir}/bind"
        BINDSRCDIR="\${top_srcdir}/bind"
        ;;
no)
        BINDDIR="\${top_srcdir}/bind"
        BINDSRCDIR="\${top_srcdir}/bind"
        ;;
*)
        BINDDIR="$use_libbind"
        if test ! -d "$srcdir/bind"; then
                # no bind directory, create it with a fake Makefile.in
                # (AC_CONFIG_FILES and top Makefile refer to it so
                # it must exits)
                mkdir $srcdir/bind
                cat > $srcdir/bind/Makefile.in << EOF
# placeholder
all check clean distclean distdir install uninstall:

EOF
        fi
        ;;
esac
AC_SUBST(BINDDIR)
AC_SUBST(BINDSRCDIR)

######## transcribed from ISC DHCP 4.3.6 code from line 550-720 ########

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
#AC_OUTPUT(Makefile server/Makefile)
AC_CONFIG_FILES([
  Makefile
  server/Makefile
  agent/Makefile
])
AC_OUTPUT
