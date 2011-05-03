SWIG=../../swig/preinst-swig
INCLUDE=-I. -I./../generic $(shell xapian-config --cxxflags)
LINK=$(shell xapian-config --libs)

all: static

static: libs
	ocamlc -pp "camlp4o ./swigp4.cmo" -c smoketest.ml	
	ocamlfind ocamlc -g -ccopt -g -cclib			\
		-g -custom  -o smoketest			\
		-package unix  -linkpkg				\
		swig.cmo  xapian.cmo  smoketest.cmo		\
		xapian_wrap.o					\
		-cc 'g++ -Wno-write-strings $(INCLUDE) $(LINK)'

swig:
	$(SWIG) -ocaml -co swigp4.ml
	$(SWIG) -ocaml -co swig.mli
	$(SWIG) -ocaml -co swig.ml
	ocamlfind ocamlc -package camlp4			\
		-pp "camlp4o pa_extend.cmo q_MLast.cmo"		\
		-c swigp4.ml					
	ocamlc -c swig.mli swig.ml


libs: swig
	$(SWIG) -Wall $(INCLUDE) -c++ -ocaml    \
		-outdir . -o xapian_wrap.c ../xapian.i		
	ocamlc -cc 'g++ -Wno-write-strings' -g -c -ccopt	\
		-g -ccopt '-xc++' -ccopt '$(INCLUDE)'		\
		xapian_wrap.c					
	ocamlc -c xapian.mli xapian.ml


toplevel: libs
	ocamlfind ocamlmktop -custom swig.cmo			\
		-package dynlink -package camlp4		\
		dynlink.cma camlp4o.cma swigp4.cmo 		\
		xapian_wrap.o xapian.cmo -o xapian_top		\
		-cc 'g++ -Wno-write-strings $(INCLUDE) $(LINK)'


clean:
	rm -rf *.dSYM smoketest
	rm -f swigp4.ml swig.mli swig.ml
	rm -f *.cmo *.cmi
	rm -f xapian_wrap.*
	rm -f xapian.*
	rm -f xapian_top
