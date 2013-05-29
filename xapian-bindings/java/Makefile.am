## Process this file with automake to produce Makefile.in

include ../generic/generic.mk

## Test programs to be run
TESTS = SmokeTest.class
AM_TESTS_ENVIRONMENT = \
	LIBTOOL="$(LIBTOOL)" ;\
	JAVA="$(JAVA)" ;\
	PATHSEP="$(JAVA_PATHSEP)" ;\
	export LIBTOOL ;\
	export JAVA ;\
	export PATHSEP ;
LOG_COMPILER = $(srcdir)/run-java-test

CLEANFILES = SmokeTest.class MyMatchDecider.class MyExpandDecider.class\
	$(XAPIAN_SWIG_JAVA_CLASS)

# SmokeTest: SmokeTest.class

# # To get debug symbols, use: make JAVACFLAGS=-g
# .java.class:
# 	$(JAVAC) $(JAVACFLAGS) -classpath $(srcdir)$(JAVA_PATHSEP). -d . $<

XAPIAN_SWIG_JAVA_SRCS=\
	org/xapian/core/Auto.java\
	org/xapian/core/BM25Weight.java\
	org/xapian/core/BoolWeight.java\
	org/xapian/core/Brass.java\
	org/xapian/core/Chert.java\
	org/xapian/core/Compactor.java\
	org/xapian/core/Database.java\
	org/xapian/core/DateValueRangeProcessor.java\
	org/xapian/core/DecreasingValueWeightPostingSource.java\
	org/xapian/core/Document.java\
	org/xapian/core/Enquire.java\
	org/xapian/core/ESet.java\
	org/xapian/core/ESetIterator.java\
	org/xapian/core/ExpandDecider.java\
	org/xapian/core/ExpandDeciderAnd.java\
	org/xapian/core/ExpandDeciderFilterPrefix.java\
	org/xapian/core/FieldProcessor.java\
	org/xapian/core/FixedWeightPostingSource.java\
	org/xapian/core/GreatCircleMetric.java\
	org/xapian/core/InMemory.java\
	org/xapian/core/KeyMaker.java\
	org/xapian/core/LatLongCoord.java\
	org/xapian/core/LatLongCoords.java\
	org/xapian/core/LatLongCoordsIterator.java\
	org/xapian/core/LatLongDistanceKeyMaker.java\
	org/xapian/core/LatLongDistancePostingSource.java\
	org/xapian/core/LatLongMetric.java\
	org/xapian/core/MatchDecider.java\
	org/xapian/core/MatchSpy.java\
	org/xapian/core/MSet.java\
	org/xapian/core/MSetIterator.java\
	org/xapian/core/MultiValueKeyMaker.java\
	org/xapian/core/NumberValueRangeProcessor.java\
	org/xapian/core/PositionIterator.java\
	org/xapian/core/PostingIterator.java\
	org/xapian/core/PostingSource.java\
	org/xapian/core/Query.java\
	org/xapian/core/QueryParser.java\
	org/xapian/core/Registry.java\
	org/xapian/core/Remote.java\
	org/xapian/core/RSet.java\
	org/xapian/core/SimpleStopper.java\
	org/xapian/core/Stem.java\
	org/xapian/core/StemImplementation.java\
	org/xapian/core/Stopper.java\
	org/xapian/core/StringValueRangeProcessor.java\
	org/xapian/core/SWIGTYPE_p_std__string.java\
	org/xapian/core/TermGenerator.java\
	org/xapian/core/TermIterator.java\
	org/xapian/core/TfIdfWeight.java\
	org/xapian/core/TradWeight.java\
	org/xapian/core/ValueCountMatchSpy.java\
	org/xapian/core/ValueIterator.java\
	org/xapian/core/ValueMapPostingSource.java\
	org/xapian/core/ValuePostingSource.java\
	org/xapian/core/ValueRangeProcessor.java\
	org/xapian/core/ValueSetMatchDecider.java\
	org/xapian/core/ValueWeightPostingSource.java\
	org/xapian/core/Version.java\
	org/xapian/core/Weight.java\
	org/xapian/core/WritableDatabase.java\
	org/xapian/core/Xapian.java\
	org/xapian/core/XapianConstants.java\
	org/xapian/core/XapianJNI.java

XAPIAN_SWIG_JAVA_CLASS = $(XAPIAN_SWIG_JAVA_SRCS:.java=.class)

