AC_PREREQ([2.69])
AC_INIT([segatex-ng], [0.1.0], [shintaro.fujiwara@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_SRCDIR([src/main.c])
# check the name of the configuration file
AC_ARG_WITH(segatex-ng-conf-file,
            AS_HELP_STRING([--with-segatex-ng-conf-file=PATH],
                           [path to segatexd configuration file @<:@/etc/segatex-ng.conf@:>@]),
            [ SEGATEX_NG_CONF_PATH="$with_segatex_ng_conf_file" ],
            [ SEGATEX_NG_CONF_PATH="/etc/segatex-ng.conf" ])
AC_DEFINE_UNQUOTED(SEGATEX_NG_CONF_PATH, "$SEGATEX_NG_CONF_PATH", [Path to segatexd configuration file.])
AC_SUBST(SEGATEX_NG_CONF_PATH)

AC_PROG_CC
AC_OUTPUT(Makefile src/Makefile)
