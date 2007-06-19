# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen and Charlie Hull
# Modified by Mark Hammond.
# May 2007

# Will build a Win32 static library (non-debug) libexpand.lib


!INCLUDE ..\win32\config.mak

OUTDIR=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs
INTDIR=.\

ALL : "$(OUTDIR)\libexpand.lib" 

OBJS= \
                 $(INTDIR)\ortermlist.obj \
                 $(INTDIR)\expandweight.obj \
                 $(INTDIR)\expand.obj \
                 $(NULL)
SRCS= \
                 $(INTDIR)\ortermlist.cc \
                 $(INTDIR)\expandweight.cc \
                 $(INTDIR)\expand.cc
		 
CLEAN :
	-@erase "$(OUTDIR)\libexpand.lib"
	-@erase "$(INTDIR)\*.pdb"
	-@erase "*.pch"
	-@erase $(OBJS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) \
 -I"..\languages" \
 -Fo"$(INTDIR)\\" -Tp$(INPUTNAME)
 
CPP_OBJS=..\win32\$(XAPIAN_DEBUG_OR_RELEASE)
CPP_SBRS=.


"$(OUTDIR)\LIBEXPAND.lib" : HEADERS "$(OUTDIR)" $(DEF_FILE) $(OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) /out:"$(OUTDIR)\libexpand.lib" $(DEF_FLAGS) $(OBJS)
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
            ..\win32\$(DEPEND) -- $(CPP_PROJ) -- $(SRCS) -I"$(INCLUDE)"
# DO NOT DELETE THIS LINE -- make depend depends on it.

