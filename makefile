#
# picberry Makefile
# 
#
CC = g++-4.7
CFLAGS  = -Wall -O2 -s -std=c++11
TARGET = picberry
PREFIX = /usr
BINDIR = $(PREFIX)/bin

default: picberry

picberry:  inhx.o dspic.o pic18fj.o picberry.o
	$(CC) $(CFLAGS) -o $(TARGET) inhx.o picberry.o dspic.o pic18fj.o

inhx.o:  inhx.cpp common.h
	$(CC) $(CFLAGS) -c inhx.cpp

picberry.o:  picberry.cpp common.h dspic.h pic18fj.h
	$(CC) $(CFLAGS) -c picberry.cpp

dspic.o:  dspic.cpp common.h dspic.h
	$(CC) $(CFLAGS) -c dspic.cpp

pic18fj.o: pic18fj.cpp common.h pic18fj.h
	$(CC) $(CFLAGS) -c pic18fj.cpp

install: $(TARGET)
	install -m 0755 $(TARGET) $(BINDIR)/$(TARGET)

uninstall:
	$(RM) $(BINDIR)/$(TARGET)

clean: 
	$(RM) $(TARGET) *.o
