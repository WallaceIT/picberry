/*
 * PIC Programmer using GPIO connector - PIC32 devices
 * https://github.com/WallaceIT/picberry
 * Copyright 2016 Francesco Valla
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
#include <ctime>

#include "pic32.h"

/* delays (in microseconds) */
#define DELAY_P1	1
#define DELAY_P1A	1
#define DELAY_P1B	1
#define DELAY_P6	15
#define DELAY_P7	15
#define DELAY_P9A	40
#define DELAY_P9B	15
#define DELAY_P14	1
#define DELAY_P16	1
#define DELAY_P17	1
#define DELAY_P18	15
#define DELAY_P19	15
#define DELAY_P20	500

#define ENTER_PROGRAM_KEY	0x4D434850

#define ETAP_ADDRESS 	0x08
#define ETAP_DATA	0x09
#define ETAP_CONTROL	0x0A
#define ETAP_EJTAGBOOT  0x0C
#define ETAP_FASTDATA	0x0E

// MCHP TAP INSTRUCTIONS
#define MTAP_COMMAND	0x07
#define MTAP_SW_MTAP	0x04
#define MTAP_SW_ETAP	0x05
#define MTAP_IDCODE	0x01

// MTAP_COMMAND DR COMMANDS
#define MCHP_STATUS		0x00 // NOP and return Status.
#define MCHP_ASSERT_RST		0xD1 // Requests the reset controller to assert device Reset.
#define MCHP_DE_ASSERT_RST	0xD0 // Removes the request for device Reset.
#define MCHP_ERASE		0xFC // Cause the Flash controller to perform a Chip Erase.
#define MCHP_FLASH_ENABLE	0xFE // Enables fetches and loads to the Flash (from the processor).
#define MCHP_FLASH_DISABLE	0xFD // Disables fetches and loads to the Flash (from the processor).

#define PE_BASEADDR		0x00000900

#define PE_CMD_ROW_PROGRAM	0x00000000
#define PE_CMD_READ		0x00010000
#define PE_CMD_PROGRAM		0x00020000
#define PE_CMD_WORD_PROGRAM	0x00030000
#define PE_CMD_CHIP_ERASE	0x00040000
#define PE_CMD_PAGE_ERASE	0x00050000
#define PE_CMD_BLANK_CHECK	0x00060000
#define PE_CMD_EXEC_VERSION	0x00070000
#define PE_CMD_GET_CRC		0x00080000
#define PE_CMD_PROGRAM_CLUSTER	0x00090000
#define PE_CMD_GET_DEVICEID	0x000A0000
#define PE_CMD_CHANGE_CFG	0x000B0000
#define PE_CMD_GET_CHECKSUM	0x000C0000
#define PE_CMD_QUAD_WORD_PGRM	0x000D0000

#define PE_RESPONSE_CODE_PASS	0x00
#define PE_RESPONSE_CODE_FAIL	0x02
#define PE_RESPONSE_CODE_NACK	0x03

#define PROGRAM_FLASH_BASEADDR	0x1D000000
#define BOOTFLASH_OFFSET	0x02C00000
#define PROGRAM_AREA		0
#define BOOT_AREA		1

void pic32::enter_program_mode(void)
{
	int i;

	GPIO_IN(pic_mclr);
	GPIO_OUT(pic_mclr);

	GPIO_CLR(pic_mclr);	/* remove VDD from MCLR pin */
	delay_us(DELAY_P6);
	GPIO_SET(pic_mclr);	/* apply VDD to MCLR pin */
	delay_us(DELAY_P20);
	GPIO_CLR(pic_mclr);	/* remove VDD from MCLR pin */
	delay_us(DELAY_P18);

	GPIO_CLR(pic_clk);
	/* Shift in the "enter program mode" key sequence (MSB first) */
	for (i = 31; i > -1; i--) {
		if ((ENTER_PROGRAM_KEY >> i) & 0x01)
			GPIO_SET(pic_data);
		else
			GPIO_CLR(pic_data);
		delay_us(DELAY_P1A);	/* Setup time */
		GPIO_SET(pic_clk);
		delay_us(DELAY_P1B);	/* Hold time */
		GPIO_CLR(pic_clk);
	}

	GPIO_CLR(pic_data);
	delay_us(DELAY_P19);
	GPIO_SET(pic_mclr);
	delay_us(DELAY_P7);
}

