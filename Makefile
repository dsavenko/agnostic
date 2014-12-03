AG_VERSION=1.0

CC=gcc
COPTS=-O2
CFLAGS=-g $(COPTS) -Wall

INCLUDES = yaml/include
LIBS = -lyaml

PROGRAMS = \
	ag-info
	
all: $(PROGRAMS)

ag-%: %.c
	$(CC) $(CFLAGS) -I$(INCLUDES) -o $@ $(filter %.c,$^) $(LIBS)

clean:
	rm -f *.o $(PROGRAMS) 
	rm -rf *.dSYM .deps .libs
