prefix     = /usr/local
includedir = $(prefix)/include
libdir     = $(prefix)/lib
CC         = gcc
CFLAGS     = -Wall -O2 -I$(prefix)/include -L$(prefix)/lib -lencdec -lmba
OBJS       = t0encdec t1encdec t2encdec t3encdec


run: $(OBJS)
	-./t0encdec.sh
	-LANG=en_US.utf8 ./t1encdec ucs2be.html UCS-2BE > out && diff out utf8.html
	-./t2encdec
	-./t3encdec

.c.o:
	$(CC) $(CFLAGS) -c -o $* $<

java: T3Encdec.java
	javac -classpath ../src:../../../j/jcifs/build T3Encdec.java

clean:
	rm -f $(OBJS) out *.class
