AG_VERSION=1.0

CC=gcc
COPTS=-O2
#COPTS=-DDEBUG
CFLAGS=-g $(COPTS) -Wall

prefix=/usr/local
INSTALL=install

INCLUDES = yaml/include

LIB_OBJS = agnostic.o agnostic-loader.o common.o ag-clone.o ag-component.o ag-script.o ag-project.o

LIB_FILE = libagnostic.a

LIBS = $(LIB_FILE) -lyaml

PROGRAMS = ag
SCRIPTS = ag-remove.sh
ALL_PROGRAMS = $(PROGRAMS) $(SCRIPTS)
	
all: $(PROGRAMS)

$(LIB_FILE): $(LIB_OBJS)
	$(AR) rcs $@ $(LIB_OBJS)

ag-%: %.c $(LIB_FILE)
	$(CC) $(CFLAGS) -I$(INCLUDES) -o $@ $(filter %.c,$^) $(LIBS)

ag: ag.c $(LIB_FILE)
	$(CC) $(CFLAGS) -I$(INCLUDES) -o $@ $(filter %.c,$^) $(LIBS)

agnostic.o: agnostic.h agnostic.c common.h

agnostic-loader.o: agnostic.h agnostic-loader.c common.h

common.o: common.h

ag-%.o: %.c agnostic.h common.h

.PHONY: install clean uninstall

clean:
	rm -f *.o $(PROGRAMS) $(LIB_FILE)
	rm -rf *.dSYM .deps .libs

install: all
	$(INSTALL) -m 0755 $(ALL_PROGRAMS) $(prefix)/bin

install-doc:
	$(MAKE) -C docs/ install

uninstall:
	rm -f $(addprefix $(prefix)/bin/, $(ALL_PROGRAMS))

uninstall-doc:
	$(MAKE) -C docs/ uninstall
