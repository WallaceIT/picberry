/*
 * Raspberry Pi PIC Programmer using GPIO connector
 * https://github.com/WallaceIT/picberry
 * Copyright 2014 Francesco Valla
 *
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

#include "common.h"
#include "pic18fj.h"

/* delays (in microseconds) */
#define DELAY_P1   	1
#define DELAY_P2   	1
#define DELAY_P2A  	1
#define DELAY_P2B  	1
#define DELAY_P3   	1
#define DELAY_P4   	1
#define DELAY_P5   	1
#define DELAY_P5A  	1
#define DELAY_P6   	1
#define DELAY_P9  	3400
#define DELAY_P10  	54000
#define DELAY_P11  	524000
#define DELAY_P12  	400
#define DELAY_P13  	1
#define DELAY_P14  	1
#define DELAY_P16  	1
#define DELAY_P17  	3
#define DELAY_P19	4000
#define DELAY_P20	1

/* commands for programming */
#define COMM_CORE_INSTRUCTION 				0x00
#define COMM_SHIFT_OUT_TABLAT 				0x02
#define COMM_TABLE_READ	     				0x08
#define COMM_TABLE_READ_POST_INC 			0x09
#define COMM_TABLE_READ_POST_DEC			0x0A
#define COMM_TABLE_READ_PRE_INC 			0x0B
#define COMM_TABLE_WRITE					0x0C
#define COMM_TABLE_WRITE_POST_INC_2			0x0D
#define COMM_TABLE_WRITE_STARTP_POST_INC_2	0x0E
#define COMM_TABLE_WRITE_STARTP				0x0F

#define ENTER_PROGRAM_KEY	0x4D434850

void pic18fj::enter_program_mode(void)
{
	int i;

	GPIO_IN(pic_mclr);
	GPIO_OUT(pic_mclr);

	GPIO_CLR(pic_mclr);			/* remove VDD from MCLR pin */
	delay_us(DELAY_P13);	/* wait P13 */
	GPIO_SET(pic_mclr);			/* apply VDD to MCLR pin */
	delay_us(10);		/* wait (no minimum time requirement) */
	GPIO_CLR(pic_mclr);			/* remove VDD from MCLR pin */
	delay_us(DELAY_P19);	/* wait P19 */

	GPIO_CLR(pic_clk);
	/* Shift in the "enter program mode" key sequence (MSB first) */
	for (i = 31; i > -1; i--) {
		if ( (ENTER_PROGRAM_KEY >> i) & 0x01 )
			GPIO_SET(pic_data);
		else
			GPIO_CLR(pic_data);
		delay_us(DELAY_P2B);	/* Setup time */
		GPIO_SET(pic_clk);
		delay_us(DELAY_P2A);	/* Hold time */
		GPIO_CLR(pic_clk);

	}
	GPIO_CLR(pic_data);
	delay_us(DELAY_P20);	/* Wait P20 */
	GPIO_SET(pic_mclr);			/* apply VDD to MCLR pin */
	delay_us(DELAY_P12);	/* Wait (at least) P12 */
}

void pic18fj::exit_program_mode(void)
{

	GPIO_CLR(pic_clk);			/* stop clock on PGC */
	GPIO_CLR(pic_data);			/* clear data pin PGD */
	delay_us(DELAY_P16);	/* wait P16 */
	GPIO_CLR(pic_mclr);			/* remove VDD from MCLR pin */
	delay_us(DELAY_P17);	/* wait (at least) P17 */
	GPIO_IN(pic_mclr);                      /* MCLR as input, puts the output driver in Hi-Z */
}

/* Send a 4-bit command to the PIC (LSB first) */
void pic18fj::send_cmd(uint8_t cmd)
{
	int i;

	for (i = 0; i < 4; i++) {
		GPIO_SET(pic_clk);
		if ( (cmd >> i) & 0x01 )
			GPIO_SET(pic_data);
		else
			GPIO_CLR(pic_data);
		delay_us(DELAY_P2B);	/* Setup time */
		GPIO_CLR(pic_clk);
		delay_us(DELAY_P2A);	/* Hold time */
	}
	GPIO_CLR(pic_data);
	delay_us(DELAY_P5);
}

/* Read 8-bit data from the PIC (LSB first) */
uint16_t pic18fj::read_data(void)
{
	uint8_t i;
	uint16_t data = 0x0000;

	for (i = 0; i < 8; i++) {
		GPIO_SET(pic_clk);
		delay_us(DELAY_P2B);
		GPIO_CLR(pic_clk);
		delay_us(DELAY_P2A);
	}

	delay_us(DELAY_P6);	/* wait for the data... */

	GPIO_IN(pic_data);

	for (i = 0; i < 8; i++) {
		GPIO_SET(pic_clk);
		delay_us(DELAY_P14);	/* Wait for data to be valid */
		data |= ( GPIO_LEV(pic_data) & 0x00000001 ) << i;
		delay_us(DELAY_P2B);
		GPIO_CLR(pic_clk);
		delay_us(DELAY_P2A);
	}

	delay_us(DELAY_P5A);
	GPIO_IN(pic_data);
	GPIO_OUT(pic_data);
	return data;
}

