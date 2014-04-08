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
#include <iostream>
#include <unistd.h>

#include "common.h"
#include "dspic.h"

/* delays (in microseconds; nanoseconds are rounded to 1us) */
#define DELAY_P1   		1		// 200ns
#define DELAY_P1A		1		// 80ns
#define DELAY_P1B		1		// 80ns
#define DELAY_P2		1		// 15ns
#define DELAY_P3		1		// 15ns
#define DELAY_P4		1		// 40ns
#define DELAY_P4A		1		// 40ns
#define DELAY_P5		1		// 20ns
#define DELAY_P6		1		// 100ns
#define DELAY_P7		25000	// 25ms
#define DELAY_P8		12		// 12us
#define DELAY_P9A		10		// 10us
#define DELAY_P9B		15		// 15us - 23us max!
#define DELAY_P10		1		// 400ns
#define DELAY_P11		330000	// 330ms
#define DELAY_P12		19500	// 19.5ms
#define DELAY_P13		1280	// 1.28ms
#define DELAY_P14		1		// 1us MAX!
#define DELAY_P15		1		// 10ns
#define DELAY_P16		0		// 0s
#define DELAY_P17   	0		// 0s - 100ns MAX!
#define DELAY_P18		1		// 1us
#define DELAY_P19		1		// 25ns
#define DELAY_P20		1		// 1us - 25ms MAX!
#define DELAY_P21		1		// 1us - 500us MAX!

#define ENTER_PROGRAM_KEY	0x4D434851

#define reset_pc() send_cmd(0x040200)
#define send_nop() send_cmd(0x000000)

unsigned int counter=0;
uint16_t nvmcon;

/* Send a 24-bit command to the PIC (LSB first) through a SIX instruction */
void dspic::send_cmd(uint32_t cmd)
{
	uint8_t i;

	GPIO_CLR(pic_data);

	/* send the SIX = 0x0000 instruction */
	for (i = 0; i < 4; i++) {
		GPIO_SET(pic_clk);
		delay_us(DELAY_P1B);
		GPIO_CLR(pic_clk);
		delay_us(DELAY_P1A);
	}

	delay_us(DELAY_P4);

	/* send the 24-bit command */
	for (i = 0; i < 24; i++) {
		if ( (cmd >> i) & 0x00000001 )
			GPIO_SET(pic_data);
		else
			GPIO_CLR(pic_data);
		delay_us(DELAY_P1A);
		GPIO_SET(pic_clk);
		delay_us(DELAY_P1B);
		GPIO_CLR(pic_clk);
	}

	delay_us(DELAY_P4A);

}

/* Read 16-bit data word from the PIC (LSB first) through a REGOUT inst */
uint16_t dspic::read_data(void)
{
	uint8_t i;
	uint16_t data = 0;

	GPIO_CLR(pic_data);
	GPIO_CLR(pic_clk);

	/* send the REGOUT=0x0001 instruction */
	for (i = 0; i < 4; i++) {
		if ( (0x0001 >> i) & 0x001 )
			GPIO_SET(pic_data);
		else
			GPIO_CLR(pic_data);
		delay_us(DELAY_P1A);
		GPIO_SET(pic_clk);
		delay_us(DELAY_P1B);
		GPIO_CLR(pic_clk);
	}

	delay_us(DELAY_P4);

	/* idle for 8 clock cycles, waiting for the data to be ready */
	for (i = 0; i < 8; i++) {
		GPIO_SET(pic_clk);
		delay_us(DELAY_P1B);
		GPIO_CLR(pic_clk);
		delay_us(DELAY_P1A);
	}

	delay_us(DELAY_P5);

	GPIO_IN(pic_data);

	/* read a 16-bit data word */
	for (i = 0; i < 16; i++) {
		GPIO_SET(pic_clk);
		delay_us(DELAY_P1B);
		data |= ( GPIO_LEV(pic_data) & 0x00000001 ) << i;
		GPIO_CLR(pic_clk);
		delay_us(DELAY_P1A);
	}

	delay_us(DELAY_P4A);
	GPIO_OUT(pic_data);
	return data;
}

