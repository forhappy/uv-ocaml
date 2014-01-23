# Change this to match your camluv installation.
UV_LIB=-luv
UV_LIBDIR=/usr/local/lib
UV_INCDIR=/usr/local/include

NAME=camluv
OBJECTS=camluv.cmo
XOBJECTS=$(OBJECTS:.cmo=.cmx)
C_OBJECTS=camluv.o\
		  camluv_loop.o\
		  camluv_handle.o\
		  camluv_async.o\
		  camluv_check.o\
		  camluv_idle.o\
		  camluv_poll.o\
		  camluv_prepare.o\
		  camluv_timer.o\
		  camluv_signal.o\
		  camluv_fs.o\
		  camluv_fs_event.o\
		  camluv_fs_poll.o\
		  camluv_tcp.o\
		  camluv_udp.o\
		  camluv_pipe.o\
		  camluv_tty.o\
		  camluv_barrier.o\
		  camluv_condition.o\
		  camluv_key.o\
		  camluv_rwlock.o\
		  camluv_sem.o\
		  camluv_thread.o\
		  camluv_threadpool.o

ARCHIVE=$(NAME).cma
XARCHIVE=$(ARCHIVE:.cma=.cmxa)
CARCHIVE_NAME=camluv
CARCHIVE=lib$(CARCHIVE_NAME).a

# Flags for the C compiler.
CFLAGS=-g -O2 -I$(UV_INCDIR)

OCAMLC=ocamlc
OCAMLOPT=ocamlopt
OCAMLDEP=ocamldep
OCAMLMKLIB=ocamlmklib
OCAMLDOC=ocamldoc
OCAMLFIND=ocamlfind

.PHONY: all
all: $(ARCHIVE)
.PHONY: allopt
allopt:  $(XARCHIVE)

depend: *.c *.ml *.mli
	gcc -MM *.c > depend
	$(OCAMLDEP) *.mli *.ml >> depend

## Library creation
$(CARCHIVE): $(C_OBJECTS)
	$(OCAMLMKLIB) -oc $(CARCHIVE_NAME) $(C_OBJECTS) \
	-L$(UV_LIBDIR) $(UV_LIB)
$(ARCHIVE): $(CARCHIVE) $(OBJECTS)
	$(OCAMLMKLIB) -o $(NAME) $(OBJECTS) -oc $(CARCHIVE_NAME) \
	-L$(UV_LIBDIR) $(UV_LIB)
$(XARCHIVE): $(CARCHIVE) $(XOBJECTS)
	$(OCAMLMKLIB) -o $(NAME) $(XOBJECTS) -oc $(CARCHIVE_NAME) \
	-L$(UV_LIBDIR) $(UV_LIB)

## Installation
.PHONY: install
install: all
	{ test ! -f $(XARCHIVE) || extra="$(XARCHIVE) $(NAME).a"; }; \
	$(OCAMLFIND) install $(NAME) META $(NAME).cmi $(NAME).mli $(ARCHIVE) \
	dll$(CARCHIVE_NAME).so lib$(CARCHIVE_NAME).a $$extra

.PHONY: uninstall
uninstall:
	$(OCAMLFIND) remove $(NAME)

## Cleaning up
.PHONY: clean
clean::
	rm -f *~ *.cm* *.o *.a *.so depend

FORCE:

.SUFFIXES: .ml .mli .cmo .cmi .cmx

.mli.cmi:
	$(OCAMLC) -c $(COMPFLAGS) $<
.ml.cmo:
	$(OCAMLC) -c $(COMPLAGS) -nolabels $<
.ml.cmx:
	$(OCAMLOPT) -c $(COMPFLAGS) -nolabels $<
.c.o:
	$(OCAMLC) -c -ccopt "$(CFLAGS)" $<

include depend