void pic32::exit_program_mode(void)
{
	SetMode(5, 0b11111);
	GPIO_CLR(pic_clk);
	GPIO_CLR(pic_data);
	delay_us(DELAY_P7);
	GPIO_CLR(pic_mclr);
	delay_us(DELAY_P7);
	GPIO_SET(pic_mclr);
	GPIO_IN(pic_mclr);
}

/* PSEUDO OPERATIONS */
uint8_t pic32::Data4Phase(uint8_t tdi, uint8_t tms)
{
	uint8_t tdo;

	// data pin to output
	GPIO_OUT(pic_data);

	// write TDI - sampling is on the falling edge
	if (tdi & 0x01)
		GPIO_SET(pic_data);
	else
		GPIO_CLR(pic_data);

	delay_us(DELAY_P1B);

	GPIO_SET(pic_clk);
	delay_us(DELAY_P1B);
	GPIO_CLR(pic_clk);
	delay_us(DELAY_P1A);

	// write TMS - sampling is on the falling edge
	if (tms & 0x01)
		GPIO_SET(pic_data);
	else
		GPIO_CLR(pic_data);

	delay_us(DELAY_P1B);

	GPIO_SET(pic_clk);
	delay_us(DELAY_P1B);
	GPIO_CLR(pic_clk);
	delay_us(DELAY_P1A);

	// data pin to input
	GPIO_CLR(pic_data);
	GPIO_IN(pic_data);
	delay_us(DELAY_P1B);

	// "empty" clock pulse
	GPIO_SET(pic_clk);
	delay_us(DELAY_P1B);
	GPIO_CLR(pic_clk);
	delay_us(DELAY_P1A);

	// read TDO, sampling on the rising edge
	GPIO_SET(pic_clk);
	tdo = GPIO_LEV(pic_data);
	delay_us(DELAY_P1B);
	GPIO_CLR(pic_clk);
	delay_us(DELAY_P1A);

	return (tdo & 0x01);
}

void pic32::Data2Phase(uint8_t tdi, uint8_t tms)
{
	// data pin to output
	GPIO_OUT(pic_data);

	// write TDI - sampling is on the falling edge
	if (tdi & 0x01)
		GPIO_SET(pic_data);
	else
		GPIO_CLR(pic_data);

	GPIO_SET(pic_clk);
	delay_us(DELAY_P1B);
	GPIO_CLR(pic_clk);
	delay_us(DELAY_P1A);

	// write TMS - sampling is on the falling edge
	if (tms & 0x01)
		GPIO_SET(pic_data);
	else
		GPIO_CLR(pic_data);

	GPIO_SET(pic_clk);
	delay_us(DELAY_P1B);
	GPIO_CLR(pic_clk);
	delay_us(DELAY_P1A);
}

void pic32::SetMode(uint8_t length, uint8_t mode)
{
	for (int i=0; i < length; i++)
		Data4Phase(0, (mode >> i));
}

void pic32::SendCommand(uint8_t command)
{
	int i;

	// TMS header 1100 (TDI set to 0)
    	Data4Phase(0, 1);
	Data4Phase(0, 1);
	Data4Phase(0, 0);
	Data4Phase(0, 0);

	for (i=0; i < 4; i++)
		Data4Phase((command >> i), 0);

	// Command MSb with TMS=1
	Data4Phase((command >> i), 1);

	// TMS footer 10 (TDI set to 0)
    	Data4Phase(0, 1);
	Data4Phase(0, 0);
}

uint32_t pic32::XferData(uint8_t length, uint32_t iData)
{
	int i;
	uint32_t oData;

	// TMS header 100 (TDI set to 0)
	Data4Phase(0, 1);
	Data4Phase(0, 0);
	oData = Data4Phase(0, 0);

	// iData, LSb first, with TMS=0
	for (i=0; i < length-1; i++)
		oData |= Data4Phase((iData >> i), 0) << (i+1);

	// iData MSb with TMS=1
	Data4Phase((iData >> i), 1);

	// TMS footer 10 (TDI set to 0)
	Data4Phase(0, 1);
	Data4Phase(0, 0);

	return oData;
}

