# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com

# Will build a Win32 static library (non-debug) libapi.lib



!INCLUDE ..\win32\config.mak


OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libapi.lib" 

OBJS= \
    $(INTDIR)/documentvaluelist.obj\
    $(INTDIR)/editdistance.obj \
    $(INTDIR)/emptypostlist.obj \
    $(INTDIR)/error.obj \
    $(INTDIR)/errorhandler.obj \
    $(INTDIR)/expanddecider.obj \
    $(INTDIR)/leafpostlist.obj \
    $(INTDIR)/matchspy.obj \
    $(INTDIR)/omdatabase.obj \
    $(INTDIR)/omdocument.obj \
    $(INTDIR)/omenquire.obj \
    $(INTDIR)/ompositionlistiterator.obj \
    $(INTDIR)/ompostlistiterator.obj \
    $(INTDIR)/omquery.obj \
    $(INTDIR)/omqueryinternal.obj \
    $(INTDIR)/omtermlistiterator.obj \
    $(INTDIR)/postlist.obj \
    $(INTDIR)/postingsource.obj \
    $(INTDIR)/replication.obj \
    $(INTDIR)/serialisationcontext.obj \
    $(INTDIR)/sortable-serialise.obj \
    $(INTDIR)/sorter.obj \
    $(INTDIR)/termlist.obj \
    $(INTDIR)/valueiterator.obj\
    $(INTDIR)/valuerangeproc.obj \
    $(INTDIR)/valuesetmatchdecider.obj \
    $(INTDIR)/version.obj
    
SRCS= \
    $(INTDIR)/documentvaluelist.cc\
    $(INTDIR)/editdistance.cc\
    $(INTDIR)/emptypostlist.cc\
    $(INTDIR)/error.cc\
    $(INTDIR)/errorhandler.cc\
    $(INTDIR)/expanddecider.cc\
    $(INTDIR)/leafpostlist.cc\
    $(INTDIR)/matchspy.cc \
    $(INTDIR)/omdatabase.cc\
    $(INTDIR)/omdocument.cc\
    $(INTDIR)/omenquire.cc\
    $(INTDIR)/ompositionlistiterator.cc\
    $(INTDIR)/ompostlistiterator.cc\
    $(INTDIR)/omquery.cc\
    $(INTDIR)/omqueryinternal.cc\
    $(INTDIR)/omtermlistiterator.cc\
    $(INTDIR)/postlist.cc\
    $(INTDIR)/postingsource.cc \
    $(INTDIR)/replication.cc \
    $(INTDIR)/serialisationcontext.cc \
    $(INTDIR)/sortable-serialise.cc\
    $(INTDIR)/sorter.cc\
    $(INTDIR)/termlist.cc\
    $(INTDIR)/valueiterator.cc\
    $(INTDIR)/valuerangeproc.cc\
    $(INTDIR)/valuesetmatchdecider.cc \
    $(INTDIR)/version.cc

	     
CLEAN :
    -@erase "$(OUTDIR)\libapi.lib"
    -@erase "*.pch"
    -@erase "$(INTDIR)\*.pdb"
    -@erase "$(INTDIR)\*.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 -I"..\languages" \
 -Fo"$(INTDIR)\\" -Tp$(INPUTNAME)
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

"$(OUTDIR)\LIBAPI.lib" : HEADERS "$(OUTDIR)" $(DEF_FILE) $(OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libapi.lib" $(DEF_FLAGS) $(OBJS)
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
            if exist "..\win32\$(DEPEND)" ..\win32\$(DEPEND) $(DEPEND_FLAGS) -- $(CPP_PROJ) -- $(SRCS) -I"$(INCLUDE)" 
# DO NOT DELETE THIS LINE -- make depend depends on it.
