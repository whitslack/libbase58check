ACLOCAL_AMFLAGS = -I build-aux/m4

AM_CPPFLAGS =
if NDEBUG
AM_CPPFLAGS += -DNDEBUG
endif
COMMON_CFLAGS := -Wall -Wextra -Wcast-qual -Wconversion -Wdisabled-optimization -Wdouble-promotion -Wno-implicit-fallthrough -Wmissing-declarations -Wno-missing-field-initializers -Wpacked -Wno-parentheses -Wredundant-decls -Wno-sign-conversion $(addprefix -Wsuggest-attribute=,pure const noreturn malloc) -Wno-vla
AM_CFLAGS = $(COMMON_CFLAGS) $(addprefix -Werror=,implicit-function-declaration incompatible-pointer-types int-conversion)
AM_CXXFLAGS = $(COMMON_CFLAGS) -Wnoexcept -Wold-style-cast -Wsign-promo -Wsuggest-override -Wno-terminate -Wzero-as-null-pointer-constant

include_HEADERS = base58check.h

pkgconfig_DATA = libbase58check.pc
EXTRA_DIST = $(pkgconfig_DATA)

lib_LTLIBRARIES = libbase58check.la
libbase58check_la_SOURCES = libbase58check.c
libbase58check_la_CFLAGS = $(GMP_CFLAGS) $(OPENSSL_CFLAGS)
libbase58check_la_LIBADD = $(GMP_LIBS) $(OPENSSL_LIBS)
# How to update version-info:
# - oldprog+newlib and newprog+oldlib are both okay => +0:+1:+0
# - oldprog+newlib is okay, but newprog+oldlib won't work => +1:=0:+1
# - oldprog+newlib won't work => +1:=0:=0
libbase58check_la_LDFLAGS = -no-undefined -version-info 0:1:0

bin_PROGRAMS = base58check
base58check_SOURCES = base58check.c
base58check_LDADD = libbase58check.la

if BUILD_TESTS

check_PROGRAMS = test
test_SOURCES = test.cpp
test_CPPFLAGS = $(filter-out -DNDEBUG,$(AM_CPPFLAGS))
test_LDFLAGS = -no-install
test_LDADD = libbase58check.la

TESTS = $(check_PROGRAMS)
noinst_PROGRAMS = $(check_PROGRAMS)

else

check-local :
	@! echo "You didn't enable the tests! Run './configure --enable-tests'." >&2

endif # BUILD_TESTS

@DX_RULES@
MOSTLYCLEANFILES = $(DX_CLEANFILES)

if BUILD_MANPAGES
man3_MANS = $(addprefix doxygen-doc/man/man3/,$(addsuffix .3,base58check.h \
	$(shell $(SED) -Ene 's/^((\w|\*)+\s+)+(\w+)\(.*$$/\3/p#)' $(srcdir)/base58check.h)))
$(man3_MANS) : doxygen-doc
$(DX_DOCDIR)/$(PACKAGE).tag : $(include_HEADERS)
endif
