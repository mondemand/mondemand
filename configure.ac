dnl Process this file with autoconf to produce a configure script.

AC_INIT([mondemand], [1.0.1], [mondemand-devel@lists.sourceforge.net])
AM_INIT_AUTOMAKE

dnl Determine the host type for the host specific inclusion below
AC_CANONICAL_HOST

dnl --- Variables I use later on --

SHORT_DESC="MonDemand is a high-performance distributed monitoring and instrumentation library.  By default it uses the Light Weight Event System in order to announce events to the network."
MAJOR_VERSION=`echo "[$]PACKAGE_VERSION" |
               perl -ne 'm%^(\d+)\.% && print "[$]1"'`
MINOR_VERSION=`echo "[$]PACKAGE_VERSION" |
               perl -ne 'm%^\d+\.(\d+)% && print "[$]1"'`
RELEASE_NUMBER=`echo "[$]PACKAGE_VERSION" |
                perl -ne 'm%^\d+\.\d+\.(\d+)% && print "[$]1"'`
MAJOR_VERSION_UNDERLINE=`echo "[$]MAJOR_VERSION" | perl -pe 'chomp; s/\W/_/g;'`
PACKAGE_UNDERLINE=`echo "[$]PACKAGE_NAME" | perl -pe 'chomp; s/\W/_/g;'`
PACKAGEPACKED=`echo "[$]PACKAGE_NAME" | perl -pe 'chomp; s/\W//g;'`
VERSION_UNDERLINE=`echo "[$]PACKAGE_VERSION" | perl -pe 'chomp; s/\W/_/g;'`

AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(RELEASE_NUMBER)
AC_SUBST(SHORT_DESC)

AC_SUBST(MAJOR_VERSION_UNDERLINE)
AC_SUBST(PACKAGE_UNDERLINE)
AC_SUBST(PACKAGEPACKED)
AC_SUBST(VERSION_UNDERLINE)

dnl -- set maintainer mode
AM_MAINTAINER_MODE
AC_SUBST(USE_MAINTAINER_MODE)

dnl -- we want a header to include in our source files with configure
dnl    info
AM_CONFIG_HEADER(src/config.h)

dnl -- make sure we have a C compiler
AC_PROG_CC

dnl -- if we are building a library we need libtool
AM_PROG_LIBTOOL

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h limits.h sys/time.h unistd.h getopt.h)
AC_CHECK_HEADER(valgrind/valgrind.h,
                AC_DEFINE([HAVE_VALGRIND_HEADER],
                          [1],
                          [Define to 1 if the <valgrind/valgrind.h> header is on the system]))

dnl check for LWES
PKG_CHECK_MODULES([LWES], [lwes-0 >= 0.23.1])
AC_SUBST(LWES_CFLAGS)
AC_SUBST(LWES_LIBS)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_CHECK_TYPES([long long],[],[exit 1])
AC_HEADER_TIME
AC_STRUCT_TM

dnl Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(gettimeofday socket strerror)

dnl These are mostly for solaris
AC_CHECK_LIB(socket,main)
AC_CHECK_LIB(nsl,main)
AC_CHECK_LIB(xnet,main)
AC_CHECK_LIB(resolv,main)

dnl Figure out what kind of varargs this platform supports
# check for flavours of varargs macros
AC_MSG_CHECKING(for ISO C99 varargs macros in C)
AC_TRY_COMPILE([],[
int a(int p1, int p2, int p3);
#define call_a(...) a(1,__VA_ARGS__)
call_a(2,3);
],g_have_iso_c_varargs=yes,g_have_iso_c_varargs=no)
AC_MSG_RESULT($g_have_iso_c_varargs)
if test "x$g_have_iso_c_varargs" = "xyes" ; then
  dnl instead of adding these to the config.h, we want to substitute them
  dnl directly in the header we ship out with the library
  HAVE_ISO_C_VARARGS="#define HAVE_ISO_VARARGS"
  AC_SUBST(HAVE_ISO_C_VARARGS)
fi

AC_MSG_CHECKING(for ISO C99 varargs macros in C++)
if test "$CXX" = ""; then
dnl No C++ compiler
g_have_iso_cxx_varargs=no
else
  AC_LANG_CPLUSPLUS
  AC_TRY_COMPILE([],[
int a(int p1, int p2, int p3);
#define call_a(...) a(1,__VA_ARGS__) call_a(2,3);
],g_have_iso_cxx_varargs=yes,g_have_iso_cxx_varargs=no)
  AC_LANG_C
