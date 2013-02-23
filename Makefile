# Make file for cc_mux
CC = c++
LD = c++
CFLAGS = -fno-exceptions
LDFLAGS = -s
RM = /bin/rm -f
OBJS = cc_mux.o bits.o
PROG = cc_mux


all: cc_mux

cc_mux: cc_mux.o bits.o
	$(LD) $(LDFLAGS) -o $(PROG) $(OBJS)

cc_mux.o: cc_mux.cpp cc_mux.h
	$(CC) $(CFLAGS) -c cc_mux.cpp

bits.o: bits.cpp bits.h
	$(CC) $(CFLAGS) -c bits.cpp


clean:
	$(RM) $(OBJS)