void pic32::XferFastData2P(uint32_t iData)
{
	uint8_t i;

	// TMS header 100 (TDI set to 0)
	Data2Phase(0, 1);
	Data2Phase(0, 0);
	Data2Phase(0, 0);

	// prAcc
	Data2Phase(0, 0);

	// iData, LSb first, with TMS=0
	for (i=0; i < 31; i++)
		Data2Phase((iData >> i), 0);

	// iData MSb with TMS=1
	Data2Phase((iData >> i), 1);

	// TMS footer 10 (TDI set to 0)
    	Data2Phase(0, 1);
	Data2Phase(0, 0);
}

uint32_t pic32::XferFastData4P(uint32_t iData)
{
	uint8_t i = 0;
	uint32_t oData = 0;

	do {
		// TMS header 100 (TDI set to 0)
		Data4Phase(0, 1);
		Data4Phase(0, 0);
		i = Data4Phase(0, 0);
	} while (!i);

	// prAcc
	oData |= Data4Phase(0, 0);

	// iData, LSb first, with TMS=0
	for (i=0; i < 31; i++)
		oData |= (Data4Phase((iData >> i), 0) << (i + 1));

	// iData MSb with TMS=1
	Data4Phase((iData >> i), 1);

	// TMS footer 10 (TDI set to 0)
    	Data4Phase(0, 1);
	Data4Phase(0, 0);

	return oData;
}

void pic32::XferInstruction(uint32_t instruction)
{
	uint32_t controlVal;
	// Select Control Register
	SendCommand(ETAP_CONTROL);
	// Wait until CPU is ready
	// Check if Processor Access bit (bit 18) is set
	do {
		controlVal = XferData(32, 0x0004C000);
	} while (!((controlVal >> 18) & 0x01));
	// Select Data Register
	SendCommand(ETAP_DATA);
	// Send the instruction
	XferData(32, instruction);
	// Tell CPU to execute instruction
	SendCommand(ETAP_CONTROL);
	XferData(32, 0x0000C000);
}

uint32_t pic32::ReadFromAddress(uint32_t address)
{
	uint32_t instruction, oData;

	// Load Fast Data register address to s3
	instruction = 0x3c130000;
	instruction |= (0xff200000>>16) & 0x0000ffff;
	XferInstruction(instruction); // lui s3, <FAST_DATA_REG_ADDRESS(31:16)> - set address of fast data register
	// Load memory address to be read into t0
	instruction = 0x3c080000;
	instruction |= (address >> 16) & 0x0000ffff;
	XferInstruction(instruction); // lui t0, <DATA_ADDRESS(31:16)> - set address of data
	instruction = 0x35080000;
	instruction |= (address & 0x0000ffff);
	XferInstruction(instruction); // ori t0, <DATA_ADDRESS(15:0)> - set address of data
	// Read data
	XferInstruction(0x8d090000); // lw t1, 0(t0)
	// Store data into Fast Data register
	XferInstruction(0xae690000); // sw t1, 0(s3) - store data to fast data register
	XferInstruction(0); // nop
	// Shift out the data
	SendCommand(ETAP_FASTDATA);
	oData = XferFastData4P(0x00000000);
	return oData;
}

uint32_t pic32::GetPEResponse(void)
{
	uint32_t response;

	// Wait until CPU is ready
	SendCommand(ETAP_CONTROL);

	// Check if Processor Access bit (bit 18) is set
	do {
		response = XferData(32, 0x0004c000);
	} while (!((response >> 18) & 0x01));

	// Select Data Register
	SendCommand(ETAP_DATA);
	// Receive Response
	response = XferData(32, 0);
	// Tell CPU to execute instruction
	SendCommand(ETAP_CONTROL);
	XferData(32, 0x0000c000);
	// return 32-bit response
	return response;
}

bool pic32::check_device_status(void)
{
	uint32_t statusVal = 0;
	clock_t start;
	bool timeout_avoided = true;

	SetMode(6, 0b011111);
	SendCommand(MTAP_SW_MTAP);
	SendCommand(MTAP_COMMAND);

	start = clock();
	do {
		statusVal = XferData(8, MCHP_STATUS);
		if ((clock() - start) / (double) CLOCKS_PER_SEC > 0.01)
			timeout_avoided = false;
	} while (timeout_avoided && ((statusVal & 0x0C) != 0x08));

	return timeout_avoided;
}

