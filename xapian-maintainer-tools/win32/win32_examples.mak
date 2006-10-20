# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Originally by Ulrik Petersen
# Modified by Charlie Hull, Lemur Consulting Ltd.
# www.lemurconsulting.com
# 31st March 2006

# Will build the following example programs
# copydatabase.exe
# delve.exe
# quest.exe
# simpleexpand.exe
# simpleindex.exe
# simplesearch.exe

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!INCLUDE ..\win32\config.mak


CPP=cl.exe
RSC=rc.exe

OUTLIBDIR=..\win32\Release\libs\

OUTDIR=..\win32\Release\

INTDIR=.\

PROGRAMS = "$(OUTDIR)\copydatabase.exe" "$(OUTDIR)\delve.exe" "$(OUTDIR)\quest.exe" \
"$(OUTDIR)\simpleexpand.exe" "$(OUTDIR)\simpleindex.exe" "$(OUTDIR)\simplesearch.exe" 

ALL : $(PROGRAMS)

CLEAN :
	-@erase $(PROGRAMS)


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=$(CPPFLAGS_EXTRA) /W3 /GX /O2 \
 /I ".." /I "..\common" /I "..\include" \
 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "__WIN32__" /YX \
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c  /D "HAVE_VSNPRINTF" /D "HAVE_STRDUP"

CPP_OBJS=..\win32\Release
CPP_SBRS=.

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib \
 wsock32.lib odbccp32.lib /subsystem:console \
 "$(OUTLIBDIR)\libgetopt.lib"  \
 "$(OUTLIBDIR)\libcommon.lib"  \
 "$(OUTLIBDIR)\libbtreecheck.lib"  \
 "$(OUTLIBDIR)\libtest.lib"  \
 "$(OUTLIBDIR)\libbackend.lib"  \
 "$(OUTLIBDIR)\libquartz.lib" \
 "$(OUTLIBDIR)\libflint.lib" \
 "$(OUTLIBDIR)\libinmemory.lib" \
 "$(OUTLIBDIR)\libmulti.lib" \
 "$(OUTLIBDIR)\libmatcher.lib"  \
 "$(OUTLIBDIR)\liblanguages.lib"  \
 "$(OUTLIBDIR)\libnet.lib"  \
 "$(OUTLIBDIR)\libapi.lib"  \
 "$(OUTLIBDIR)\libqueryparser.lib"  

	
PROGRAM_DEPENDENCIES = 

 
# delve.exe
# quest.exe
# simpleexpand.exe
# simpleindex.exe
# simplesearch.exe

"$(OUTDIR)\copydatabase.exe" : "$(OUTDIR)" $(DEF_FILE) "$(INTDIR)\copydatabase.obj" \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(LINK32_FLAGS) /out:"$(OUTDIR)\copydatabase.exe" $(DEF_FLAGS) "$(INTDIR)\copydatabase.obj"
<<

"$(OUTDIR)\delve.exe" : "$(OUTDIR)" $(DEF_FILE) "$(INTDIR)\delve.obj" \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(LINK32_FLAGS) /out:"$(OUTDIR)\delve.exe" $(DEF_FLAGS) "$(INTDIR)\delve.obj"
<<

"$(OUTDIR)\quest.exe" : "$(OUTDIR)" $(DEF_FILE) "$(INTDIR)\quest.obj" \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(LINK32_FLAGS) /out:"$(OUTDIR)\quest.exe" $(DEF_FLAGS) "$(INTDIR)\quest.obj"
<<

"$(OUTDIR)\simpleexpand.exe" : "$(OUTDIR)" $(DEF_FILE) "$(INTDIR)\simpleexpand.obj" \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(LINK32_FLAGS) /out:"$(OUTDIR)\simpleexpand.exe" $(DEF_FLAGS) "$(INTDIR)\simpleexpand.obj"
<<

"$(OUTDIR)\simpleindex.exe" : "$(OUTDIR)" $(DEF_FILE) "$(INTDIR)\simpleindex.obj" \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(LINK32_FLAGS) /out:"$(OUTDIR)\simpleindex.exe" $(DEF_FLAGS) "$(INTDIR)\simpleindex.obj"
<<

"$(OUTDIR)\simplesearch.exe" : "$(OUTDIR)" $(DEF_FILE) "$(INTDIR)\simplesearch.obj" \
                      $(PROGRAM_DEPENDENCIES)
    $(LINK32) @<<
  $(LINK32_FLAGS) /out:"$(OUTDIR)\simplesearch.exe" $(DEF_FLAGS) "$(INTDIR)\simplesearch.obj"
<<

    
"$(INTDIR)\copydatabase.obj" : ".\copydatabase.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\delve.obj" : ".\delve.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\quest.obj" : ".\quest.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\simpleexpand.obj" : ".\simpleexpand.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\simpleindex.obj" : ".\simpleindex.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<

"$(INTDIR)\simplesearch.obj" : ".\simplesearch.cc"
        $(CPP) @<<
   $(CPP_PROJ) $**
<<



.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<
