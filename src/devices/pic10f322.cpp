/*
 * Raspberry Pi PIC Programmer using GPIO connector
 * https://github.com/WallaceIT/picberry
 * Copyright 2014 Francesco Valla
 * Copyright 2017 Akimasa Tateba
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <iostream>

#include "pic10f322.h"

/* delays (in microseconds) */
#define DELAY_SETUP	1
#define	DELAY_HOLD	1
#define DELAY_TENTS	1
#define DELAY_TENTH	250
#define DELAY_TCKH	1
#define DELAY_TCKL 	1
#define DELAY_TCO 	1
#define DELAY_TDLY	1
#define DELAY_TERAB	5000
#define DELAY_TEXIT	1
#define DELAY_TPINT_DATA	2500
#define DELAY_TPINT_CONF	5000

/* commands for programming */
#define COMM_LOAD_CONFIG	0x00
#define COMM_LOAD_FOR_PROG	0x02
#define COMM_READ_FROM_PROG	0x04
#define COMM_INC_ADDR		0x06
#define COMM_RESET_ADDR		0x16
#define COMM_BEGIN_IN_TIMED_PROG	0x08
#define COMM_BULK_ERASE		0x09

#define ENTER_PROGRAM_KEY	0x4D434850

void pic10f322::enter_program_mode(void)
{
	int i;

	GPIO_IN(pic_mclr);
	GPIO_OUT(pic_mclr);

	GPIO_SET(pic_mclr);			/* apply VDD to MCLR pin */
	delay_us(DELAY_TENTS);	/* wait TENTS */
	GPIO_CLR(pic_mclr);			/* remove VDD from MCLR pin */
	GPIO_CLR(pic_clk);
	delay_us(DELAY_TENTH);		/* wait TENTH */
	/* Shift in the "enter program mode" key sequence (LSB! first) */
	for (i = 0; i < 32; i++) {
		if ( (ENTER_PROGRAM_KEY >> i) & 0x01 )
			GPIO_SET(pic_data);
		else
			GPIO_CLR(pic_data);

		delay_us(DELAY_TCKL);	/* Setup time */
		GPIO_SET(pic_clk);
		delay_us(DELAY_TCKH);	/* Hold time */
		GPIO_CLR(pic_clk);

	}
	GPIO_CLR(pic_data);

	//Last clock(Don't care data)
	delay_us(DELAY_TCKL);	/* Setup time */
	GPIO_SET(pic_clk);
	delay_us(DELAY_TCKH);	/* Hold time */
	GPIO_CLR(pic_clk);

}

void pic10f322::exit_program_mode(void)
{

	GPIO_CLR(pic_clk);			/* stop clock on PGC */
	GPIO_CLR(pic_data);			/* clear data pin PGD */

	GPIO_IN(pic_mclr);
}

/* Send a 4-bit command to the PIC (LSB first) */
void pic10f322::send_cmd(uint8_t cmd, unsigned int delay)
{
	int i;

	for (i = 0; i < 6; i++) {
		GPIO_SET(pic_clk);
		if ( (cmd >> i) & 0x01 )
			GPIO_SET(pic_data);
		else
			GPIO_CLR(pic_data);
		delay_us(DELAY_TCKH);	/* Setup time */
		GPIO_CLR(pic_clk);
		delay_us(DELAY_TCKL);	/* Hold time */
	}
	GPIO_CLR(pic_data);
	delay_us(delay);
}

/* Read 8-bit data from the PIC (LSB first) */
uint16_t pic10f322::read_data(void)
{
	uint8_t i;
	uint16_t data = 0x0000;

	GPIO_IN(pic_data);

	for (i = 0; i < 16; i++) {
		GPIO_SET(pic_clk);
		delay_us(DELAY_TCKH);
		delay_us(DELAY_TCO);	/* Wait for data to be valid */
		data |= ( GPIO_LEV(pic_data) & 0x00000001 ) << i;
		GPIO_CLR(pic_clk);
		delay_us(DELAY_TCKL);
	}

	GPIO_IN(pic_data);
	GPIO_OUT(pic_data);
	data >>= 1;
	return data;
}

