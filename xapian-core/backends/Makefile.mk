noinst_HEADERS +=\
	backends/slowvaluelist.h

EXTRA_DIST +=\
	backends/dir_contents\
	backends/Makefile

lib_src +=\
	backends/alltermslist.cc\
	backends/database.cc\
	backends/databasereplicator.cc\
	backends/dbfactory.cc\
	backends/slowvaluelist.cc\
	backends/valuelist.cc

if BUILD_BACKEND_REMOTE
lib_src +=\
	backends/dbfactory_remote.cc
endif

if BUILD_BACKEND_FLINT
lib_src +=\
	backends/contiguousalldocspostlist.cc
else
if BUILD_BACKEND_CHERT
lib_src +=\
        backends/contiguousalldocspostlist.cc
endif
endif

# To add a new database backend:
#
# 1) Add lines to configure.ac to define the automake conditional
#    "BUILD_BACKEND_NEWONE"
# 2) Add lines below to "include backends/newone/Makefile.mk"
# 3) Write backends/newone/Makefile.mk - it should add files to noinst_HEADERS
#    and lib_src conditional on BUILD_BACKEND_NEWONE.
# 4) Update backends/dbfactory.cc.
# 5) If it needs to support replication, update backends/databasereplicator.cc
# 6) Write the backend code!

include backends/chert/Makefile.mk
include backends/flint/Makefile.mk
include backends/inmemory/Makefile.mk
include backends/multi/Makefile.mk
include backends/remote/Makefile.mk
