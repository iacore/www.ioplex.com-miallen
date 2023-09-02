prefix     = /usr/local
includedir = $(prefix)/include
libdir     = $(prefix)/lib
CFLAGS     = -g -Wall -W -DPROFILE -DMSGNO -I$(includedir) -L$(libdir) -ldomc -lmba -lencdec
OBJS       = dump.o e0.o e1.o e2.o e3.o d0.o d4.o

all: $(OBJS)

.c.o:
	#gcc $(CFLAGS) -lencdec -lmba -lexpat -ldomc -o $*.o $<
	gcc $(CFLAGS) -o $*.o $<

clean:
	rm -f $(OBJS)

