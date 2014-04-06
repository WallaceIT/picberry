#
# picberry Makefile
# 
#
CC = g++
CFLAGS  = -Wall -O2 -s -std=c++11
TARGET = picberry
PREFIX = /usr
BINDIR = $(PREFIX)/bin

GCCVERSION := $(shell expr `gcc -dumpversion | cut -f2 -d.` \>= 7)

ifeq "$(GCCVERSION)" "0"
    CC = g++-4.7
endif

a10: CFLAGS += -DBOARD_A10
raspberrypi: CFLAGS += -DBOARD_RPI

default:
	 @echo "Please specify a target with 'make raspberrypi' or 'make a10'."

raspberrypi: picberry
	
a10: picberry

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

install:
	install -m 0755 $(TARGET) $(BINDIR)/$(TARGET)

uninstall:
	$(RM) $(BINDIR)/$(TARGET)

clean: 
	$(RM) $(TARGET) *.o