/* Load 16-bit data to the PIC (LSB first) */
void pic18fj::write_data(uint16_t data)
{
	int i;

	for (i = 0; i < 16; i++) {
		GPIO_SET(pic_clk);
		if ( (data >> i) & 0x0001 )
			GPIO_SET(pic_data);
		else
			GPIO_CLR(pic_data);
		delay_us(DELAY_P2B);	/* Setup time */
		GPIO_CLR(pic_clk);
		delay_us(DELAY_P2A);	/* Hold time */
	}
	GPIO_CLR(pic_data);
	delay_us(DELAY_P5A);
}

/* set Table Pointer */
void pic18fj::goto_mem_location(uint32_t data)
{

	data = data & 0x00FFFFFF;	/* set the MSB byte to zero (it should already be zero)	*/

	send_cmd(COMM_CORE_INSTRUCTION);
	write_data( 0x0E00 | ( (data >> 16) & 0x000000FF) );/* MOVLW Addr[21:16] */
	send_cmd(COMM_CORE_INSTRUCTION);
	write_data(0x6EF8);					/* MOVWF TBLPTRU */
	send_cmd(COMM_CORE_INSTRUCTION);
	write_data( 0x0E00 | ( (data >> 8) & 0x000000FF) );	/* MOVLW Addr[15:8] */
	send_cmd(COMM_CORE_INSTRUCTION);
	write_data(0x6EF7);					/* MOVWF TBLPTRH */
	send_cmd(COMM_CORE_INSTRUCTION);
	write_data( 0x0E00 | (data & 0x000000FF) );		/* MOVLW Addr[7:0] */
	send_cmd(COMM_CORE_INSTRUCTION);
	write_data(0x6EF6);					/* MOVWF TBLPTRL */
}

/* Read PIC device id word, located at 0x3FFFFE:0x3FFFFF */
bool pic18fj::read_device_id(void)
{
	uint16_t id;
	bool found = 0;

	goto_mem_location(0x3FFFFE);

	send_cmd(COMM_TABLE_READ_POST_INC);
	id = read_data();
	send_cmd(COMM_TABLE_READ_POST_INC);
	id = ( read_data() << 8) | (id & 0xE0) ;

	device_id = id;

	if(debug)
		fprintf(stderr,"Device ID: 0x%x\n", device_id );

	for (unsigned short i=0;i < sizeof(piclist)/sizeof(piclist[0]);i++){

		if (piclist[i].device_id == device_id){

			strcpy(name,piclist[i].name);
			mem.code_memory_size = piclist[i].code_memory_size;
			mem.program_memory_size = 0x0F80018;
			mem.location = (uint16_t*) calloc(mem.program_memory_size,sizeof(uint16_t));
			mem.filled = (bool*) calloc(mem.program_memory_size,sizeof(bool));
			cerr << name << " detected!" << endl;
			found = 1;
			break;
		}
	}

	if (found == 0){
		cerr << "Error: unknown/unsupported device or programmer not connected." << endl;
	}

	return found;
}

/* Bulk erase the chip */
void pic18fj::bulk_erase(void)
{

	cerr << "Performing a Bulk Erase...";
	goto_mem_location(0x3C0004);
	send_cmd(COMM_TABLE_WRITE);
	write_data(0x0180);
	send_cmd(COMM_CORE_INSTRUCTION);
	write_data(0x0000);                 /* NOP */
	send_cmd(COMM_CORE_INSTRUCTION);
	write_data(0x0000);                 /* NOP */
	GPIO_CLR(pic_data);	                /* Hold PGD low until erase completes. */
	delay_us(DELAY_P11);
	delay_us(DELAY_P10);
	cerr << "DONE!" << endl;

}

/* Read PIC memory and write the contents to a .hex file */
void pic18fj::read(char *outfile, uint32_t start, uint32_t count)
{
	uint16_t addr, data = 0x0000;

	cerr << "Reading chip..." << endl;

	/* Read Memory */

	goto_mem_location(0x000000);

	for (addr = 0; addr < mem.code_memory_size; addr++) {

		send_cmd(COMM_TABLE_READ_POST_INC);
		data = read_data();
		send_cmd(COMM_TABLE_READ_POST_INC);
		data = ( read_data() << 8 ) | (data & 0x00FF);

		if (debug) fprintf(stderr, "  addr = 0x%04X  data = 0x%04X\n", addr*2, data);

		if (data != 0xFFFF) {
			mem.location[addr]        = data;
			mem.filled[addr]      = 1;
		}
	}

	/* TODO: Checksum */

	write_inhx(&mem, outfile);
}