# # Java generates nested classes with filenames containing a $ (smart move) so
# # we pick them up with a wildcard and omit them from dependencies to avoid
# # escaping hell.  The lack of a dependency shouldn't really be an issue since
# # these classes are always generated along with the containing class which
# # is listed in the dependencies.
# XAPIAN_SWIG_JAVA_EXTRA_CLASSES=\
# 	org/xapian/Compactor*compaction_level.class\
# 	org/xapian/Enquire*docid_order.class\
# 	org/xapian/Query*1.class\
# 	org/xapian/Query*op.class\
# 	org/xapian/QueryParser*feature_flag.class\
# 	org/xapian/QueryParser*stem_strategy.class\
# 	org/xapian/TermGenerator*flags.class

# noinst_DATA = built/xapian_jni.jar

# built/xapian_jni.jar: $(XAPIAN_SWIG_JAVA_CLASS)
# 	-test -d built || mkdir built
# 	$(JAR) -cf built/xapian_jni.jar org/xapian/*.class
##$(XAPIAN_SWIG_JAVA_CLASS) $(XAPIAN_SWIG_JAVA_EXTRA_CLASSES)

noinst_DATA = xapian-core/src/main/resources/libxapian_jni.so
xapian-core/src/main/resources/libxapian_jni.so:
	cp .libs/libxapian_jni.so xapian-core/src/main/resources/

jnidir = `pwd`/built

jni_LTLIBRARIES = libxapian_jni.la

# Remove the .la file - libxapian_jni.la is never linked against (it's a
# module) and JVMs don't use libltdl.  Note that the library gets installed by
# install-data, so that's where we need to hook.
install-data-hook:
	rm -f $(DESTDIR)$(jnidir)/libxapian_jni.la

# Because we don't install the .la file, "make uninstall" doesn't work and
# we need to remove the file ourselves.
uninstall-local:
if NEED_JNILIB_EXT
	rm -f $(DESTDIR)$(jnidir)/libxapian_jni.jnilib
else
	eval `grep '^dlname=' $(jni_LTLIBRARIES)` ; \
	  rm -f $(DESTDIR)$(jnidir)/"$$dlname"
endif

if NEED_JNILIB_EXT
SHREXT=-shrext .jnilib
else
SHREXT=
endif
AM_CXXFLAGS = $(SWIG_CXXFLAGS) $(XAPIAN_CXXFLAGS)
AM_CPPFLAGS = $(JAVA_CPPFLAGS)
libxapian_jni_la_LDFLAGS = -avoid-version -module $(SHREXT) $(NO_UNDEFINED)
libxapian_jni_la_LIBADD = $(XAPIAN_LIBS)
libxapian_jni_la_SOURCES = xapian_wrap.cc

BUILT_SOURCES = xapian_wrap.cc xapian_wrap.h $(XAPIAN_SWIG_JAVA_SRCS)

EXTRA_DIST = run-java-test SmokeTest.java $(BUILT_SOURCES)

if MAINTAINER_MODE
stamp = xapian_wrap.stamp
RUN_SWIG = stamp='$(stamp)' $(PERL) '$(top_srcdir)'/swig-depcomp $(SWIG)

xapian_wrap.cc xapian_wrap.h $(XAPIAN_SWIG_JAVA_SRCS): $(stamp)
	$(make_many_locked)
$(stamp):
	$(multitarget_begin)
	: # Make sure that we don't package stale generated sources in the
	: # case where SWIG changes its mind as to which files it generates.
	-rm -f org/xapian/core/*
	$(MKDIR_P) org/xapian/core
	$(RUN_SWIG) $(SWIG_WERROR) -I'$(srcdir)' $(SWIG_FLAGS) -c++ \
	    -java -module Xapian -package org.xapian.core \
	    -outdir org/xapian/core \
	    -o xapian_wrap.cc '$(srcdir)/'java.i
	: # Insert code to automatically load the JNI library.
	$(PERL) -pi -e 'print "    org.xapian.util.LoaderUtils.loadDynamic();\n" if /^\s*swig_module_init/' org/xapian/core/XapianJNI.java
	: # Check that exactly the sources we expected were generated.
	$(PERL) -e 'print "$$_\n" for sort @ARGV' org/xapian/core/*.java > org/xapian/core/list
	$(PERL) -e 'print "$$_\n" for sort @ARGV' $(XAPIAN_SWIG_JAVA_SRCS) |diff -c - org/xapian/core/list
	cp -r org/xapian/core/* xapian-core/src/main/java/org/xapian/core/
	$(multitarget_end)

-include xapian_wrap.d

CLEANFILES += $(stamp) \
	xapian-core/src/main/java/org/xapian/core/*.java

endif
MAINTAINERCLEANFILES = $(BUILT_SOURCES)

maintainer-clean-local:
	rm -rf built org 

exampledatadir = $(docdir)/java/examples
dist_exampledata_DATA =\
	docs/examples/SimpleSearch.java\
	docs/examples/SimpleIndex.java

docdatadir = $(docdir)/java
dist_docdata_DATA = docs/index.html
