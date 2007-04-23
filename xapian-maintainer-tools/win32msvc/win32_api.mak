# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libapi.lib



!INCLUDE ..\win32\config.mak


OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libapi.lib" 

LIBAPI_OBJS= \
             $(INTDIR)\error.obj \
             $(INTDIR)\errorhandler.obj \
             $(INTDIR)\expanddecider.obj \
             $(INTDIR)\omenquire.obj  \
             $(INTDIR)\omquery.obj  \
             $(INTDIR)\omqueryinternal.obj  \
             $(INTDIR)\omdatabase.obj  \
             $(INTDIR)\omdocument.obj  \
             $(INTDIR)\ompostlistiterator.obj  \
             $(INTDIR)\ompositionlistiterator.obj  \
             $(INTDIR)\omtermlistiterator.obj  \
             $(INTDIR)\omvalueiterator.obj \
             $(INTDIR)\valuerangeproc.obj \
	     $(INTDIR)\version.obj


CLEAN :
	-@erase "$(OUTDIR)\libapi.lib"
	-@erase "*.pch"
        -@erase $(LIBAPI_OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 /I"..\languages" \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.

"$(OUTDIR)\LIBAPI.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBAPI_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libapi.lib" $(DEF_FLAGS) $(LIBAPI_OBJS)
<<

"$(INTDIR)\error.obj" : ".\error.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\errorhandler.obj" : ".\errorhandler.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\expanddecider.obj" : ".\expanddecider.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\omenquire.obj" : ".\omenquire.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\omquery.obj" : ".\omquery.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\omqueryinternal.obj" : ".\omqueryinternal.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\omdatabase.obj" : ".\omdatabase.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\omdocument.obj" : ".\omdocument.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<



"$(INTDIR)\ompostlistiterator.obj" : ".\ompostlistiterator.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\ompositionlistiterator.obj" : ".\ompositionlistiterator.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\omtermlistiterator.obj" : ".\omtermlistiterator.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\omvalueiterator.obj" : ".\omvalueiterator.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\valuerangeproc.obj" : ".\valuerangeproc.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\version.obj" : ".\version.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<


{.}.cc{$(INTDIR)}.obj:
	$(CPP) @<<
	$(CPP_PROJ) $< 
<<

{.}.cc{$(CPP_SBRS)}.sbr:
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