fi
AC_MSG_RESULT($g_have_iso_cxx_varargs)
if test "x$g_have_iso_cxx_varargs" = "xyes" ; then
  dnl instead of adding these to the config.h, we want to substitute them
  dnl directly in the header we ship out with the library
  HAVE_ISO_CXX_VARARGS="#define HAVE_ISO_VARARGS"
  AC_SUBST(HAVE_ISO_CXX_VARARGS)
fi

AC_MSG_CHECKING(for GNUC varargs macros)
AC_TRY_COMPILE([],[
int a(int p1, int p2, int p3);
#define call_a(params...) a(1,params)
call_a(2,3);
],g_have_gnuc_varargs=yes,g_have_gnuc_varargs=no)
AC_MSG_RESULT($g_have_gnuc_varargs)
if test "x$g_have_gnuc_varargs" = "xyes" ; then
  dnl instead of adding these to the config.h, we want to substitute them
  dnl directly in the header we ship out with the library
  HAVE_GNUC_C_VARARGS="#define HAVE_GNUC_VARARGS"
  AC_SUBST(HAVE_GNUC_C_VARARGS)
fi

AC_MSG_CHECKING(for GNUC varargs macros in C++)
if test "$CXX" = ""; then
dnl No C++ compiler
g_have_iso_cxx_varargs=no
else
  AC_LANG_CPLUSPLUS
  AC_TRY_COMPILE([],[
int a(int p1, int p2, int p3);
#define call_a(params...) a(1,params)
call_a(2,3);
],g_have_gnuc_cxx_varargs=yes,g_have_gnuc_cxx_varargs=no)
  AC_LANG_C
fi
AC_MSG_RESULT($g_have_gnuc_cxx_varargs)
if test "x$g_have_gnuc_cxx_varargs" = "xyes" ; then
  dnl instead of adding these to the config.h, we want to substitute them
  dnl directly in the header we ship out with the library
  HAVE_GNUC_CXX_VARARGS="#define HAVE_GNUC_VARARGS"
  AC_SUBST(HAVE_GNUC_CXX_VARARGS)
fi


# --- Coverage hooks ---

AC_ARG_ENABLE(coverage,
              [  --enable-coverage      turn on -fprofile-arcs -ftest-coverage],              [case "${enableval}" in
                yes) ENABLE_COVERAGE=1 ;;
                no) ENABLE_COVERAGE=0 ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --enable-cov) ;;
              esac],
              [ENABLE_COVERAGE=2])

if test "x[$]ENABLE_COVERAGE" = "x1"; then
  AC_MSG_WARN(enable coverage)
  CFLAGS="`echo \"[$]CFLAGS\" | perl -pe 's/-O\d+//g;'` -fprofile-arcs -ftest-coverage"
  CXXFLAGS="`echo \"[$]CXXFLAGS\" | perl -pe 's/-O\d+//g;'` -fprofile-arcs -ftest-coverage"
  GCOV_LTFLAGS="-XCClinker -fprofile-arcs -XCClinker -ftest-coverage"
fi

AC_SUBST(ENABLE_COVERAGE)
AC_SUBST(GCOV_LTFLAGS)

# --- Compiler warnings ---
#
# for developer use, enable lots of compile warnings,
# but don't require this generally, because some system's
# header files (BSD) can't handle it
#
# NB: must add -Werror after AC_PROG_CC, etc., so do this last

AC_ARG_ENABLE(hardcore,
              [  --disable-hardcore      turn off -W -Wall -Werror],
              [case "${enableval}" in
                yes) ENABLE_HARDCORE=1 ;;
                no) ENABLE_HARDCORE=0 ;;
                *) AC_MSG_ERROR(bad value ${enableval} for --disable-hardcore) ;;              esac],
              [ENABLE_HARDCORE=1])

if test "x[$]ENABLE_HARDCORE" = "x1"; then
  AC_MSG_WARN(enable hardcore compile warnings)
  if test "x$CXX" = "x"; then
    dnl - only valid for C with newer gcc's
    CPPFLAGS="[$]CPPFLAGS -Wmissing-prototypes"
  fi
  CPPFLAGS="[$]CPPFLAGS -Werror -W -Wall -Wpointer-arith -Wcast-align -Wwrite-strings"
fi


AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/mondemandlib.h
                 tests/Makefile
                 doxygen.config
                 pkgconfig-template.pc])
AC_CONFIG_FILES([tests/test-wrapper.sh],
                [chmod +x tests/test-wrapper.sh])

AC_OUTPUT
