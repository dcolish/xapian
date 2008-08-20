# Makefile for Microsoft Visual C++ 7.0 (or compatible)
# Written by Zsolt Sz. Sztupák by modifying the Python bindings code made by Ulrik Petersen
#  and Modified by Charlie Hull, Lemur Consulting Ltd. www.lemurconsulting.com
#  email: mail AT sztupy DOT hu

# Will build the Ruby bindings 

# Where the core is, relative to the Ruby bindings
# Change this to match your environment
XAPIAN_CORE_REL_RUBY=..\..\xapian-core-1.0.7

OUTLIBDIR=$(XAPIAN_CORE_REL_RUBY)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\libs

!INCLUDE $(XAPIAN_CORE_REL_RUBY)\win32\config.mak

LIB_XAPIAN_OBJS= ".\xapian_wrap.obj"

OUTDIR=$(XAPIAN_CORE_REL_RUBY)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\Ruby
INTDIR=.\

ALL : "$(OUTDIR)\_xapian.so" "$(OUTDIR)\xapian.rb" "$(OUTDIR)\smoketest.rb" 

CLEAN :
	-@erase $(LIB_XAPIAN_OBJS)
	-@erase /Q /s "$(OUTDIR)\$(RUBY_PACKAGE)"
	-@erase /Q /s "$(OUTDIR)"
	-@erase *.so

CLEANSWIG:
    -@erase xapian_wrap.cc
    -@erase xapian_wrap.h
    

DOTEST :
	cd "$(OUTDIR)"
	copy "$(ZLIB_BIN_DIR)\zlib1.dll"
	"$(RUBY_EXE)" smoketest.rb
	
CHECK: ALL DOTEST	

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"   

CPP_PROJ=$(CPPFLAGS_EXTRA)  /GR \
 /I "$(XAPIAN_CORE_REL_RUBY)" /I "$(XAPIAN_CORE_REL_RUBY)\include" \
 /I "$(RUBY_INCLUDE)" /I"." \
 /Fo"$(INTDIR)\\" /Tp$(INPUTNAME)
CPP_OBJS=$(XAPIAN_CORE_REL_RUBY)\win32\$(XAPIAN_DEBUG_OR_RELEASE)\
CPP_SBRS=.

ALL_LINK32_FLAGS=$(LINK32_FLAGS) $(XAPIAN_LIBS) $(RUBY_LIB_DIR)\msvcrt-ruby18.lib "/LIBPATH:$(RUBY_LIB_DIR)" 

!IF "$(SWIGBUILD)" == "1"
xapian_wrap.cc xapian_wrap.h: util.i ..\xapian.i
	$(SWIG) $(SWIG_FLAGS) -I$(XAPIAN_CORE_REL_RUBY)\include -I..\generic \
        -c++ \
	    -initname _xapian -ruby \
	    -o xapian_wrap.cc ..\xapian.i      
        
!ENDIF

"$(OUTDIR)\_xapian.so" : "$(OUTDIR)" $(DEF_FILE) $(LIB_XAPIAN_OBJS) 
                            
    $(LINK32) @<<
  $(ALL_LINK32_FLAGS) /DLL /out:"$(OUTDIR)\_xapian.so" $(DEF_FLAGS) $(LIB_XAPIAN_OBJS)
<<

"$(OUTDIR)\xapian.rb" : ".\xapian.rb"
	-copy $** "$(OUTDIR)\xapian.rb"
	$(MANIFEST) "$(OUTDIR)\_xapian.so.manifest" -outputresource:"$(OUTDIR)\_xapian.so;2"


"$(OUTDIR)\smoketest.rb" : ".\smoketest.rb"
	-copy $** "$(OUTDIR)\smoketest.rb"

INSTALL : ALL
	-copy "$(OUTDIR)\xapian.rb" "$(RUBY_RB_DIR)\xapian.rb"
	-copy "$(OUTDIR)\_xapian.so" "$(RUBY_SO_DIR)\_xapian.so"	

TEST : INSTALL
	$(RUBY_EXE) smoketest.rb
#
# Rules
#

".\xapian_wrap.obj" : ".\xapian_wrap.cc"
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
