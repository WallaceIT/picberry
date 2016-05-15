/*
 * Raspberry Pi PIC Programmer using GPIO connector
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

#include <iostream>

#include "../common.h"
#include "device.h"

using namespace std;

#define SF_PIC32MX1		0x00
#define SF_PIC32MX2		0x01
#define SF_PIC32MX3		0x02
#define SF_PIC32MZ		0x03
#define SF_PIC32MK		0x04

class pic32: public Pic{

	public:
		pic32(uint8_t sf){
			subfamily=sf;
		};
		void enter_program_mode(void);
		void exit_program_mode(void);
		bool setup_pe(void);
		bool read_device_id(void);
		void bulk_erase(void);
		void dump_configuration_registers(void);
		void read(char *outfile, uint32_t start=0, uint32_t count=0);
		void write(char *infile);
		uint8_t blank_check(void);

	protected:
		uint8_t Data4Phase(uint8_t tdi, uint8_t tms);
		void Data2Phase(uint8_t tdi, uint8_t tms);
		void SetMode(uint8_t length, uint8_t mode);
		void SendCommand(uint8_t command);
		uint32_t XferData(uint8_t length, uint32_t iData);
		void XferFastData2P(uint32_t iData);
		uint32_t XferFastData4P(uint32_t iData);
		void XferInstruction(uint32_t instruction);
		uint32_t ReadFromAddress(uint32_t address);
		uint32_t GetPEResponse(void);
		bool check_device_status(void);
		void code_protected_bulk_erase(void);
		bool enter_serial_exec_mode(void);
		bool download_pe(char *pe_infile);
		
		uint32_t bootsize;
		uint32_t rowsize;

		/*
		* DEVICES SECTION
		*                    	ID       NAME           MEMSIZE (16bit words)
		*/
		pic_device piclist[1] = {{0x04d00053,  "PIC32MX250F128B", 0x10000}};
};