/* enter program mode */
void dspic::enter_program_mode(void)
{
	int i;

	GPIO_IN(pic_mclr);
	GPIO_OUT(pic_mclr);

	GPIO_CLR(pic_clk);

	GPIO_CLR(pic_mclr);		/*  remove VDD from MCLR pin */
	delay_us(DELAY_P6);
	GPIO_SET(pic_mclr);		/*  apply VDD to MCLR pin */
	delay_us(DELAY_P21);
	GPIO_CLR(pic_mclr);		/* remove VDD from MCLR pin */
	delay_us(DELAY_P18);

	/* Shift in the "enter program mode" key sequence (MSB first) */
	for (i = 31; i > -1; i--) {
		if ( (ENTER_PROGRAM_KEY >> i) & 0x01 )
			GPIO_SET(pic_data);
		else
			GPIO_CLR(pic_data);
		delay_us(DELAY_P1A);
		GPIO_SET(pic_clk);
		delay_us(DELAY_P1B);
		GPIO_CLR(pic_clk);

	}
	GPIO_CLR(pic_data);
	delay_us(DELAY_P19);
	GPIO_SET(pic_mclr);
	delay_us(DELAY_P7);

	/* idle for 5 clock cycles */
	for (i = 0; i < 5; i++) {
		GPIO_SET(pic_clk);
		delay_us(DELAY_P1B);
		GPIO_CLR(pic_clk);
		delay_us(DELAY_P1A);
	}

}

/* exit program mode */
void dspic::exit_program_mode(void)
{
	GPIO_CLR(pic_clk);
	GPIO_CLR(pic_data);
	delay_us(DELAY_P16);
	GPIO_CLR(pic_mclr);		/* remove VDD from MCLR pin */
	delay_us(DELAY_P17);	/* wait (at least) P17 */
}

/* read the device ID and revision; returns only the id */
bool dspic::read_device_id(void)
{
	bool found = 0;

	reset_pc();
	reset_pc();
	send_nop();

	send_cmd(0x200FF0);
	send_cmd(0x880190);
	send_cmd(0xEB0300);
	send_cmd(0x207847);
	send_nop();

	send_cmd(0xBA0BB6);
	send_nop();
	send_nop();
	device_id = read_data();
	if(debug)
		fprintf(stdout,"Device ID: 0x%x\n", device_id );

	send_cmd(0xBA0BB6);
	send_nop();
	send_nop();
	if(debug)
		fprintf(stdout,"Revision: 0x%x\n", read_data());

	reset_pc();
	send_nop();

	for (unsigned short i=0;i < sizeof(piclist)/sizeof(piclist[0]);i++){

		if (piclist[i].device_id == device_id){

			strcpy(name,piclist[i].name);
			mem.code_memory_size = piclist[i].code_memory_size;
			mem.program_memory_size = 0x0F80018;
			mem.location = (uint16_t*) calloc(mem.program_memory_size,sizeof(uint16_t));
			mem.filled = (bool*) calloc(mem.program_memory_size,sizeof(bool));
			cout << name << " detected!" << endl << endl;
			if(client) cerr << "PRT@" << name << endl;
			found = 1;
			break;
		}
	}

	return found;

}