/* Load 16-bit data to the PIC (LSB first) */
void pic10f322::write_data(uint16_t data)
{
	int i;
	data <<= 1;

	for (i = 0; i < 16; i++) {
		GPIO_SET(pic_clk);
		if ( (data >> i) & 0x0001 )
			GPIO_SET(pic_data);
		else
			GPIO_CLR(pic_data);
		delay_us(DELAY_SETUP);	/* Setup time */
		GPIO_CLR(pic_clk);
		delay_us(DELAY_HOLD);	/* Hold time */
	}
	GPIO_CLR(pic_data);
}

/* set Table Pointer */
void pic10f322::reset_mem_location(void)
{
	send_cmd(COMM_RESET_ADDR, DELAY_TDLY);
}

/* Read PIC device id word */
bool pic10f322::read_device_id(void)
{
	uint16_t id;
	bool found = 0, found2 = 0;

	send_cmd(COMM_LOAD_CONFIG, DELAY_TDLY);
	write_data(0x00);

	for(int i=0; i < 6; i++){
		send_cmd(COMM_INC_ADDR, DELAY_TDLY);
	}
	send_cmd(COMM_READ_FROM_PROG, DELAY_TDLY);
	id = read_data();
	device_id = (id >> 5) & 0x1ff;
	device_rev = id & 0x1f;

	for (unsigned short i=0;i < sizeof(piclist)/sizeof(piclist[0]);i++){

		if (piclist[i].device_id == device_id){

			strcpy(name,piclist[i].name);
			mem.code_memory_size = piclist[i].code_memory_size;
			mem.program_memory_size = 0x0F80018;
			mem.location = (uint16_t*) calloc(mem.program_memory_size,sizeof(uint16_t));
			mem.filled = (bool*) calloc(mem.program_memory_size,sizeof(bool));
			found = 1;
			break;
		}
	}
	for (unsigned short i=0;i < sizeof(detailed_subfamily_table)/sizeof(detailed_subfamily_table[0]);i++){

		if (detailed_subfamily_table[i].device_id == device_id){
			detailed_subfamily = detailed_subfamily_table[i].detailed_subfamily;
			latch_size = detailed_subfamily_table[i].latch_size;
			found2 = 1;
			break;
		}
	}
	return found & found2;

}

/* Blank Check */
uint8_t pic10f322::blank_check(void)
{
	unsigned int lcounter = 0;

	uint16_t addr, data;
	uint8_t ret = 0;

	if(!flags.debug) cerr << "[ 0%]";
	lcounter = 0;

	reset_mem_location();

	for(addr = 0; addr < mem.code_memory_size; addr++){
		send_cmd(COMM_READ_FROM_PROG, DELAY_TDLY);
		data = read_data() & 0x3FFF;
		send_cmd(COMM_INC_ADDR, DELAY_TDLY);

		if(data != 0x3FFF) {
			fprintf(stderr, "Chip not Blank! Address: 0x%x, Read: 0x%x.\n",  addr, data);
			ret = 1;
			break;
		}

		if(lcounter != addr*100/mem.code_memory_size){
			lcounter = addr*100/mem.code_memory_size;
			fprintf(stderr, "\b\b\b\b\b[%2d%%]", lcounter);
		}
	}

	/* Read Confuguration Fuses */
	send_cmd(COMM_LOAD_CONFIG, DELAY_TDLY);
	write_data(0x00);

	addr = 0x2000;
	if((detailed_subfamily == SF_PIC12F1822) || (detailed_subfamily == SF_PIC16LF1826))
		addr = 0x8000;
	for(int i = 0; i < 7; i++){
		send_cmd(COMM_INC_ADDR, DELAY_TDLY);
		addr++;
	}
	/* Config Word 1 */
	send_cmd(COMM_READ_FROM_PROG, DELAY_TDLY);

	data = read_data() & 0x3FFF;

	if (data != 0x3FFF) {
		fprintf(stderr, "Chip not Blank! Address: 0x%x, Read: 0x%x.\n",  addr, data);
		ret = 1;
	}
	/* Config Word 2 */
	if((detailed_subfamily == SF_PIC12F1822) || (detailed_subfamily == SF_PIC16LF1826)){
		addr++;
		send_cmd(COMM_INC_ADDR, DELAY_TDLY);
		send_cmd(COMM_READ_FROM_PROG, DELAY_TDLY);
		
		data = read_data() & 0x3FFF;

		if (data != 0x3FFF) {
			fprintf(stderr, "Chip not Blank! Address: 0x%x, Read: 0x%x.\n",  addr, data);
			ret = 1;
		}
	}

	if(!flags.debug) cerr << "\b\b\b\b\b";

	return ret;

}

