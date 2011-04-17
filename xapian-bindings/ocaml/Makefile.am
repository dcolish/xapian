## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

TESTS_ENVIRONMENT = \
srcdir="$(srcdir)" LIBTOOL="$(LIBTOOL)" OCAML="$(OCAML)"

## Test programs to be run
#TESTS = smoketest.lua

# EXTRA_DIST = lua_util.i $(TESTS) $(BUILT_SOURCES)

# ocamllibdir = @OCAML_LIB@/xapian@VERSION@

# ocaml_LTLIBRARIES = xapian.la

# # Remove the .la file - xapian.la is never linked against (it's a module)
# # and LUA doesn't use libltdl.  Note that the library gets installed by
# # install-data, so that's where we need to hook.
# install-data-hook:
# 	rm -f $(DESTDIR)$(lualibdir)/xapian.la

# # Because we don't install the .la file, "make uninstall" doesn't work and
# # we need to remove the file ourselves.
# uninstall-local:
# 	eval `grep '^dlname=' $(lualib_LTLIBRARIES)` ; \
# 	  rm -f $(DESTDIR)$(lualibdir)/"$$dlname"

AM_CPPFLAGS = @OCAML_CPPFLAGS@
AM_CXXFLAGS = @SWIG_CXXFLAGS@ $(XAPIAN_CXXFLAGS)
xapian_la_LDFLAGS = -avoid-version -module $(NO_UNDEFINED)
nodist_xapian_la_SOURCES = xapian_wrap.cc
xapian_la_LIBADD = $(XAPIAN_LIBS) $(OCAML_LIBS)

BUILT_SOURCES = xapian_wrap.cc

if MAINTAINER_MODE
xapian_wrap.cc: $(SWIG_sources) util.i
	$(SWIG) $(SWIG_includes) $(SWIG_FLAGS) -c++ \
	    -ocaml \
	    -o xapian_wrap.cc -MD -MF xapian_wrap_d.tmp $(SWIG_mainsource)
	mv xapian_wrap_d.tmp xapian_wrap.d

-include xapian_wrap.d
endif
MAINTAINERCLEANFILES = $(BUILT_SOURCES)