/* check if the device is blank */
void dspic::blank_check(void)
{
	uint32_t addr;
	unsigned short i;
	uint16_t data[8], raw_data[6];

	cout << endl << "Blank check...";
	if(!log && !debug) cout << "[ 0%]";

	if(client){
		cerr << "Blank check...";
	}

	counter=0;

	/* exit reset vector */
	reset_pc();
	reset_pc();
	send_nop();

	/* Output data to W0:W5; repeat until all desired code memory is read. */
	for(addr=0; addr < mem.code_memory_size; addr=addr+8) {

		if((addr & 0x0000FFFF) == 0){
			send_cmd(0x200000 | ((addr & 0x00FF0000) >> 12) );	// MOV #<DestAddress23:16>, W0
			send_cmd(0x880190);									// MOV W0, TBLPAG */
			send_cmd(0x200006 | ((addr & 0x0000FFFF) << 4) );	// MOV #<DestAddress15:0>, W6
		}

		/* Fetch the next four memory locations and put them to W0:W5 */
		send_cmd(0xEB0380);
		send_nop();
		send_cmd(0xBA1B96);
		send_nop();
		send_nop();
		send_cmd(0xBADBB6);
		send_nop();
		send_nop();
		send_cmd(0xBADBD6);
		send_nop();
		send_nop();
		send_cmd(0xBA1BB6);
		send_nop();
		send_nop();
		send_cmd(0xBA1B96);
		send_nop();
		send_nop();
		send_cmd(0xBADBB6);
		send_nop();
		send_nop();
		send_cmd(0xBADBD6);
		send_nop();
		send_nop();
		send_cmd(0xBA0BB6);
		send_nop();
		send_nop();

		/* read six data words (16 bits each) */
		for(i=0; i<6; i++){
			send_cmd(0x883C20 + i);
			send_nop();
			send_nop();
			raw_data[i] = read_data();
			send_nop();
		}

		reset_pc();
		send_nop();

		/* store data correctly */
		data[0] = raw_data[0];
		data[1] = raw_data[1] & 0x00FF;
		data[3] = (raw_data[1] & 0xFF00) >> 8;
		data[2] = raw_data[2];
		data[4] = raw_data[3];
		data[5] = raw_data[4] & 0x00FF;
		data[7] = (raw_data[4] & 0xFF00) >> 8;
		data[6] = raw_data[5];

		if(!log && counter != addr*100/mem.code_memory_size){
			fprintf(stdout, "\b\b\b\b\b[%2d%%]", addr*100/mem.code_memory_size);
			counter = addr*100/mem.code_memory_size;
		}

		for(i=0; i<8; i++)
			if ((i%2 == 0 && data[i] != 0xFFFF) || (i%2 == 1 && data[i] != 0x00FF)) {
				if(!log && !debug) cout << "\b\b\b\b\b";
				cout << "chip is not blank!" << endl;
				addr = mem.code_memory_size + 10;
				break;
			}
	}

	if(addr <= (mem.code_memory_size + 8)){
		if(!log && !debug) cout << "\b\b\b\b\b";
		cout << "chip is blank!" << endl;
	};

	reset_pc();
	reset_pc();
	send_nop();
}

/* Bulk erase the chip */
void dspic::bulk_erase(void)
{

    reset_pc();
    reset_pc();
    send_nop();

	cout << "Performing a Bulk Erase...";

	send_cmd(0x2404FA);
	send_cmd(0x883B0A);

	send_cmd(0xA8E761);
	send_nop();
	send_nop();
	send_nop();
	send_nop();

	/* wait while the erase operation completes */
	do{
		send_cmd(0x803B00);
		send_cmd(0x883C20);
		send_nop();
		nvmcon = read_data();
		reset_pc();
		send_nop();
	} while((nvmcon & 0x8000) == 0x8000);

	cout << "DONE!" << endl;
}