void pic32::code_protected_bulk_erase(void)
{
	uint32_t statusVal = 0;

	SendCommand(MTAP_SW_MTAP);
	SendCommand(MTAP_COMMAND);
	XferData(8, MCHP_ERASE);
	if (!(subfamily == SF_PIC32MX1 || subfamily == SF_PIC32MX2 || subfamily == SF_PIC32MX3))
		XferData(8, MCHP_DE_ASSERT_RST);
	delay_us(10000);
	do {
		statusVal = XferData(8, MCHP_STATUS);
	} while ((statusVal & 0x0C) != 0x08);
	if (flags.client) fprintf(stdout, "@FIN");
}

bool pic32::enter_serial_exec_mode(void) {
	uint32_t statusVal = 0;

	SendCommand(MTAP_SW_MTAP);
	SendCommand(MTAP_COMMAND);
	statusVal = XferData(8, MCHP_STATUS);
	if (!((statusVal >> 7) & 0x01))
		return false;	// the device must be erased first.
	XferData(8, MCHP_ASSERT_RST);
	SendCommand(MTAP_SW_ETAP);
	SendCommand(ETAP_EJTAGBOOT);
	SendCommand(MTAP_SW_MTAP);
	SendCommand(MTAP_COMMAND);
	XferData(8, MCHP_DE_ASSERT_RST);
	if (subfamily == SF_PIC32MX1 || subfamily == SF_PIC32MX2 || subfamily == SF_PIC32MX3)
		XferData(8, MCHP_FLASH_ENABLE);
	SendCommand(MTAP_SW_ETAP);

	delay_us(1000);

	return true;
}

void pic32::download_pe(vector<uint32_t> pe_pointer)
{

	uint32_t i;

	if (subfamily == SF_PIC32MX1 || subfamily == SF_PIC32MX2 || subfamily == SF_PIC32MX3) {
		// PIC32MX devices only: Initialize BMXCON to 0x1F0040
		XferInstruction(0x3c04bf88);
		XferInstruction(0x34842000);
		XferInstruction(0x3c05001f);
		XferInstruction(0x34a50040);
		XferInstruction(0xac850000);
		// PIC32MX devices only: Initialize BMXDKPBA to 0x800.
		XferInstruction(0x34050800);
		XferInstruction(0xac850010);
		// PIC32MX devices only: Initialize BMXDUDBA and BMXDUPBA to the value of BMXDRMSZ.
		XferInstruction(0x8c850040);
		XferInstruction(0xac850020);
		XferInstruction(0xac850030);
	}

	// Set up PIC32 RAM address for PE.
	XferInstruction(0x3c04a000);
	XferInstruction(0x34840800);

	// Load the PE_Loader
	for (i=0;i<pe_loader.size();i++) {
		XferInstruction(0x3c060000 | (pe_loader[i] >> 16));
		XferInstruction(0x34c60000 | (pe_loader[i] & 0x0000ffff));
		XferInstruction(0xac860000);
		XferInstruction(0x24840004);
	};

	// Jump to the PE_Loader
	XferInstruction(0x3c19a000);
	XferInstruction(0x37390800);
	XferInstruction(0x03200008);
	XferInstruction(0x00000000);

	// Load the PE using the PE_Loader.
	uint32_t pe_size = pe_pointer.size();

	SendCommand(ETAP_FASTDATA);

	XferFastData4P(PE_BASEADDR); 	// Address of PE program block
	XferFastData4P(pe_size); // Number of 32-bit words of the program block from PE Hex file
	for (i=0; i<pe_size; i++) {
		XferFastData4P(pe_pointer[i]); // PE software op code from PE Hex file (PE Instructions)
	}

	// Jump to PE
	XferFastData4P(0x00000000);
	XferFastData4P(0xdead0000);

	XferFastData4P(PE_CMD_EXEC_VERSION);
	GetPEResponse(); /* CHECKME: why removed? */
}

bool pic32::setup_pe(void)
{

	if (!check_device_status()) {
		cerr << "Timeout occurred checking device status!" << endl;
		return false;
	}

	if (!enter_serial_exec_mode()) {
		cerr << "Error entering serial exec mode!" << endl;
		return false;
	}

	switch (subfamily) {
		case SF_PIC32MX1:
		case SF_PIC32MX2:
			download_pe(pic32_pemx1);
			break;
		case SF_PIC32MX3:
			download_pe(pic32_pemx3);
			break;
		case SF_PIC32MZ:
		case SF_PIC32MK:
			download_pe(pic32_pemz);
			break;
		default:
			return false;
	}

	return true;
}

