
AC_INIT([mondemand], [4.4.3], [anthonym@alumni.caltech.edu])

dnl -- we want a header to include in our source files with configure
dnl dnl    info
AC_CONFIG_HEADERS(src/config.h)

AM_INIT_AUTOMAKE([foreign])

dnl --- Variables I use later on --

SHORT_DESC="Emit log, stats, annotation, trace and performance trace messages over LWES from C or the command line."
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
PACKAGE_DEPS=`echo "lwes-1"`

AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(RELEASE_NUMBER)
AC_SUBST(SHORT_DESC)

AC_SUBST(MAJOR_VERSION_UNDERLINE)
AC_SUBST(PACKAGE_UNDERLINE)
AC_SUBST(PACKAGEPACKED)
AC_SUBST(VERSION_UNDERLINE)

AC_SUBST(PACKAGE_DEPS)

dnl -- set maintainer mode
AM_MAINTAINER_MODE
AC_SUBST(USE_MAINTAINER_MODE)

dnl -- make sure we have a C compiler
AC_PROG_CC

dnl dnl -- if we are building a library we need libtool
AM_PROG_LIBTOOL

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

PKG_CHECK_MODULES([LWES], [$PACKAGE_DEPS])
AC_SUBST(LWES_CFLAGS)
AC_SUBST(LWES_LIBS)

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

dnl - AC_MSG_CHECKING(for ISO C99 varargs macros in C++)
dnl - if test "$CXX" = ""; then
dnl - dnl No C++ compiler
dnl - g_have_iso_cxx_varargs=no
dnl - else
dnl -   AC_LANG_CPLUSPLUS
dnl -   AC_TRY_COMPILE([],[
dnl - int a(int p1, int p2, int p3);
dnl - #define call_a(...) a(1,__VA_ARGS__) call_a(2,3);
dnl - ],g_have_iso_cxx_varargs=yes,g_have_iso_cxx_varargs=no)
dnl -   AC_LANG_C
dnl - fi
dnl - AC_MSG_RESULT($g_have_iso_cxx_varargs)
dnl - if test "x$g_have_iso_cxx_varargs" = "xyes" ; then
dnl -   dnl instead of adding these to the config.h, we want to substitute them
dnl -   dnl directly in the header we ship out with the library
dnl -   HAVE_ISO_CXX_VARARGS="#define HAVE_ISO_VARARGS"
dnl -   AC_SUBST(HAVE_ISO_CXX_VARARGS)
dnl - fi

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

dnl - AC_MSG_CHECKING(for GNUC varargs macros in C++)
dnl - if test "$CXX" = ""; then
dnl - dnl No C++ compiler
dnl - g_have_iso_cxx_varargs=no
dnl - else
dnl -   AC_LANG_CPLUSPLUS
dnl -   AC_TRY_COMPILE([],[
dnl - int a(int p1, int p2, int p3);
dnl - #define call_a(params...) a(1,params)
dnl - call_a(2,3);
dnl - ],g_have_gnuc_cxx_varargs=yes,g_have_gnuc_cxx_varargs=no)
dnl -   AC_LANG_C
dnl - fi
dnl - AC_MSG_RESULT($g_have_gnuc_cxx_varargs)
dnl - if test "x$g_have_gnuc_cxx_varargs" = "xyes" ; then
dnl -   dnl instead of adding these to the config.h, we want to substitute them
dnl -   dnl directly in the header we ship out with the library
dnl -   HAVE_GNUC_CXX_VARARGS="#define HAVE_GNUC_VARARGS"
dnl -   AC_SUBST(HAVE_GNUC_CXX_VARARGS)
dnl - fi

dnl # --- Coverage hooks ---

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