/* Bulk erase the chip */
void pic10f322::bulk_erase(void)
{
	send_cmd(COMM_RESET_ADDR, DELAY_TDLY);
	send_cmd(COMM_BULK_ERASE, DELAY_TERAB);
	if(flags.client) fprintf(stdout, "@FIN");
}

/* Read PIC memory and write the contents to a .hex file */
void pic10f322::read(char *outfile, uint32_t start, uint32_t count)
{
	uint16_t addr, data = 0x0000;

	if(!flags.debug) cerr << "[ 0%]";
	if(flags.client) fprintf(stdout, "@000");
	unsigned int lcounter = 0;

	/* Read Memory */

	reset_mem_location();

	for (addr = 0; addr < mem.code_memory_size; addr++) {
		send_cmd(COMM_READ_FROM_PROG, DELAY_TDLY);
		data = read_data() & 0x3FFF;
		send_cmd(COMM_INC_ADDR, DELAY_TDLY);

		if (flags.debug)
			fprintf(stderr, "  addr = 0x%04X  data = 0x%04X\n", addr, data);

		if (data != 0x3FFF) {
			mem.location[addr]        = data;
			mem.filled[addr]      = 1;
		}

		if(lcounter != addr*100/mem.code_memory_size){
			if(flags.client)
				fprintf(stderr,"RED@%2d\n", (addr*100/mem.code_memory_size));
			if(!flags.debug)
				fprintf(stderr,"\b\b\b\b%2d%%]", addr*100/mem.code_memory_size);
			lcounter = addr*100/mem.code_memory_size;
		}
	}
	/* Read Confuguration Fuses */
	send_cmd(COMM_LOAD_CONFIG, DELAY_TDLY);
	write_data(0x00);

	addr = 0x2000;
	if((detailed_subfamily == SF_PIC12F1822) || (detailed_subfamily == SF_PIC16LF1826))
		addr = 0x8000;
	for(int i = 0; i < 7; i++){
		send_cmd(COMM_INC_ADDR, DELAY_TDLY);
		addr++;
	}
	/* Config Word 1 */
	send_cmd(COMM_READ_FROM_PROG, DELAY_TDLY);

	data = read_data() & 0x3FFF;

	if (flags.debug)
		fprintf(stderr, "  addr = 0x%04X  data = 0x%04X\n", addr, data);

	if (data != 0x3FFF) {
		mem.location[addr]        = data;
		mem.filled[addr]      = 1;
	}
	/* Config Word 2 */
	if((detailed_subfamily == SF_PIC12F1822) || (detailed_subfamily == SF_PIC16LF1826)){
		uint16_t mask = 0x3FFF;
		addr++;
		send_cmd(COMM_INC_ADDR, DELAY_TDLY);
		send_cmd(COMM_READ_FROM_PROG, DELAY_TDLY);
		
		if(detailed_subfamily == SF_PIC12F1822)
			mask = 0x3703;
		else if(detailed_subfamily == SF_PIC16LF1826)
			mask = 0x3713;
		data = read_data() & mask;

		if (flags.debug)
			fprintf(stderr, "  addr = 0x%04X  data = 0x%04X\n", addr, data);

		if (data != mask) {
			mem.location[addr]        = data;
			mem.filled[addr]      = 1;
		}
	}

	if(!flags.debug) cerr << "\b\b\b\b\b";
	if(flags.client) fprintf(stdout, "@FIN");
	write_inhx(&mem, outfile);
}