bool pic32::read_device_id(void)
{
	uint32_t rxp;

	bool found = false;

	SendCommand(ETAP_FASTDATA);
	XferFastData4P(PE_CMD_READ | 0x01);

	switch (subfamily) {
		case SF_PIC32MX1:
		case SF_PIC32MX2:
		case SF_PIC32MX3:
			XferFastData4P(0x1F80F220);
			break;
		case SF_PIC32MZ:
		case SF_PIC32MK:
			XferFastData4P(0x1F800020);
			break;
		default:
			XferFastData4P(0x1F80F220);
			break;
	}

	GetPEResponse();
	rxp = GetPEResponse();
	device_id = (rxp & 0x0FFFFFFF);
	device_rev = (uint16_t)(rxp >> 28);

	for (unsigned short i=0;i < sizeof(piclist)/sizeof(piclist[0]);i++) {

		if (piclist[i].device_id == device_id) {
			strcpy(name, piclist[i].name);
			mem.code_memory_size = piclist[i].code_memory_size;
			mem.program_memory_size = 0x03000000;
			mem.location = (uint16_t*) calloc(mem.program_memory_size,sizeof(uint16_t));
			mem.filled = (bool*) calloc(mem.program_memory_size,sizeof(bool));
			found = true;
			break;
		}
	}

	switch (subfamily) {
		case SF_PIC32MX1:
		case SF_PIC32MX2:
			rowsize  = 128;
			bootsize = 0x00000C00;
			break;
		case SF_PIC32MX3:
			rowsize  = 512;
			bootsize = 0x00003000;
			break;
		case SF_PIC32MK:
			rowsize  = 2048;
			bootsize = 0x00005000;
			break;
		case SF_PIC32MZ:
			rowsize  = 2048;
			bootsize = 0x00014000;
			break;
		default:
			rowsize  = 128;
			bootsize = 0x00000C00;
			break;
	}

	return found;
}

void pic32::bulk_erase(void)
{
	uint32_t rxp;

	SendCommand(ETAP_FASTDATA);
	XferFastData4P(PE_CMD_CHIP_ERASE);
	rxp = GetPEResponse();
	if (rxp!=PE_CMD_CHIP_ERASE)
		fprintf(stderr, "___ERR(erase)___ %08x\n", rxp);

	if (flags.client) fprintf(stdout, "@FIN");
}

uint8_t pic32::blank_check(void)
{
	uint32_t rxp = 0;
	SendCommand(ETAP_FASTDATA);
	XferFastData4P(PE_CMD_BLANK_CHECK);
	XferFastData4P(PROGRAM_FLASH_BASEADDR);
	XferFastData4P(mem.code_memory_size * 2);
	rxp = GetPEResponse();
	return (rxp == PE_CMD_BLANK_CHECK) ? 0 : 1;
};

void pic32::read(char *outfile, uint32_t start, uint32_t count)
{
	uint32_t rxp = 0;
	uint32_t blocksize = 0;	// expressed in bytes
	const uint32_t max_blocksize = 0x0000FFFF*4;
	const uint32_t programsize = mem.code_memory_size*2;
	uint32_t counter = 0, read_locations = 0, i = 0;
	uint8_t area = PROGRAM_AREA;
	uint32_t addr = 0, startaddr = 0, stopaddr = 0, total_to_read = 0;

	if(!flags.debug) cerr << "[ 0%]";
	if(flags.client) fprintf(stdout, "@000");

	if (!flags.program_only)
		total_to_read += bootsize;
	if (!flags.boot_only)
		total_to_read += programsize;

	do {
		switch (area) {
			case PROGRAM_AREA:	// Read Program Flash (0x1D000000 to 0x1D000000+CodeMem)
				startaddr = 0;
				stopaddr = programsize;
				blocksize = max_blocksize;
				if(programsize < max_blocksize)
					blocksize = programsize;
				break;
			case BOOT_AREA:	// Read bootflash+configuration
				startaddr = BOOTFLASH_OFFSET;
				blocksize = bootsize;
				stopaddr = startaddr + blocksize;
				break;
			default:
				break;
		}

		if (((area == PROGRAM_AREA) & !flags.boot_only) ||
		    ((area == BOOT_AREA) & !flags.program_only)) {

			// addr is espressed in BYTES
			uint32_t cur_blocksize;
			for (addr = startaddr; addr < stopaddr; addr += cur_blocksize){
				cur_blocksize = std::min(stopaddr - addr, blocksize);

				SendCommand(ETAP_FASTDATA);
				XferFastData4P(PE_CMD_READ | (cur_blocksize / 4));
				XferFastData4P(PROGRAM_FLASH_BASEADDR + addr);

				rxp = GetPEResponse();
				if (rxp != PE_CMD_READ)
					fprintf(stderr, "___ERR(read)___: %08x\n", rxp);

				// i is expressed in BYTES
				for(i = 0; i < cur_blocksize; i += 4){
					int word_addr = (addr + i) / 2;
					rxp = GetPEResponse();
					if (flags.fulldump || (rxp != 0xFFFFFFFF)) {
						mem.location[word_addr] = rxp & 0x0000FFFF;
						mem.filled[word_addr] = 1;
						mem.location[word_addr+1] = rxp >> 16;
						mem.filled[word_addr+1] = 1;
					}

					read_locations += 4;

					uint32_t cur_counter = read_locations * 100 / total_to_read;
					if (counter != cur_counter) {
						counter = cur_counter;
						if (flags.client)
							fprintf(stdout,"@%03d", counter);
						if (!flags.debug)
							fprintf(stderr,"\b\b\b\b\b[%2d%%]", counter);
					}
				}
			}
		}
		area++;
	} while (area <= BOOT_AREA);

	cerr << "\b\b\b\b\b";
	if (flags.client) fprintf(stdout, "@FIN");
	write_inhx(&mem, outfile, PROGRAM_FLASH_BASEADDR);
};