/* Read PIC memory and write the contents to a .hex file */
void dspic::read(char *outfile, uint32_t start, uint32_t count)
{
	uint32_t addr, startaddr, stopaddr;
	uint16_t data[8], raw_data[6];
	int i=0;

	startaddr = start;
	stopaddr = mem.code_memory_size;
	if(count != 0 && count < stopaddr){
		stopaddr = startaddr + count;
		fprintf(stdout, "Read only %d memory locations, from %06X to %06X\n",
				count, startaddr, stopaddr);
	}

	cout << endl <<"Reading chip...";
	if(!log && !debug) cout << "[ 0%]";
	if(client) cerr << "RED@00" << endl;
	counter=0;

	/* exit reset vector */
	reset_pc();
	reset_pc();
	send_nop();

	/* Output data to W0:W5; repeat until all desired code memory is read. */
	for(addr=startaddr; addr < stopaddr; addr=addr+8) {

		if((addr & 0x0000FFFF) == 0 || startaddr != 0){
			send_cmd(0x200000 | ((addr & 0x00FF0000) >> 12) );	// MOV #<DestAddress23:16>, W0
			send_cmd(0x880190);									// MOV W0, TBLPAG
			send_cmd(0x200006 | ((addr & 0x0000FFFF) << 4) );	// MOV #<DestAddress15:0>, W6
			startaddr = 0;
		}

		/* Fetch the next four memory locations and put them to W0:W5 */
		send_cmd(0xEB0380);
		send_nop();
		send_cmd(0xBA1B96);
		send_nop();
		send_nop();
		send_cmd(0xBADBB6);
		send_nop();
		send_nop();
		send_cmd(0xBADBD6);
		send_nop();
		send_nop();
		send_cmd(0xBA1BB6);
		send_nop();
		send_nop();
		send_cmd(0xBA1B96);
		send_nop();
		send_nop();
		send_cmd(0xBADBB6);
		send_nop();
		send_nop();
		send_cmd(0xBADBD6);
		send_nop();
		send_nop();
		send_cmd(0xBA0BB6);
		send_nop();
		send_nop();

		/* read six data words (16 bits each) */
		for(i=0; i<6; i++){
			send_cmd(0x883C20 + i);
			send_nop();
			send_nop();
			raw_data[i] = read_data();
			send_nop();
		}

		reset_pc();
		send_nop();

		/* store data correctly */
		data[0] = raw_data[0];
		data[1] = raw_data[1] & 0x00FF;
		data[3] = (raw_data[1] & 0xFF00) >> 8;
		data[2] = raw_data[2];
		data[4] = raw_data[3];
		data[5] = raw_data[4] & 0x00FF;
		data[7] = (raw_data[4] & 0xFF00) >> 8;
		data[6] = raw_data[5];

		for(i=0; i<8; i++){
			if (debug)
				fprintf(stdout, "\n addr = 0x%06X data = 0x%04X",
						(addr+i), data[i]);

			if (i%2 == 0 && data[i] != 0xFFFF) {
				mem.location[addr+i]        = data[i];
				mem.filled[addr+i]      = 1;
			}

			if (i%2 == 1 && data[i] != 0x00FF) {
				mem.location[addr+i]        = data[i];
				mem.filled[addr+i]      = 1;
			}
		}

		if(counter != addr*100/stopaddr){
			if(client && counter/10 != addr*10/stopaddr)
				fprintf(stderr,"RED@%2d\n", (addr*10/stopaddr)*10);
			if(!log)fprintf(stdout,"\b\b\b\b\b[%2d%%]", addr*100/stopaddr);

			counter = addr*100/stopaddr;
		}

		/* TODO: checksum */
	}

	reset_pc();
	reset_pc();
	send_nop();

	send_cmd(0x200F80);
	send_cmd(0x880190);
	send_cmd(0xEB0300);
	send_cmd(0x207847);
	send_nop();

	addr = 0x00F80000;

	for(i=0; i<12; i++){
		send_cmd(0xBA0BB6);
		send_nop();
		send_nop();
		data[0] = read_data();
		if (data[0] != 0xFFFF) {
			mem.location[addr+2*i]        = data[0];
			mem.filled[addr+2*i]      = 1;
		}
	}

	if(!log && !debug) cout << "\b\b\b\b\b";
	cout << "DONE! " << endl;
	if(client) cerr << "RED@100" << endl;
	write_inhx(&mem, outfile);
}

