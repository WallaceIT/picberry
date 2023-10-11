#
# picberry Makefile
#
#
CC = $(CROSS_COMPILE)g++
CFLAGS = -Wall -O2 -s -std=c++11
TARGET = picberry
PREFIX = /usr
BINDIR = $(PREFIX)/bin
SRCDIR = src
BUILDDIR = build
MKDIR = mkdir -p
LDFLAGS =

DEVICES = $(BUILDDIR)/devices/dspic33e.o \
		  $(BUILDDIR)/devices/dspic33f.o \
		  $(BUILDDIR)/devices/pic10f322.o \
		  $(BUILDDIR)/devices/pic18fj.o \
		  $(BUILDDIR)/devices/pic24fjxxxga0xx.o \
		  $(BUILDDIR)/devices/pic24fjxxxga3xx.o \
		  $(BUILDDIR)/devices/pic24fjxxga1xx_gb0xx.o \
		  $(BUILDDIR)/devices/pic24fjxxxga1_gb1.o \
		  $(BUILDDIR)/devices/pic24fjxxxga2_gb2.o \
		  $(BUILDDIR)/devices/pic24fxxka1xx.o\
		  $(BUILDDIR)/devices/pic32.o $(BUILDDIR)/devices/pic32_pe.o

a10: CFLAGS += -DBOARD_A10
raspberrypi: CFLAGS += -DBOARD_RPI
raspberrypi2: CFLAGS += -DBOARD_RPI2
raspberrypi4: CFLAGS += -DBOARD_RPI4
am335x: CFLAGS += -DBOARD_AM335X
rk3399: CFLAGS += -DBOARD_RK3399
rk3399: LDFLAGS += -L/usr/lib -l:libwiringx.a

default:
	 @echo "Please specify a target with 'make raspberrypi', 'make a10', 'make am335x', or 'make rk3399'."

raspberrypi: prepare picberry
raspberrypi2: prepare picberry
raspberrypi4: prepare picberry
a10: prepare picberry
rk3399: prepare picberry
am335x: prepare picberry gpio_test

prepare:
	$(MKDIR) $(BUILDDIR)/devices

picberry:  $(BUILDDIR)/inhx.o $(DEVICES) $(BUILDDIR)/picberry.o
	$(CC) $(CFLAGS) -o $(TARGET) $(BUILDDIR)/inhx.o $(DEVICES) $(BUILDDIR)/picberry.o $(LDFLAGS)

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
