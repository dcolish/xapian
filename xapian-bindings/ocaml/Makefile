all: static

static: libs
	ocamlc -pp "camlp4o ./swigp4.cmo" -c smoketest.ml
	ocamlc -g -ccopt -g -cclib -g -custom  -o smoketest \
		swig.cmo \
		xapian.cmo \
		smoketest.cmo \
		xapian_wrap.o \
		-cclib -L../../xapian-core/.libs \
		-cclib -lxapian -cc 'g++ -Wno-write-strings'


libs:
	../../swig/preinst-swig -ocaml -co swigp4.ml
	../../swig/preinst-swig -ocaml -co swig.mli	
	../../swig/preinst-swig -ocaml -co swig.ml
	ocamlc -I ` camlp4 -where` -pp "camlp4o pa_extend.cmo q_MLast.cmo" -c swigp4.ml
	../../swig/preinst-swig -I. -I./../generic -I/Users/dcolish/workspace/xapian/xapian-core/include -c++  -ocaml -outdir .  -o xapian_wrap.c ../xapian.i
	ocamlc -cc 'g++ -Wno-write-strings' -g -c -ccopt -g -ccopt "-xc++ " -ccopt "-I. -I./../generic -I../../xapian-core/include" xapian_wrap.c
	ocamlc -c swig.mli 
	ocamlc -c swig.ml
	ocamlc -c xapian.mli 
	ocamlc -c xapian.ml	

toplevel: libs
	ocamlmktop -custom swig.cmo -I /opt/local/lib/ocaml -I `camlp4 -where` dynlink.cma camlp4o.cma swigp4.cmo xapian_wrap.o xapian.cmo -o xapian_top -cclib -L../../xapian-core/.libs -cclib -lxapian -cc 'g++ -Wno-write-strings'

clean:
	rm -f swigp4.ml swig.mli swig.ml
	rm -f *.cmo *.cmi
	rm -f xapian_wrap.*
	rm -f xapian.*
	rm -f xapian_top
	rm -rf _build smoketest smoketest.dSYM/