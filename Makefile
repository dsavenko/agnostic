AG_VERSION=1.0

CC=gcc
COPTS=-O2
CFLAGS=-g $(COPTS) -Wall

INCLUDES = yaml/include

LIB_OBJS = agnostic.o common.o ag-clone.o ag-component.o ag-build.o

LIB_FILE = libagnostic.a

LIBS = $(LIB_FILE) -lyaml

PROGRAMS = \
	ag
	
all: $(PROGRAMS)

$(LIB_FILE): $(LIB_OBJS)
	$(AR) rcs $@ $(LIB_OBJS)

ag-%: %.c $(LIB_FILE)
	$(CC) $(CFLAGS) -I$(INCLUDES) -o $@ $(filter %.c,$^) $(LIBS)

ag: ag.c $(LIB_FILE)
	$(CC) $(CFLAGS) -I$(INCLUDES) -o $@ $(filter %.c,$^) $(LIBS)

agnostic.o: agnostic.h

common.o: common.h

ag-%.o: %.c agnostic.h common.h

clean:
	rm -f *.o $(PROGRAMS) $(LIB_FILE)
	rm -rf *.dSYM .deps .libs
