# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 3rd Jan 2007


# Will build a Win32 static library (non-debug) libremote.lib

!INCLUDE ..\..\win32\config.mak

OUTDIR=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libremote.lib" 

LIBREMOTE_OBJS= \
                 $(INTDIR)\remote-database.obj \
                 $(INTDIR)\net_document.obj \
                 $(INTDIR)\net_termlist.obj \

CLEAN :
	-@erase "$(OUTDIR)\libremote.lib"
	-@erase "*.pch"
    -@erase "$(INTDIR)\getopt.obj"
	-@erase $(LIBREMOTE_OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA)  \
 /I "..\.." /I "..\..\include" /I"..\..\common" /I"..\..\languages" \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME) 
CPP_OBJS=..\..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.


"$(OUTDIR)\LIBREMOTE.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIBREMOTE_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libremote.lib" $(DEF_FLAGS) $(LIBREMOTE_OBJS)
<<



"$(INTDIR)\remote-database.obj" : "remote-database.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<
   
"$(INTDIR)\net_document.obj" : "net_document.cc"
       $(CPP) @<<
   $(CPP_PROJ) $**
<<


"$(INTDIR)\net_termlist.obj" : "net_termlist.cc"
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

