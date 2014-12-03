AG_VERSION=1.0

CC=gcc
COPTS=-O2
CFLAGS=-g $(COPTS) -Wall

INCLUDES = yaml/include

LIB_OBJS = sds.o

LIB_FILE=libagnostic.a

LIBS = $(LIB_FILE) -lyaml

PROGRAMS = \
	ag-info
	
all: $(PROGRAMS)

$(LIB_FILE): $(LIB_OBJS)
	$(AR) rcs $@ $(LIB_OBJS)

ag-%: %.c $(LIB_FILE)
	$(CC) $(CFLAGS) -I$(INCLUDES) -o $@ $(filter %.c,$^) $(LIBS)

sds.o: sds.h

clean:
	rm -f *.o $(PROGRAMS) $(LIB_FILE)
	rm -rf *.dSYM .deps .libs
