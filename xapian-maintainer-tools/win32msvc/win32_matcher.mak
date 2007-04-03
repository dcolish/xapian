# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 17th March 2006

# Will build a Win32 static library (non-debug) libmatcher.lib


!INCLUDE ..\win32\config.mak

OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libmatcher.lib" 

LIBMATCHER_OBJS= \
                 $(INTDIR)\orpostlist.obj \
                 $(INTDIR)\andpostlist.obj \
                 $(INTDIR)\andnotpostlist.obj \
                 $(INTDIR)\andmaybepostlist.obj \
                 $(INTDIR)\xorpostlist.obj \
                 $(INTDIR)\phrasepostlist.obj \
                 $(INTDIR)\selectpostlist.obj \
                 $(INTDIR)\filterpostlist.obj \
                 $(INTDIR)\ortermlist.obj \
                 $(INTDIR)\expandweight.obj \
                 $(INTDIR)\rset.obj \
                 $(INTDIR)\bm25weight.obj \
                 $(INTDIR)\tradweight.obj \
                 $(INTDIR)\localmatch.obj \
                 $(INTDIR)\multimatch.obj \
                 $(INTDIR)\expand.obj \
                 $(INTDIR)\stats.obj \
                 $(INTDIR)\mergepostlist.obj \
                 $(INTDIR)\msetpostlist.obj \
		 $(INTDIR)\msetcmp.obj \
                 $(INTDIR)\emptysubmatch.obj \
                 $(INTDIR)\exactphrasepostlist.obj \
                 $(INTDIR)\valuerangepostlist.obj \
                 $(INTDIR)\weight.obj \
                 $(INTDIR)\remotesubmatch.obj \
                 $(NULL)

CLEAN :
	-@erase "$(OUTDIR)\libmatcher.lib"
	-@erase "*.pch"
        -@erase "$(INTDIR)\getopt.obj"
	-@erase $(LIBMATCHER_OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 /I"..\languages" \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
 
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.


"$(OUTDIR)\LIBMATCHER.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBMATCHER_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libmatcher.lib" $(DEF_FLAGS) $(LIBMATCHER_OBJS)
<<

"$(INTDIR)\orpostlist.obj" : ".\orpostlist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\andpostlist.obj" : ".\andpostlist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\andnotpostlist.obj" : ".\andnotpostlist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\andmaybepostlist.obj" : ".\andmaybepostlist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\xorpostlist.obj" : ".\xorpostlist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\phrasepostlist.obj" : ".\phrasepostlist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\selectpostlist.obj" : ".\selectpostlist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\filterpostlist.obj" : ".\filterpostlist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\ortermlist.obj" : ".\ortermlist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\expandweight.obj" : ".\expandweight.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\rset.obj" : ".\rset.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\bm25weight.obj" : ".\bm25weight.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\tradweight.obj" : ".\tradweight.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\localmatch.obj" : ".\localmatch.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\multimatch.obj" : ".\multimatch.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\expand.obj" : ".\expand.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\stats.obj" : ".\stats.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\mergepostlist.obj" : ".\mergepostlist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\msetpostlist.obj" : ".\msetpostlist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\networkmatch.obj" : ".\networkmatch.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\msetcmp.obj" : ".\msetcmp.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\emptysubmatch.obj" : ".\emptysubmatch.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\exactphrasepostlist.obj" : ".\exactphrasepostlist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\valuerangepostlist.obj" : ".\valuerangepostlist.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\weight.obj" : ".\weight.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\remotesubmatch.obj" : ".\remotesubmatch.cc"
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