void pic32::write(char *infile)
{
	uint32_t rxp = 0;
	uint32_t addr = 0, chk_size = 0;
	uint32_t filled_locations = 0, programmed_locations = 0;
	uint32_t counter = 0;
	uint32_t device_checksum = 0, calc_checksum = 0;
	uint32_t i;

	filled_locations = read_inhx(infile, &mem, PROGRAM_FLASH_BASEADDR);
	if (!filled_locations) return;

	bulk_erase();

	if (!flags.debug) cerr << "[ 0%]";
	if (flags.client) fprintf(stdout, "@000");

	/* Write and check program area */
	if (!flags.boot_only) {
		for (addr = 0; addr < (mem.code_memory_size * 2) - 1; addr += rowsize) {

			/* Check if entire row is blank */
			for (i = 0; i < rowsize; i++) {
				if (mem.filled[(addr + i) / 2]) {
					break;
				}
			}

			if (i == rowsize) {
				calc_checksum += (0x000000FF * rowsize);
				continue;
			}

			SendCommand(ETAP_FASTDATA);
			XferFastData4P(PE_CMD_ROW_PROGRAM);
			XferFastData4P(PROGRAM_FLASH_BASEADDR + addr);

			for (uint32_t i = 0; i < rowsize; i += 4) {
				if (mem.filled[(addr + i) / 2]) {
					XferFastData4P((uint32_t)mem.location[(addr + i) / 2] |
						       ((uint32_t)mem.location[(addr + i) / 2 + 1] << 16));
					programmed_locations += 2;
					calc_checksum += (mem.location[(addr + i) / 2] & 0x00FF) +
							 (mem.location[(addr + i) / 2] >> 8) +
							 (mem.location[(addr + i) / 2 + 1] & 0x00FF) +
							 (mem.location[(addr + i) / 2 + 1] >> 8);
				} else {
					XferFastData4P(0xFFFFFFFF);
					calc_checksum += (0x000000FF * 4);
				}
			}

			rxp = GetPEResponse();
			if (rxp != PE_CMD_ROW_PROGRAM)
				fprintf(stderr, "___ERR___: %08x\n", rxp);

			if (counter != programmed_locations * 100 / filled_locations) {
				counter = programmed_locations * 100 / filled_locations;
				if (flags.client)
					fprintf(stdout, "@%03d", counter);
				if (!flags.debug)
					fprintf(stderr, "\b\b\b\b\b[%2d%%]", counter);
			}
		}

		SendCommand(ETAP_FASTDATA);
		XferFastData4P(PE_CMD_GET_CHECKSUM);
		XferFastData4P(PROGRAM_FLASH_BASEADDR);
		XferFastData4P(mem.code_memory_size * 2);
		rxp = GetPEResponse();
		if (rxp != PE_CMD_GET_CHECKSUM)
			fprintf(stderr, "___ERR___: %08x\n", rxp);
		device_checksum += GetPEResponse();
		if (calc_checksum != device_checksum) {
			fprintf(stderr, "___PROGRAM AREA CHECKSUM ERROR!___\n");
			fprintf(stderr, "DEVICE CHECKSUM: 0x%08x\n", device_checksum);
			fprintf(stderr, "CALCULATED CHECKSUM: 0x%08x\n", calc_checksum);
			if (flags.client) fprintf(stdout, "@ERR");
			return;
		}
	}

	/* Write and check boot area */
	if (!flags.program_only) {
		switch (subfamily) {
			case SF_PIC32MK:
				/* Fallthrough */
			case SF_PIC32MZ:
				chk_size = 0xFF00;
				break;
			default:
				chk_size = (bootsize - 16);
				break;
		}

		for (addr = BOOTFLASH_OFFSET; addr < (BOOTFLASH_OFFSET + bootsize - 1); addr += rowsize) {

			/* Check if entire row is blank */
			for (i = 0; i < rowsize; i++) {
				if (mem.filled[(addr + i) / 2]) {
					break;
				}
			}

			if (i == rowsize) {
				if (addr < (BOOTFLASH_OFFSET + chk_size)) {
					calc_checksum += (0x000000FF * rowsize);
				}
				continue;
			}

			SendCommand(ETAP_FASTDATA);
			XferFastData4P(PE_CMD_ROW_PROGRAM);
			XferFastData4P(PROGRAM_FLASH_BASEADDR + addr);

			for (uint32_t i = 0; i < rowsize; i += 4) {
				if (mem.filled[(addr + i) / 2]) {
					XferFastData4P((uint32_t)mem.location[(addr + i) / 2] |
						       ((uint32_t)mem.location[(addr + i) / 2 + 1] << 16));
					programmed_locations += 2;
					if ((addr + i) < (BOOTFLASH_OFFSET + chk_size)) {
						calc_checksum += (mem.location[(addr + i) / 2] & 0x00FF) +
								 (mem.location[(addr + i) / 2] >> 8) +
								 (mem.location[(addr + i) / 2 + 1] & 0x00FF) +
								 (mem.location[(addr + i) / 2 + 1] >> 8);
					}
				} else {
					XferFastData4P(0xFFFFFFFF);
					if ((addr + i) < (BOOTFLASH_OFFSET + chk_size)) {
						calc_checksum += (0x000000FF * 4);
					}
				}
			}

			rxp = GetPEResponse();
			if (rxp != PE_CMD_ROW_PROGRAM)
				fprintf(stderr, "___ERR___: %08x\n", rxp);

			if (counter != programmed_locations * 100 / filled_locations) {
				counter = programmed_locations * 100 / filled_locations;
				if (flags.client)
					fprintf(stdout,"@%03d", counter);
				if (!flags.debug)
					fprintf(stderr,"\b\b\b\b\b[%2d%%]", counter);
			}
		}

		SendCommand(ETAP_FASTDATA);
		XferFastData4P(PE_CMD_GET_CHECKSUM);
		XferFastData4P(PROGRAM_FLASH_BASEADDR + BOOTFLASH_OFFSET);
		XferFastData4P(chk_size);
		rxp = GetPEResponse();
		if (rxp != PE_CMD_GET_CHECKSUM)
			fprintf(stderr, "___ERR___: %08x\n", rxp);
		device_checksum += GetPEResponse();

		if (calc_checksum != device_checksum) {
			fprintf(stderr, "___BOOT AREA CHECKSUM ERROR!___\n");
			fprintf(stderr, "DEVICE CHECKSUM: %08x\n", device_checksum);
			fprintf(stderr, "CALCULATED CHECKSUM: %08x\n", calc_checksum);
			if (flags.client) fprintf(stdout, "@ERR");
			return;
		}
	}

	if (!flags.debug) cerr << "\b\b\b\b\b\b";
	if (flags.client) fprintf(stdout, "@FIN");
};

void pic32::dump_configuration_registers(void)
{
	SendCommand(ETAP_FASTDATA);
	XferFastData4P(PE_CMD_READ | 0x04);
	XferFastData4P(PROGRAM_FLASH_BASEADDR + BOOTFLASH_OFFSET + bootsize - 16);
	GetPEResponse();
	for (uint8_t r=0; r<4; r++) {
		fprintf(stderr, "DEVCFG%d = %08x\n", (3 - r), GetPEResponse());
	}
};
