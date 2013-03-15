AC_INIT([hud],
        [13.10.0],
        [http://bugs.launchpad.net/hud],
        [hud],
        [http://launchpad.net/hud])

AC_COPYRIGHT([Copyright 2010-2012 Canonical])

AC_PREREQ(2.53)

AM_CONFIG_HEADER(config.h)
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.11 -Wall foreign subdir-objects])
AM_MAINTAINER_MODE([enable])

AM_SILENT_RULES([yes])

# Check for programs
AC_PROG_CC
AM_PROG_CC_C_O
AM_PROG_VALAC([])

# Initialize libtool
LT_PREREQ([2.2.6])
LT_INIT

AC_PATH_PROG([GLIB_MKENUMS], [glib-mkenums])
AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal])

PKG_PROG_PKG_CONFIG

AC_ARG_ENABLE([deprecations],
  [AS_HELP_STRING([--enable-deprecations],
    [allow deprecated API usage @<:@default=yes@:>@])],
  [],
  [enable_deprecations=yes])
AS_IF([test "x$enable_deprecations" = xno],
  [CFLAGS="$CFLAGS -DG_DISABLE_DEPRECATED -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGTK_DISABLE_DEPRECATED -DGSEAL_ENABLE -DGTK_DISABLE_SINGLE_INCLUDES"]
)

###########################
# GTK Doc
###########################

GTK_DOC_CHECK([1.9], [--flavour=no-tmpl])
AC_CONFIG_MACRO_DIR(m4)

###########################
# GLib GSettings
###########################

GLIB_GSETTINGS

###########################
# GObject Introspection
###########################

GOBJECT_INTROSPECTION_CHECK([0.9.5])

###########################
# Vala API Generation
###########################

AC_ARG_ENABLE([vala],
    AC_HELP_STRING([--disable-vala], [Disable vala]),
    [enable_vala=$enableval], [enable_vala=auto])

AS_IF([test "x$enable_vala" != "xno"],[
    AM_COND_IF([HAVE_INTROSPECTION],,[
        AC_MSG_ERROR([Vala bindings require introspection support, please --enable-introspection])
    ])  
AC_PATH_PROG([VALA_API_GEN], [vapigen])
])
AM_CONDITIONAL([HAVE_VALA], [test -n "$VALA_API_GEN"])

###########################
# Dependencies
###########################

GLIB_REQUIRED_VERSION=2.34.0
DBUSMENU_GLIB_REQUIRED_VERSION=0.4.0
SQLITE_REQUIRED_VERSION=0.0

PKG_CHECK_MODULES([HUD],[
		glib-2.0 >= $GLIB_REQUIRED_VERSION
		gio-unix-2.0 >= $GLIB_REQUIRED_VERSION
		dbusmenu-glib-0.4 >= $DBUSMENU_GLIB_REQUIRED_VERSION
		sqlite3 >= $SQLITE_REQUIRED_VERSION
		dee-1.0
		pocketsphinx
])

GTK_REQUIRED_VERSION=3.0
PKG_CHECK_MODULES(gtk,[
		gtk+-3.0 >= $GTK_REQUIRED_VERSION
])

PKG_CHECK_MODULES([DBUSMENU_JSON], [dbusmenu-jsonloader-0.4])

###########################
# BAMF Dependency
###########################

BAMF_REQUIRED_VERSION=0.2.53

AC_ARG_ENABLE([bamf],
    AC_HELP_STRING([--disable-bamf], [Disable bamf]),
    [enable_bamf=$enableval], [enable_bamf=auto])

AS_IF([test "x$enable_bamf" != "xno"],[
PKG_CHECK_MODULES([BAMF],[
		libbamf3 
])
AC_DEFINE([HAVE_BAMF], [1], [Defined if we are building with BAMF])
])

###########################
# Hybris Dependency
###########################

AC_ARG_ENABLE([hybris],
    AC_HELP_STRING([--disable-hybris], [Disable hybris]),
    [enable_hybris=$enableval], [enable_hybris=auto])

AS_IF([test "x$enable_hybris" != "xno"],[
AC_DEFINE([HAVE_HYBRIS], [1], [Defined if we are building with libhybris])
HYBRIS_CFLAGS="-I/usr/include/hybris"
HYBRIS_LIBS="-lubuntu_application_api"
AC_SUBST([HYBRIS_CFLAGS])
AC_SUBST([HYBRIS_LIBS])
])

###########################
# Sanity Check
###########################

AS_IF([test "x$enable_hybris" == "x$enable_bamf"],[
AC_MSG_ERROR([Invalid to use hybris and bamf together or neither of them])
])

###########################
# CLI Dependencies
###########################

AC_CHECK_HEADER("readline/readline.h",
	[has_readline_h="yes"])
AC_CHECK_HEADER("readline/history.h",
	[has_history_h="yes"])
AC_CHECK_HEADER("curses.h",
	[has_curses_h="yes"])

AM_CONDITIONAL([BUILD_CLI], [test "x$has_readline_h" == "xyes" && test "x$has_history_h" == "xyes" && test "x$has_curses_h" == "xyes"])

###########################
# Test Dependencies
###########################

DBUSMENU_JSONLOADER_REQUIRED_VERSION=0.3.3
LIBAPPINDICATOR_REQUIRED_VERSION=0.0.0

AC_ARG_ENABLE([tests],
	AC_HELP_STRING([--disable-tests], [Disable test scripts and tools]),,
	[enable_tests=auto])

