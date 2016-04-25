#
# picberry Makefile
# 
#
CC = g++
CFLAGS = -Wall -O2 -s -std=c++11
TARGET = picberry
PREFIX = /usr
BINDIR = $(PREFIX)/bin

GCCVERSION := $(shell expr `gcc -dumpversion | cut -f2 -d.` \>= 7)

ifeq "$(GCCVERSION)" "0"
    CC = g++-4.7
endif

a10: CFLAGS += -DBOARD_A10
raspberrypi: CFLAGS += -DBOARD_RPI
am335x: CFLAGS += -DBOARD_AM335X

default:
	 @echo "Please specify a target with 'make raspberrypi' or 'make a10'."

raspberrypi: picberry gpio_test
	
a10: picberry gpio_test

am335x: picberry gpio_test

picberry:  inhx.o dspic33f.o dspic33e.o pic18fj.o pic24fj.o picberry.o
	$(CC) $(CFLAGS) -o $(TARGET) inhx.o picberry.o dspic33f.o dspic33e.o pic18fj.o pic24fj.o
	
gpio_test:  gpio_test.o
	$(CC) $(CFLAGS) -o gpio_test gpio_test.o

inhx.o:  inhx.cpp common.h
	$(CC) $(CFLAGS) -c inhx.cpp

picberry.o:  picberry.cpp common.h dspic33f.h dspic33e.h pic18fj.h pic24fj.h
	$(CC) $(CFLAGS) -c picberry.cpp
	
gpio_test.o:  gpio_test.cpp common.h
	$(CC) $(CFLAGS) -c gpio_test.cpp

dspic33f.o:  dspic33f.cpp common.h dspic33f.h
	$(CC) $(CFLAGS) -c dspic33f.cpp

dspic33e.o:  dspic33e.cpp common.h dspic33e.h
	$(CC) $(CFLAGS) -c dspic33e.cpp

pic18fj.o: pic18fj.cpp common.h pic18fj.h
	$(CC) $(CFLAGS) -c pic18fj.cpp
	
pic24fj.o: pic24fj.cpp common.h pic24fj.h
	$(CC) $(CFLAGS) -c pic24fj.cpp

install:
	install -m 0755 $(TARGET) $(BINDIR)/$(TARGET)

uninstall:
	$(RM) $(BINDIR)/$(TARGET)

clean: 
	$(RM) $(TARGET) *.o
