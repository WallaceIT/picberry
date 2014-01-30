Raspberry Pi/Allwinner A10 PIC Programmer using GPIO connector

Copyright 2014 Francesco Valla

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Overview

picberry is a Raspberry Pi/Allwinner A10 PIC programmer using GPIOs that doesn't require additional programming hardware.
It theorically supports dsPIC33F/PIC24H and PIC18FxxJxx families, but only dsPIC33FJ128GP802 has been tested.

# Building and Installing picberry

picberry is written in C++11, so it requires g++ version 4.7.

On Raspian/Debian install it entering:

	sudo apt-get install g++-4.7

To build picberry, after cloning the repository, launch

	make raspberrypi

for Raspberry Pi target or

	make a10
	
for A10 target.

Then launch
	
	sudo make install

to install it to /usr/bin.

To change destination prefix use PREFIX=, e.g.

	sudo make install PREFIX=/usr/local


Compilation is possible either directly on Raspberry Pi / A10 or in a cross-build environment, using g++ 4.7.

# Usage of picberry

	picberry [options]
       
Programming Options

	-h                print help
	-D                turn ON debug
	-g PGC,PGD,MCLR   GPIO selection, default if not present
	-f family	  	  PIC family (dspic or 18fj)
	-i file           input file
	-o file           output file (default: ofile.hex)

	-r                read chip
	-w                bulk erase and write chip
	-e                bulk erase chip
	-b                blank check of the chip
	-d                read configuration registers

Runtime Options

       -R                reset

For Example, to connect the PIC to GPIOs 11 (PGC), 9 (PGD), 22 (MCLR) and write on a dsPIC33FJ128GP802 the file fw.hex:

	picberry -w -g 11,9,22 -f dspic -i fw.hex

# Hardware

To use picberry you will need only the "recommended minimum connections" outlined in each PIC datasheet (avoiding the cap on MCLR).

Between PIC and the SoC you must have the four basic ICSP lines: PGC (clock), PGD (data), MCLR (Reset), GND.
You can also connect PIC VDD line to Raspberry Pi/Allwinner A10 3v3 line, but be careful: Raspberry Pi/Allwinner A10 3v3 pins have only 50mA of current capability, so consider your circuit current drawn!

If not specified in the command line, the default GPIOs <-> PIC connections for the Raspberry Pi are:

	PGC  <-> GPIO23
	PGD  <-> GPIO24
	MCLR <-> GPIO18
	
and for the Allwinner A10

	PGC  <-> PB15
	PGD  <-> PB17
	MCLR <-> PB12
	
PLEASE NOTE: picberry can use only pins on the same port on A10! Also, you can't set the port using a command line option (yet?).
	
# References

- [dsPIC33F/PIC24H Flash Programming Specification](http://ww1.microchip.com/downloads/en/DeviceDoc/70152H.pdf)
- [PIC18F2XJXX/4XJXX Family Programming Specification](http://ww1.microchip.com/downloads/en/DeviceDoc/39687e.pdf)

# Licensing

picberry is released under the GPLv3 license; for full license see the `LICENSE` file.

The Microchip name and logo, PIC, In-Circuit Serial Programming, ICSP are registered trademarks of Microchip Technology Incorporated in the U.S.A. and other countries.
Raspberry Pi is a trademark of The Raspberry Pi Foundation



