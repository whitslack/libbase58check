m4_define([version_major], [1])
m4_define([version_minor], [1])

AC_INIT([libbase58check], m4_join([.], [version_major], [version_minor]))
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIRS([build-aux/m4])
AM_INIT_AUTOMAKE([foreign subdir-objects])
LT_INIT([disable-static])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_SED
PKG_INSTALLDIR

AC_ARG_ENABLE([assertions],
	[AS_HELP_STRING([--enable-assertions], [check assertions at runtime [default=no]])],
	[enable_assertions=$enableval],
	[enable_assertions=no])
AM_CONDITIONAL([NDEBUG], [test x"$enable_assertions" = xno])

AC_ARG_ENABLE([tests],
	[AS_HELP_STRING([--enable-tests], [build unit tests [default=yes]])],
	[enable_tests=$enableval],
	[enable_tests=yes])
AM_CONDITIONAL([BUILD_TESTS], [test x"$enable_tests" = xyes])

PKG_CHECK_MODULES([GMP], [gmp])
PKG_CHECK_MODULES([OPENSSL], [libcrypto])

AM_COND_IF([BUILD_TESTS], [
	AX_CXX_COMPILE_STDCXX([20], [], [optional])
	AX_CXX_COMPILE_STDCXX([17], [], [optional])
	AX_CXX_COMPILE_STDCXX([11], [], [optional])
])

DX_DOXYGEN_FEATURE([ON])
DX_HTML_FEATURE([OFF])
DX_MAN_FEATURE([ON])
DX_PDF_FEATURE([OFF])
DX_PS_FEATURE([OFF])
DX_INIT_DOXYGEN([libbase58check])
AM_CONDITIONAL([BUILD_MANPAGES], [DX_TEST_FEATURE([man])])

AC_CONFIG_FILES([Makefile libbase58check.pc])
AC_OUTPUT
