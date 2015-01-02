AG_VERSION=1.0

CC=gcc
COPTS=-O2
CFLAGS=-g $(COPTS) -Wall

INCLUDES = yaml/include

LIB_OBJS = sds.o agnostic.o

LIB_FILE=libagnostic.a

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

sds.o: sds.h

agnostic.o: agnostic.h sds.h

clean:
	rm -f *.o $(PROGRAMS) $(LIB_FILE)
	rm -rf *.dSYM .deps .libs
