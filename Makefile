#
# picberry Makefile
# 
#
CC = g++
CFLAGS = -Wall -O2 -s -std=c++11
TARGET = picberry
PREFIX = /usr
BINDIR = $(PREFIX)/bin
SRCDIR = src
BUILDDIR = build
MKDIR = mkdir -p

DEVICES = $(BUILDDIR)/devices/dspic33e.o \
		  $(BUILDDIR)/devices/dspic33f.o \
		  $(BUILDDIR)/devices/pic18fj.o \
		  $(BUILDDIR)/devices/pic24fjxxxga0xx.o \
		  $(BUILDDIR)/devices/pic32.o $(BUILDDIR)/devices/pic32_pe.o

a10: CFLAGS += -DBOARD_A10
raspberrypi: CFLAGS += -DBOARD_RPI
am335x: CFLAGS += -DBOARD_AM335X

default:
	 @echo "Please specify a target with 'make raspberrypi', 'make a10' or 'make am335x'."

raspberrypi: prepare picberry
a10: prepare picberry
am335x: prepare picberry gpio_test

prepare:
	$(MKDIR) $(BUILDDIR)/devices

picberry:  $(BUILDDIR)/inhx.o $(DEVICES) $(BUILDDIR)/picberry.o  
	$(CC) $(CFLAGS) -o $(TARGET) $(BUILDDIR)/inhx.o $(DEVICES) $(BUILDDIR)/picberry.o

gpio_test:  $(BUILDDIR)/gpio_test.o
	$(CC) $(CFLAGS) -o gpio_test $(BUILDDIR)/gpio_test.o

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@
	
$(BUILDDIR)/devices/%.o: $(SRCDIR)/devices/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

install:
	install -m 0755 $(TARGET) $(BINDIR)/$(TARGET)

uninstall:
	$(RM) $(BINDIR)/$(TARGET)

clean: 
	$(RM) $(TARGET) *_test *.o $(BUILDDIR)/*.o $(BUILDDIR)/devices/*.o
