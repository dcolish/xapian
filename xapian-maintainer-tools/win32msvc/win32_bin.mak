# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build quartzcheck.exe, quartzcompact.exe, quartdump.exe and xapian_compact.exe


!INCLUDE ..\win32\config.mak

OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
OUTLIBDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

PROGRAMS = "$(OUTDIR)\quartzcheck.exe" \
           "$(OUTDIR)\quartzcompact.exe" \
           "$(OUTDIR)\quartzdump.exe" \
           "$(OUTDIR)\xapian-compact.exe" \
           "$(OUTDIR)\xapian-progsrv.exe" \
           "$(OUTDIR)\xapian-tcpsrv.exe" \
           "$(OUTDIR)\xapian-inspect.exe" \
           "$(OUTDIR)\xapian-check.exe" \
           "$(OUTDIR)\xapian-replicate.exe" \
           "$(OUTDIR)\xapian-replicate-server.exe"

SRCS = \
	"$(INTDIR)\quartzcheck.cc" \
	"$(INTDIR)\quartzcompact.cc" \
	"$(INTDIR)\quartzdump.cc" \
	"$(INTDIR)\xapian-compact.cc" \
	"$(INTDIR)\xapian-progsrv.cc" \
	"$(INTDIR)\xapian-tcpsrv.cc" \
	"$(INTDIR)\xapian-inspect.cc" \
	"$(INTDIR)\xapian-check.cc" \
    "$(INTDIR)\xapian-replicate.cc" \
    "$(INTDIR)\xapian-replicate-server.cc"
	   
ALL : $(PROGRAMS)

QUARTZCHECK_OBJS= "$(INTDIR)\quartzcheck.obj" 

QUARTZCOMPACT_OBJS= "$(INTDIR)\quartzcompact.obj" 

QUARTZDUMP_OBJS= "$(INTDIR)\quartzdump.obj" 

XAPIAN_COMPACT_OBJS= "$(INTDIR)\xapian-compact.obj" 

XAPIAN_PROGSRV_OBJS= "$(INTDIR)\xapian-progsrv.obj" 

XAPIAN_TCPSRV_OBJS= "$(INTDIR)\xapian-tcpsrv.obj" 

XAPIAN_INSPECT_OBJS= "$(INTDIR)\xapian-inspect.obj" 

XAPIAN_CHECK_OBJS= "$(INTDIR)\xapian-check.obj" 

XAPIAN_REPLICATE_OBJS= "$(INTDIR)\xapian-replicate.obj" 

XAPIAN_REPLICATE_SERVER_OBJS= "$(INTDIR)\xapian-replicate-server.obj" 


	
CLEAN :
	-@erase $(PROGRAMS)
	-@erase $(QUARTZCHECK_OBJS)
	-@erase $(QUARTZCOMPACT_OBJS)
	-@erase $(QUARTZDUMP_OBJS)
	-@erase $(XAPIAN_COMPACT_OBJS)
	-@erase $(XAPIAN_PROGSRV_OBJS)
	-@erase $(XAPIAN_TCPSRV_OBJS)
	-@erase $(XAPIAN_INSPECT_OBJS)
	-@erase $(XAPIAN_CHECK_OBJS)
	-@erase $(XAPIAN_REPLICATE_OBJS)
	-@erase $(XAPIAN_REPLICATE_SERVER_OBJS)
	-@erase "$(INTDIR)\*.pdb"


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA)  \
 /I ".." /I "..\testsuite" /I"..\backends\quartz" /I"..\backends\flint" \
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /Tp$(INPUTNAME)

CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

ALL_LINK32_FLAGS=$(LINK32_FLAGS) $(XAPIAN_LIBS)
 
PROGRAM_DEPENDENCIES = $(XAPIAN_LIBS)



"$(OUTDIR)\quartzcheck.exe" : "$(OUTDIR)" $(DEF_FILE) $(QUARTZCHECK_OBJS) \
                      $(PROGRAM_DEPENDENCIES) 
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\quartzcheck.exe" $(DEF_FLAGS) $(QUARTZCHECK_OBJS) "$(OUTLIBDIR)\libquartzbtreecheck.lib" 
<<

"$(OUTDIR)\quartzcompact.exe" : "$(OUTDIR)" $(DEF_FILE) $(QUARTZCOMPACT_OBJS) \
                                 $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\quartzcompact.exe" $(DEF_FLAGS) $(QUARTZCOMPACT_OBJS)
<<

"$(OUTDIR)\quartzdump.exe" : "$(OUTDIR)" $(DEF_FILE) $(QUARTZDUMP_OBJS) \
                             $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\quartzdump.exe" $(DEF_FLAGS) $(QUARTZDUMP_OBJS)
<<

"$(OUTDIR)\xapian-compact.exe" : "$(OUTDIR)" $(DEF_FILE) $(XAPIAN_COMPACT_OBJS) \
                             $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\xapian-compact.exe" $(DEF_FLAGS) $(XAPIAN_COMPACT_OBJS)
<<

"$(OUTDIR)\xapian-progsrv.exe" : "$(OUTDIR)" $(DEF_FILE) $(XAPIAN_PROGSRV_OBJS) \
                             $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\xapian-progsrv.exe" $(DEF_FLAGS) $(XAPIAN_PROGSRV_OBJS)
<<

"$(OUTDIR)\xapian-tcpsrv.exe" : "$(OUTDIR)" $(DEF_FILE) $(XAPIAN_TCPSRV_OBJS) \
                             $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\xapian-tcpsrv.exe" $(DEF_FLAGS) $(XAPIAN_TCPSRV_OBJS)
<<

"$(OUTDIR)\xapian-inspect.exe" : "$(OUTDIR)" $(DEF_FILE) $(XAPIAN_INSPECT_OBJS) \
                             $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\xapian-inspect.exe" $(DEF_FLAGS) $(XAPIAN_INSPECT_OBJS)
<<

"$(OUTDIR)\xapian-check.exe" : "$(OUTDIR)" $(DEF_FILE) $(XAPIAN_CHECK_OBJS) \
                             $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\xapian-check.exe" $(DEF_FLAGS) $(XAPIAN_CHECK_OBJS) "$(OUTLIBDIR)\libflintbtreecheck.lib" 
<<

"$(OUTDIR)\xapian-replicate.exe" : "$(OUTDIR)" $(DEF_FILE) $(XAPIAN_REPLICATE_OBJS) \
                      $(PROGRAM_DEPENDENCIES) 
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\xapian-replicate.exe" $(DEF_FLAGS) $(XAPIAN_REPLICATE_OBJS) 
<<

"$(OUTDIR)\xapian-replicate-server.exe" : "$(OUTDIR)" $(DEF_FILE) $(XAPIAN_REPLICATE_SERVER_OBJS) \
                      $(PROGRAM_DEPENDENCIES) 
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /out:"$(OUTDIR)\xapian-replicate-server.exe" $(DEF_FLAGS) $(XAPIAN_REPLICATE_SERVER_OBJS) 
<<

# inference rules, showing how to create one type of file from another with the same root name
{.}.cc{$(INTDIR)}.obj::
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

{.}.cc{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

# Calculate any header dependencies and automatically insert them into this file
HEADERS :
            if exist ..\..\win32\$(DEPEND) ..\..\win32\$(DEPEND) -- $(CPP_PROJ) -- $(SRCS) -I"$(INCLUDE)"
# DO NOT DELETE THIS LINE -- make depend depends on it.