if test x"$enable_tests" != x"no" ; then
   PKG_CHECK_MODULES(INDICATORTEST,
                     dbusmenu-jsonloader-0.4 >= $DBUSMENU_JSONLOADER_REQUIRED_VERSION
                     appindicator3-0.1 >= $LIBAPPINDICATOR_REQUIRED_VERSION,
                     [have_dbusmenu_jsonloader=yes],
                     [have_dbusmenu_jsonloader=no]
                    )
   if test x${enable_tests} = xyes && test x${have_dbusmenu_jsonloader} = xno; then
     AC_MSG_ERROR([Test scripts and tools configured but dbusmenu-jsonloader not found])
   fi
else
   have_dbusmenu-jsonloader=no
fi
AM_CONDITIONAL(BUILD_TESTS, test x${have_dbusmenu_jsonloader} = xyes)

AC_SUBST(INDICATORTEST_CFLAGS)
AC_SUBST(INDICATORTEST_LIBS)

PKG_CHECK_MODULES(DBUSTEST, dbustest-1)
AC_SUBST(DBUSTEST_CFLAGS)
AC_SUBST(DBUSTEST_LIBS)

###########################
# gcov coverage reporting
###########################

m4_include([m4/gcov.m4])
AC_TDD_GCOV
AM_CONDITIONAL([HAVE_GCOV], [test "x$ac_cv_check_gcov" = xyes])
AM_CONDITIONAL([HAVE_LCOV], [test "x$ac_cv_check_lcov" = xyes])
AM_CONDITIONAL([HAVE_GCOVR], [test "x$ac_cv_check_gcovr" = xyes])
AC_SUBST(COVERAGE_CFLAGS)
AC_SUBST(COVERAGE_CXXFLAGS)
AC_SUBST(COVERAGE_LDFLAGS)

###########################
# XSLT Processor
###########################

AC_PATH_PROG([XSLT_PROC], [xsltproc])

###########################
# Check to see if we're local
###########################

with_localinstall="no"
AC_ARG_ENABLE(localinstall, AS_HELP_STRING([--enable-localinstall], [install all of the files localy instead of system directories (for distcheck)]), with_localinstall=$enableval, with_localinstall=no)

###########################
# Indicator Info
###########################

if test "x$with_localinstall" = "xyes"; then
	INDICATORDIR="${libdir}/indicators/3/"
	INDICATORICONSDIR="${datadir}/libindicate/icons/"
elif test "x$with_gtk" = x2; then
	INDICATORDIR=`$PKG_CONFIG --variable=indicatordir indicator-0.4`
	INDICATORICONSDIR=`$PKG_CONFIG --variable=iconsdir indicator-0.4`
else
	INDICATORDIR=`$PKG_CONFIG --variable=indicatordir indicator3-0.4`
	INDICATORICONSDIR=`$PKG_CONFIG --variable=iconsdir indicator3-0.4`
fi
AC_SUBST(INDICATORDIR)
AC_SUBST(INDICATORICONSDIR)

###########################
# DBus Service Info
###########################

if test "x$with_localinstall" = "xyes"; then
	DBUSSERVICEDIR="${datadir}/dbus-1/services/"
else
	DBUSSERVICEDIR=`$PKG_CONFIG --variable=session_bus_services_dir dbus-1`
fi
AC_SUBST(DBUSSERVICEDIR)

if test "x$with_localinstall" = "xyes"; then
	DBUSIFACEDIR="${datadir}/dbus-1/interfaces/"
else
	DBUSIFACEDIR=`$PKG_CONFIG --variable=interfaces_dir dbus-1`
fi
AC_SUBST(DBUSIFACEDIR)

##############################
# Custom Junk
##############################

AC_DEFUN([AC_DEFINE_PATH], [
	test "x$prefix" = xNONE && prefix="$ac_default_prefix"
	test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'
	ac_define_path=`eval echo [$]$2`
	ac_define_path=`eval echo [$]ac_define_path`
	$1="$ac_define_path"
	AC_SUBST($1)
	ifelse($3, ,
		AC_DEFINE_UNQUOTED($1, "$ac_define_path"),
		AC_DEFINE_UNQUOTED($1, "$ac_define_path", $3))
])

###########################
# Internationalization
###########################

GETTEXT_PACKAGE=hud

IT_PROG_INTLTOOL([0.50.2])

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.17])

AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Define to the gettext package name.])
AC_DEFINE_PATH(GNOMELOCALEDIR, "${datadir}/locale", [locale directory])

###########################
# Files
###########################

AC_OUTPUT([
Makefile
src/Makefile
libhud/Makefile
libhud/tests/Makefile
libhud/demo/Makefile
libhud-gtk/Makefile
libhud-client/Makefile
data/Makefile
po/Makefile.in
tools-vala/Makefile
tools/Makefile
tools/hudkeywords/Makefile
tools/hudkeywords/bin/Makefile
tools/hudkeywords/hudkeywords/Makefile
tests/Makefile
docs/Makefile
docs/man/Makefile
docs/libhud-client/Makefile
])

###########################
# Results
###########################

AC_MSG_NOTICE([

App Menu Indicator Configuration:

	Prefix:        $prefix
	Indicator Dir: $INDICATORDIR
	gcov:          $use_gcov
	Local Install: $with_localinstall
	Test tools:    $have_dbusmenu_jsonloader
])