/* Write contents of the .hex file to the PIC */
void dspic::write(char *infile)
{
	uint8_t i,j,k,p;
	bool skip, skipped=0;
	uint32_t data[8],raw_data[6];
	uint32_t addr = 0;

	unsigned int filled_locations=1;

	const char *regname[] = {"FBS","FSS","FGS","FOSCSEL","FOSC","FWDT","FPOR",
								"FICD","FUID0","FUID1","FUID2","FUID3"};

	filled_locations = read_inhx(infile, &mem);
	if(!filled_locations) return;

	bulk_erase();

	/* Exit reset vector */
	reset_pc();
	reset_pc();
	send_nop();

	/* WRITE CODE MEMORY */
	cout << "Writing chip...";
	if(!log && !debug) cout << "[ 0%]";
	if(client) cerr << "WRT@00" << endl;
	counter=0;

	send_nop();
	send_cmd(0x24001A);
	send_cmd(0x883B0A);

	for (addr = 0; addr < mem.code_memory_size; ){

		skip = 1;

		for(k=0; k<128; k+=2)
			if(mem.filled[addr+k]) skip = 0;

		if(skip){
			addr=addr+128;
			continue;
		}

		send_cmd(0x200000 | ((addr & 0x00FF0000) >> 12) );
		send_cmd(0x880190);
		send_cmd(0x200007 | ((addr & 0x0000FFFF) << 4) );

		for(p=0; p<16; p++){

			for(j=0;j<8;j++){
				if (mem.filled[addr+j]) data[j] = mem.location[addr+j];
				else data[j] = 0xFFFF;
				if(debug)
					fprintf(stdout,"\n  Writing 0x%04X to address 0x%06X ", data[j], addr+j );
			}

			send_cmd(0x200000 | (data[0] << 4));										// MOV #<LSW0>, W0
			send_cmd(0x200001 | (0x00FFFF & ((data[3] << 8) | (data[1] & 0x00FF))) <<4);// MOV #<MSB1:MSB0>, W1
			send_cmd(0x200002 | (data[2] << 4));										// MOV #<LSW1>, W2
			send_cmd(0x200003 | (data[4] << 4));										// MOV #<LSW2>, W3
			send_cmd(0x200004 | (0x00FFFF & ((data[7] << 8) | (data[5] & 0x00FF))) <<4);// MOV #<MSB3:MSB2>, W4
			send_cmd(0x200005 | (data[6] << 4));										// MOV #<LSW3>, W5

			/* set_W6_and_load_latches */
			send_cmd(0xEB0300);
			send_nop();
			send_cmd(0xBB0BB6);
			send_nop();
			send_nop();
			send_cmd(0xBBDBB6);
			send_nop();
			send_nop();
			send_cmd(0xBBEBB6);
			send_nop();
			send_nop();
			send_cmd(0xBB1BB6);
			send_nop();
			send_nop();
			send_cmd(0xBB0BB6);
			send_nop();
			send_nop();
			send_cmd(0xBBDBB6);
			send_nop();
			send_nop();
			send_cmd(0xBBEBB6);
			send_nop();
			send_nop();
			send_cmd(0xBB1BB6);
			send_nop();
			send_nop();

			addr = addr+8;
		}

		send_cmd(0xA8E761);
		send_nop();
		send_nop();
		send_nop();
		send_nop();

		do{
			send_cmd(0x803B00);
			send_cmd(0x883C20);
			send_nop();
			nvmcon = read_data();
			reset_pc();
			send_nop();
		} while((nvmcon & 0x8000) == 0x8000);

		if(counter != addr*100/filled_locations){
			if(client && counter/10 != addr*10/filled_locations)
				fprintf(stderr,"WRT@%2d\n", (addr*10/filled_locations)*10);
			if(!log) fprintf(stdout,"\b\b\b\b\b[%2d%%]", addr*100/filled_locations);
			counter = addr*100/filled_locations;
		}
	};

	if(!log && !debug) cout << "\b\b\b\b\b\b";
	cout << "DONE! " << endl;
	if(client) cerr << "WRT@100" << endl;

	/* WRITE CONFIGURATION REGISTERS */

	cout << "Writing Configuration registers...";

	send_cmd(0x200007);
	send_cmd(0x24000A);
	send_cmd(0x883B0A);
	send_cmd(0x200F80);
	send_cmd(0x880190);

	addr = 0x00F80000;

	for(i=0; i<12; i++){

		if(mem.filled[addr+2*i]){

			send_cmd(0x200007 | (0x000FFFF0 & ((addr+2*i) << 4)));

			send_cmd(0x200000 | (mem.location[addr+2*i] << 4));
			send_cmd(0xBB1B80);
			send_nop();
			send_nop();

			send_cmd(0xA8E761);
			send_nop();
			send_nop();
			send_nop();
			send_nop();
			do{
				send_cmd(0x803B00);
				send_cmd(0x883C20);
				send_nop();
				nvmcon = read_data();
				reset_pc();
				send_nop();
			} while((nvmcon & 0x8000) == 0x8000);

			if(debug)
				fprintf(stdout,"\n - %s set to 0x%02x",
						regname[i], mem.location[addr+2*i]);
		}

	}
	if(debug) cout << endl;
	cout << "DONE!" << endl;

	/* VERIFY CODE MEMORY */
	if(verify){
		cout << "Verifying written memory locations...";
		if(!log && !debug) cout << "[ 0%]";
		if(client) cerr << "VRF@00" << endl;
		counter = 0;

		reset_pc();
		reset_pc();
		send_nop();

		for(addr=0; addr < mem.code_memory_size; addr=addr+8) {

			for(k=0; k<8; k+=2)
				if(mem.filled[addr+k]) skip = 0;
				else skip =1;

			if(((addr & 0x0000FFFF) == 0 || skipped) & !skip){
				send_cmd(0x200000 | ((addr & 0x00FF0000) >> 12) );	// MOV #<DestAddress23:16>, W0
				send_cmd(0x880190);									// MOV W0, TBLPAG
				send_cmd(0x200006 | ((addr & 0x0000FFFF) << 4) );	// MOV #<DestAddress15:0>, W6
			}

			if(skip){
				skipped=1;
				continue;
			}
			else skipped=0;

			/* Fetch the next four memory locations and put them to W0:W5 */
			send_cmd(0xEB0380);
			send_nop();
			send_cmd(0xBA1B96);
			send_nop();
			send_nop();
			send_cmd(0xBADBB6);
			send_nop();
			send_nop();
			send_cmd(0xBADBD6);
			send_nop();
			send_nop();
			send_cmd(0xBA1BB6);
			send_nop();
			send_nop();
			send_cmd(0xBA1B96);
			send_nop();
			send_nop();
			send_cmd(0xBADBB6);
			send_nop();
			send_nop();
			send_cmd(0xBADBD6);
			send_nop();
			send_nop();
			send_cmd(0xBA0BB6);
			send_nop();
			send_nop();

			/* read six data words (16 bits each) */
			for(i=0; i<6; i++){
				send_cmd(0x883C20 + i);
				send_nop();
				send_nop();
				raw_data[i] = read_data();
				send_nop();
			}

			reset_pc();
			send_nop();

			/* store data correctly */
			data[0] = raw_data[0];
			data[1] = raw_data[1] & 0x00FF;
			data[3] = (raw_data[1] & 0xFF00) >> 8;
			data[2] = raw_data[2];
			data[4] = raw_data[3];
			data[5] = raw_data[4] & 0x00FF;
			data[7] = (raw_data[4] & 0xFF00) >> 8;
			data[6] = raw_data[5];

			for(i=0; i<8; i++){
				if (debug)
					fprintf(stdout, "\n addr = 0x%06X data = 0x%04X",
								(addr+i), data[i]);

				if(mem.filled[addr+i] && data[i] != mem.location[addr+i]){
					fprintf(stdout,"\n\n ERROR at address %06X: written %04X but %04X read!\n\n",
									addr+i, mem.location[addr+i], data[i]);
					if(client)
						fprintf(stderr,"MSG@ERROR at address %06X: written %04X but %04X read!\n",
										addr+i, mem.location[addr+i], data[i]);
					return;
				}

			}

			if(counter != addr*100/filled_locations){
				if(client && counter/10 != addr*10/filled_locations)
					fprintf(stderr,"VRF@%2d\n", (addr*10/filled_locations)*10);
				if(!log) fprintf(stdout,"\b\b\b\b\b[%2d%%]", addr*100/filled_locations);
				counter = addr*100/filled_locations;
			}
		}

		if(!log && !debug) cout << "\b\b\b\b\b";
		cout << "DONE!" << endl;
		if(client) cerr << "VRF@100" << endl;
	}
	else{
		cout << "Memory verification skipped." << endl;
		if(client) cerr << "VRF@SKP" << endl;
	}

}

/* write to screen the configuration registers, without saving them anywhere */
void dspic::dump_configuration_registers(void)
{
	const char *regname[] = {"FBS","FSS","FGS","FOSCSEL","FOSC","FWDT","FPOR",
							"FICD","FUID0","FUID1","FUID2","FUID3"};

	cout << endl << "Configuration registers:" << endl << endl;

	reset_pc();
	reset_pc();
	send_nop();

	send_cmd(0x200F80);
	send_cmd(0x880190);
	send_cmd(0xEB0300);
	send_cmd(0x207847);
	send_nop();

	for(unsigned short i=0; i<12; i++){
		send_cmd(0xBA0BB6);
		send_nop();
		send_nop();
		fprintf(stdout," - %s: 0x%02x\n", regname[i], read_data());
		if(log && debug)
			fprintf(stdout," - %s: 0x%02x\n", regname[i], read_data());
	}

	cout << endl;

	reset_pc();
	send_nop();
}