/* Bulk erase the chip, and then write contents of the .hex file to the PIC */
void pic18fj::write(char *infile)
{
	int i;
	uint16_t data;
	uint32_t addr = 0x00000000;

	read_inhx(infile, &mem);

	bulk_erase();

	cerr << "Writing chip";

	send_cmd(COMM_CORE_INSTRUCTION);
	write_data(0x84A6);			/* enable writes */

	for (addr = 0; addr < mem.code_memory_size; addr += 32){        /* address in WORDS (2 Bytes) */

		goto_mem_location(2*addr);
		if (debug) fprintf(stderr, "Go to address 0x%08X \n", addr);

		for(i=0; i<31; i++){		                        /* write the first 62 bytes */
			if (mem.filled[addr+i]) {
				if (debug) fprintf(stderr, "  Writing 0x%04X to address 0x%06X \n", mem.location[addr + i], (addr+i)*2 );
				send_cmd(COMM_TABLE_WRITE_POST_INC_2);
				write_data(mem.location[addr+i]);
			}
			else {
				if (debug) fprintf(stderr, "  Writing 0xFFFF to address 0x%06X \n", (addr+i)*2 );
				send_cmd(COMM_TABLE_WRITE_POST_INC_2);
				write_data(0xFFFF);			/* write 0xFFFF in empty locations */
			};
		}

		/* write the last 2 bytes and start programming */
		if (mem.filled[addr+31]) {
			if (debug) fprintf(stderr, "  Writing 0x%04X to address 0x%06X and then start programming...\n", mem.location[addr+31], (addr+31)*2);
			send_cmd(COMM_TABLE_WRITE_STARTP);
			write_data(mem.location[addr+31]);
		}
		else {
			if (debug) fprintf(stderr, "  Writing 0xFFFF to address 0x%06X and then start programming...\n", (addr+31)*2);
			send_cmd(COMM_TABLE_WRITE_STARTP);
			write_data(0xFFFF);			         /* write 0xFFFF in empty locations */
		};

		/* Programming Sequence */
		GPIO_CLR(pic_data);
		for (i = 0; i < 3; i++) {

		                GPIO_SET(pic_clk);
		                delay_us(DELAY_P2B);       /* Setup time */
		                GPIO_CLR(pic_clk);
		                delay_us(DELAY_P2A);       /* Hold time */
		        }
		GPIO_SET(pic_clk);
		delay_us(DELAY_P9);        /* Programming time */
		GPIO_CLR(pic_clk);
		delay_us(DELAY_P5);
		write_data(0x0000);
		/* end of Programming Sequence */
		if(!(addr%320))
			cerr << ".";
	};

	cerr << "DONE!" << endl;

	/* Verify Code Memory and Configuration Word */
	if(verify){
		cerr << "Verifying written data";

		goto_mem_location(0x000000);

		for (addr = 0; addr < mem.code_memory_size; addr++) {

			send_cmd(COMM_TABLE_READ_POST_INC);
			data = read_data();
			send_cmd(COMM_TABLE_READ_POST_INC);
			data = ( read_data() << 8 ) | ( data & 0xFF );

			if (debug) fprintf(stderr, "addr = 0x%06X:  pic = 0x%04X, file = 0x%04X\n", addr*2, data, (mem.filled[addr]) ? (mem.location[addr]) : 0xFFFF);

			if ( (data != mem.location[addr]) & ( mem.filled[addr]) ) {
				fprintf(stderr, "Error at addr = 0x%06X:  pic = 0x%04X, file = 0x%04X.\nExiting...", addr*2, data, mem.location[addr]);
				break;
			}
			if(!(addr%320))
				cerr << ".";
		}

		cerr << "DONE!" << endl;
	}
	else
		cerr << "Memory verification skipped." << endl;

}

/* Blank Check */
void pic18fj::blank_check(void)
{
	uint16_t addr, data;
	uint8_t blank = 1;

	cerr << "Performing Blank Check...";

	goto_mem_location(0x000000);

	for (addr = 0; addr < (mem.code_memory_size - 4); addr++) {

		send_cmd(COMM_TABLE_READ_POST_INC);
		data = read_data();
		send_cmd(COMM_TABLE_READ_POST_INC);
		data = (read_data() << 8) | (data & 0xFF) ;

		if (data != 0xFFFF) {
			fprintf(stderr, "Chip not Blank! Address: 0x%d, Read: 0x%x.\n",  addr*2, data);
			blank = 0;
			break;
		}
	}

	if (blank)
		cerr << "Blank Chip!" << endl;
	cerr << "DONE!" << endl;

}

/* Dum configuration words */
void pic18fj::dump_configuration_registers(void)
{

	cerr << "Configuration Words:" << endl;

	goto_mem_location(mem.code_memory_size - 4);

	for (int i=1; i<5; i++) {
		send_cmd(COMM_TABLE_READ_POST_INC);
		fprintf(stderr, " - CONFIG%dL = 0x%2x.\n", i,read_data());
		send_cmd(COMM_TABLE_READ_POST_INC);
		fprintf(stderr, " - CONFIG%dH = 0x%2x.\n", i,read_data());
	}

	cerr << endl;
}
