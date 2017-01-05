Raspberry Pi/Allwinner A10/TI AM335x PIC Programmer using GPIO connector

Copyright 2016 Francesco Valla

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Overview

picberry is a PIC programmer using GPIOs that doesn't require additional programming hardware, working on the [Raspberry Pi](https://www.raspberrypi.org/), Allwinner A10-based boards (such as the [Cubieboard](http://cubieboard.org/)) and TI AM335x-based boards (such as the [Beaglebone Black](https://beagleboard.org/black) or the [AM3359 ICEv2](http://www.ti.com/tool/tmdsice3359)).
It theorically supports dsPIC33E/PIC24E, dsPIC33F/PIC24H, PIC24FJ, PIC18FxxJxx, PIC32MX, PIC32MZ and (still not released) PIC32MK families, but only some PICs have been tested.

*NOTE on PIC32 support:* while software support is complete, device informations have been filled only for PIC32MX250F128B.
Users are encouraged to add devices (inside *pic32.h*) as needed.

# Building and Installing picberry

picberry is written using C++11 features, so it requires at least g++ version 4.7.

To build picberry, after cloning the repository, launch

	make raspberrypi

for Raspberry Pi v1 target or

	make raspberrypi2

for Raspberry Pi v2 or v3 target or

	make a10

for A10 target or

	make am335x
	
for AM335x target.

Then launch

	sudo make install

to install it to /usr/bin.

To change destination prefix use PREFIX=, e.g.

	sudo make install PREFIX=/usr/local


Compilation is possible either directly on target board or in a cross-build environment.

# Usage of picberry

	picberry [options]

Programming Options

	-h                print help
	-D                turn ON debug
	-g PGC,PGD,MCLR   GPIO selection in form PORT:NUM, default if not present
	-S port			  server mode, listening on given port                
	-f family		  PIC family (dspic33e, dspic33f, pic18fj, pic32mx1, pic32mx2, pic32mx3, pic32mz)
	-i file           input file
	-o file           output file (default: ofile.hex)

	-r                read chip
	-w                bulk erase and write chip
	-e                bulk erase chip
	-b                blank check of the chip
	-d                read configuration registers

Runtime Options

       -R                reset

For Example, to connect the PIC to RPi GPIOs 11 (PGC), 9 (PGD), 22 (MCLR) and write on a dsPIC33FJ128GP802 the file fw.hex:

	picberry -w -g 11,9,22 -f dspic33f -i fw.hex

To connect the PIC to A10 GPIOs B15 (PGC), B17 (PGD), I15 (MCLR):

	picberry -w -g B:15,B:17,I:15 -f dspic33f -i fw.hex

# Hardware

To use picberry you will need only the "recommended minimum connections" outlined in each PIC datasheet (avoiding the cap on MCLR).

Between PIC and the SoC you must have the four basic ICSP lines: PGC (clock), PGD (data), MCLR (Reset), GND.
You can also connect PIC VDD line to target board 3v3 line, but be careful: such pins normally have low current capabilities, so consider your circuit current drawn!

If not specified in the command line, the default GPIOs <-> PIC connections are the following:

for the Raspberry Pi:

	PGC  <-> GPIO23
	PGD  <-> GPIO24
	MCLR <-> GPIO18

for the Allwinner A10:

	PGC  <-> PB15
	PGD  <-> PB17
	MCLR <-> PB12
	
for AM335x:

	PGC  <-> GPIO12
	PGD  <-> GPIO13
	MCLR <-> GPIO40

Photos of the setup can be find in the [Wiki] (../../wiki/Setup Images).

# Remote GUI

Remote GUI is written in Qt5 and allows to control a picberry session running in *server mode* (that is, launched with the  `-S <port>` command line argument).

To compile it, just launch `qmake` and then `make` in the *remote_gui* folder.

# References

- [dsPIC33E/PIC24E Flash Programming Specification](http://ww1.microchip.com/downloads/en/DeviceDoc/70619B.pdf)
- [dsPIC33F/PIC24H Flash Programming Specification](http://ww1.microchip.com/downloads/en/DeviceDoc/70152H.pdf)
- [PIC24FJ Flash Programming Specification](http://ww1.microchip.com/downloads/en/DeviceDoc/30010057d.pdf)
- [PIC18F2XJXX/4XJXX Family Programming Specification](http://ww1.microchip.com/downloads/en/DeviceDoc/39687e.pdf)
- [PIC32 Flash Programming Specification](http://ww1.microchip.com/downloads/en/DeviceDoc/60001145S.pdf)

# Licensing

picberry is released under the GPLv3 license; for full license see the `LICENSE` file.

The Microchip name and logo, PIC, In-Circuit Serial Programming, ICSP are registered trademarks of Microchip Technology Incorporated in the U.S.A. and other countries.
Raspberry Pi is a trademark of The Raspberry Pi Foundation.