/* Bulk erase the chip, and then write contents of the .hex file to the PIC */
void pic10f322::write(char *infile)
{
	int i;
	uint16_t data, fileconf;
	uint32_t addr = 0x00000000;

	read_inhx(infile, &mem);

	bulk_erase();

	if(!flags.debug) cerr << "[ 0%]";
	if(flags.client) fprintf(stdout, "@000");
	unsigned int lcounter = 0;

	reset_mem_location();

	for (addr = 0; addr < mem.code_memory_size; addr += latch_size){        /* address in WORDS (2 Bytes) */


		if (flags.debug)
			fprintf(stderr, "Current address 0x%08X \n", addr);
		for(i=0; i<latch_size-1; i++){		                        /* write the first 62 bytes */
			if (mem.filled[addr+i]) {
				if (flags.debug)
					fprintf(stderr, "  Writing 0x%04X to address 0x%06X \n", mem.location[addr + i], (addr+i) );
				send_cmd(COMM_LOAD_FOR_PROG, DELAY_TDLY);
				write_data(mem.location[addr+i]);
			}
			else {
				if (flags.debug)
					fprintf(stderr, "  Writing 0x3FFF to address 0x%06X \n", (addr+i) );
				send_cmd(COMM_LOAD_FOR_PROG, DELAY_TDLY);
				write_data(0x3FFF);			/* write 0x3FFF in empty locations */
			};
			send_cmd(COMM_INC_ADDR, DELAY_TDLY);
		}

		/* write the last 2 bytes and start programming */
		if (mem.filled[addr+latch_size-1]) {
			if (flags.debug)
				fprintf(stderr, "  Writing 0x%04X to address 0x%06X and then start programming...\n", mem.location[addr+latch_size-1], (addr+latch_size-1));
			send_cmd(COMM_LOAD_FOR_PROG, DELAY_TDLY);
			write_data(mem.location[addr+latch_size-1]);
		}
		else {
			if (flags.debug)
				fprintf(stderr, "  Writing 0x3FFF to address 0x%06X and then start programming...\n", (addr+latch_size-1));
			send_cmd(COMM_LOAD_FOR_PROG, DELAY_TDLY);
			write_data(0x3FFF);			         /* write 0x3FFF in empty locations */
		};

		/* Programming Sequence */
		send_cmd(COMM_BEGIN_IN_TIMED_PROG, DELAY_TPINT_DATA);
		/* end of Programming Sequence */

		send_cmd(COMM_INC_ADDR, DELAY_TDLY);

		if(lcounter != addr*100/mem.code_memory_size){
			lcounter = addr*100/mem.code_memory_size;
			if(flags.client)
				fprintf(stdout,"@%03d", lcounter);
			if(!flags.debug)
				fprintf(stderr,"\b\b\b\b\b[%2d%%]", lcounter);
		}
	};

	if(!flags.debug) cerr << "\b\b\b\b\b\b";
	if(flags.client) fprintf(stdout, "@100");

	/* Write Confuguration Fuses
	 * Writing User ID is not implemented.
	 */
	send_cmd(COMM_LOAD_CONFIG, DELAY_TDLY);
	write_data(0x00);

	addr = 0x2000;
	if((detailed_subfamily == SF_PIC12F1822) || (detailed_subfamily == SF_PIC16LF1826))
		addr = 0x8000;
	for(i = 0; i < 7; i++){
		send_cmd(COMM_INC_ADDR, DELAY_TDLY);
		addr++;
	}
	if(mem.filled[addr]){
		send_cmd(COMM_LOAD_FOR_PROG, DELAY_TDLY);
		write_data(mem.location[addr]);

		send_cmd(COMM_BEGIN_IN_TIMED_PROG, DELAY_TPINT_CONF);
	}

	if((detailed_subfamily == SF_PIC12F1822) || (detailed_subfamily == SF_PIC16LF1826)){
		addr++;
		send_cmd(COMM_INC_ADDR, DELAY_TDLY);
		if(mem.filled[addr]){
			send_cmd(COMM_LOAD_FOR_PROG, DELAY_TDLY);
			write_data(mem.location[addr]);

			send_cmd(COMM_BEGIN_IN_TIMED_PROG, DELAY_TPINT_CONF);
		}
	}
	/* Verify Code Memory and Configuration Word */
	if(!flags.noverify){
		if(!flags.debug) cerr << "[ 0%]";
		if(flags.client) fprintf(stdout, "@000");
		lcounter = 0;

		reset_mem_location();

		for (addr = 0; addr < mem.code_memory_size; addr++) {
			send_cmd(COMM_READ_FROM_PROG, DELAY_TDLY);
			data = read_data() & 0x3FFF;
			send_cmd(COMM_INC_ADDR, DELAY_TDLY);

			if (flags.debug)
				fprintf(stderr, "addr = 0x%06X:  pic = 0x%04X, file = 0x%04X\n",
						addr, data, (mem.filled[addr]) ? (mem.location[addr]) : 0x3FFF);

			if ( (data != mem.location[addr]) & ( mem.filled[addr]) ) {
				fprintf(stderr, "Error at addr = 0x%06X:  pic = 0x%04X, file = 0x%04X.\nExiting...",
						addr, data, mem.location[addr]);
				return;
			}
			if(lcounter != addr*100/mem.code_memory_size){
				lcounter = addr*100/mem.code_memory_size;
				if(flags.client)
					fprintf(stdout,"@%03d", lcounter);
				if(!flags.debug)
					fprintf(stderr,"\b\b\b\b\b[%2d%%]", lcounter);
			}
		}

		/* Read Confuguration Fuses */
		send_cmd(COMM_LOAD_CONFIG, DELAY_TDLY);
		write_data(0x00);

		addr = 0x2000;
		if((detailed_subfamily == SF_PIC12F1822) || (detailed_subfamily == SF_PIC16LF1826))
			addr = 0x8000;
		for(int i = 0; i < 7; i++){
			send_cmd(COMM_INC_ADDR, DELAY_TDLY);
			addr++;
		}

		send_cmd(COMM_READ_FROM_PROG, DELAY_TDLY);

		/* NOTE: It is impossible to program LVP bit when Low-Voltage Programming.
		 * We will ignore LVP bit in Configuration Fuse by using 0x3EFF mask.
		 */
		uint16_t mask = 0x3FFF;
		if(detailed_subfamily == SF_PIC10F322)
			mask = 0x3EFF;

		data = read_data() & mask;
		fileconf = mem.location[addr] & mask;
		if ( ( data != fileconf ) & ( mem.filled[addr] ) ) {
			fprintf(stderr, "Error at addr = 0x%06X:  pic = 0x%04X, file = 0x%04X.\nExiting...",
					addr, data, mem.location[addr] & mask);
			return;
		}

		/* Config Word 2 */
		if((detailed_subfamily == SF_PIC12F1822) || (detailed_subfamily == SF_PIC16LF1826)){
			uint16_t mask = 0x3FFF;
			addr++;
			send_cmd(COMM_INC_ADDR, DELAY_TDLY);
			send_cmd(COMM_READ_FROM_PROG, DELAY_TDLY);
			
			if(detailed_subfamily == SF_PIC12F1822)
				mask = 0x3703;
			else if(detailed_subfamily == SF_PIC16LF1826)
				mask = 0x3713;
			/* Ignore LVP bit. */
			mask &= ~(1 << 13);
			data = read_data() & mask;
			fileconf = mem.location[addr] & mask;
			if ( ( data != fileconf ) & ( mem.filled[addr] ) ) {
				fprintf(stderr, "Error at addr = 0x%06X:  pic = 0x%04X, file = 0x%04X.\nExiting...",
						addr, data & mask, mem.location[addr] & mask);
				return;
			}
		}

		if(!flags.debug) cerr << "\b\b\b\b\b";
		if(flags.client) fprintf(stdout, "@FIN");
	}
	else{
		if(flags.client) fprintf(stdout, "@FIN");
	}

}

/* Dum configuration words */
void pic10f322::dump_configuration_registers(void)
{
	send_cmd(COMM_LOAD_CONFIG, DELAY_TDLY);
	write_data(0x00);

	for(int i=0; i < 7; i++){
		send_cmd(COMM_INC_ADDR, DELAY_TDLY);
	}
	send_cmd(COMM_READ_FROM_PROG, DELAY_TDLY);
	cout << "Configuration Words:" << endl;
	fprintf(stdout, " - CONFIG1 = 0x%2x.\n", (read_data() & 0x3FFF));
	if((detailed_subfamily == SF_PIC12F1822) || (detailed_subfamily == SF_PIC16LF1826)){
		send_cmd(COMM_INC_ADDR, DELAY_TDLY);
		send_cmd(COMM_READ_FROM_PROG, DELAY_TDLY);
		fprintf(stdout, " - CONFIG2 = 0x%2x.\n", (read_data() & 0x3FFF));
	}
}
